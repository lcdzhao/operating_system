# 进程的生命周期(Linux 0.11)
> 本文章参考文章：
> - [linux4.8.4 进程状态](https://quant67.com/post/linux/taskstatus.html)
> - [Linux 0.11 下进程状态的切换]([https://quant67.com/post/linux/taskstatus.html](https://or1onx.github.io/Linux%E4%B8%8B%E8%BF%9B%E7%A8%8B%E7%8A%B6%E6%80%81%E7%9A%84%E5%88%87%E6%8D%A2/))
## 进程的所有状态

linux 0.11 在`include/linux/sched.h` 中定义进程的各个状态

```C
#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4
```
### 状态介绍
#### TASK_RUNNING 
表示进程处于运行中，运行中并不代表一定正在使用CPU，`schedule`会在所有的`RUNNING`的进程中选择一个来使用CPU。所以`RUNNING`状态的所有的进程其实又被分为了:`就绪态`和`运行态`，其区别为是否正在CPU上运行，并不是真正对应了一种状态。
#### TASK_INTERRUPTIBLE 和 TASK_UNINTERRUPTIBLE
当进程和慢速设备打交道，或者需要等待条件满足时，这种等待时间是不可预估的，这种情况下，内核会将该进程从CPU的运行队列中移除，从而进程进入睡眠状态。

Linux的进程有两种睡眠状态：`TASK_INTERRUPTIBLE`和`TASK_UNINTERRUPTIBLE`，这两种状态的区别是能否响应收到的信号。
处于`TASK_INTERRUPTIBLE`状态的进程遇到下面两种情况会返回到TASK_RUNNING状态：

- 等待条件满足(`wakeup`唤醒)；
- 收到未被屏蔽的信号。

收到信号时，会返回EINTR，需要检测返回值以作出正确处理。
对于`TASK_UNINTERRUPTIBLE`，只有等待条件满足才有可能返回运行状态(即：只能被`wakeup`唤醒)，任何信号都无法打断它。如果这种状态的进程出错，无法杀死，只能重启。

`TASK_UNINTERRUPTIBLE`的存在是因为内核中某些处理是不能被打断的，比如read系统调用正在操作磁盘，就要用`TASK_UNINTERRUPTIBLE`将其保护起来以免受到打扰而陷入不可控的状态。

睡眠状态的进程都保存在等待队列中。
#### TASK_ZOMBIE 
在`fork()/execve()`过程中，假设子进程结束时父进程仍存在，而父进程`fork()`之前既没安装SIGCHLD信号处理函数调用`waitpid()`等待子进程结束，又没有显式忽略该信号，则子进程成为僵死进程，无法正常结束，此时即使是`root`身份`kill -9`也不能杀死僵死进程。**补救办法是杀死僵尸进程的父进程(僵死进程的父进程必然存在)，僵死进程成为`孤儿进程`，过继给1号进程init，init始终会负责清理僵死进程**。
#### TASK_STOPPED 
进程已结束。
## Linux 0.11 状态切换点源码
> Post author: Or1onX
> 
> Post link: https://or1onx.github.io/Linux下进程状态的切换/
> 
> Copyright Notice: All articles in this blog are licensed under CC BY-NC-SA 3.0 unless stating additionally.

### 一些宏定义
#### 具体状态转换
![状态转换图](http://ws4.sinaimg.cn/large/a105112bly1g3n5lydlmrj215n0n8tdb.jpg)

#### Linux 0.11支持的任务总数
```C
#define NR_TASK 64
```
#### 第一个任务和最后一个任务（遍历任务时会用到）
```C
#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]
```
### 全局变量 
#### 任务结构
```C
struct task_struct {
    ...
}
```
#### 存放任务结构的数组
```C
struct task_struct * task[NR_TASK] = {&(init_task.task), };
```

### 关键函数
让我们假设一切初始化工作运转良好，操作系统顺利执行了init/main.c的main()函数，内核开始继续进行所有硬件的初始化工作，随后启动任务 0，并通过
```C
move_to_user_mode();
```
CPU 从 0 特权级转换为 3 特权级

#### fork
我感觉最具神秘色彩的一段代码便是下面这几行，就好像上帝前五天创造了光、空气、动物等，完成了环境的初始化工作，在第六天开始，照着自己的模样创造（fork）出了人类（新进程）
```C
if (!fork()) {
    init();
}
```
接下来会先调用 `fork` 程序，我们继续顺着程序流程进入到 `fork` 里去。

在`kernel/fork.c`文件中，定义了一些函数，
```C
extern void write_verify(unsigned long address);

void verify_area(void * addr,int size);

int copy_mem(int nr,struct task_struct * p);

int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
            long ebx,long ecx,long edx,
            long fs,long es,long ds,
            long eip,long cs,long eflags,long esp,long ss);

int find_empty_process(void);
```
前面我们提到程序特权级变为 3，在运行到

if (!fork()) {
    init();
}
时进程是用户状态，而fork()程序涉及到内存页面复制等内核级操作，因此，执行fork()实际上是通过int $0x80进入系统调用，进而执行了sys_fork，这一点我们可以在kernel/system_call.s中找到：

_sys_fork:
    call _find_empty_process
    testl %eax,%eax
    js 1f
    push %gs
    pushl %esi
    pushl %edi
    pushl %ebp
    pushl %eax
    call _copy_process
    addl $20,%esp
1:	ret
可以看出，sys_fork首先调用kernel/fork.c中的find_empty_process函数，该函数旨在遍历任务数组中的任务，生成一个空闲的 pid 并分配给新进程，最后返回分配了新 pid 的任务号到eax中，之后push一堆参数入栈，以调用copy_process。

fork函数最重要的部分想必就是copy_process，而这也是进程状态发生切换的一个地方。

int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
        long ebx,long ecx,long edx,
        long fs,long es,long ds,
        long eip,long cs,long eflags,long esp,long ss)
{
    struct task_struct *p;
    int i;
    struct file *f;

    p = (struct task_struct *) get_free_page();
    if (!p)
        return -EAGAIN;
    task[nr] = p;
    *p = *current;	/* NOTE! this doesn't copy the supervisor stack */
    p->state = TASK_UNINTERRUPTIBLE;
    p->pid = last_pid;
    p->father = current->pid;
    ...
    p->state = TASK_RUNNING;	/* do this last, just in case */
    return last_pid;
}
copy_process的主要作用是对父进程的状态进行拷贝并赋给新进程，参数nr就是find_empty_process的返回值，它是task数组的下标，在这段代码里，我们可以看到有两处切换点：

p->state = TASK_UNINTERRUPTIBLE;
p->state = TASK_RUNNING;
schedule
schedule 是进程调度函数，它可以说是进程切换的核心部分，先来看一下 Linus Torvalds 巨佬对这个函数的描述，便可知其重要性

/*

*  'schedule()' is the scheduler function. This is GOOD CODE! There
* probably won't be any reason to change this, as it should work well
* in all circumstances (ie gives IO-bound processes good response etc).
* The one thing you might take a look at is the signal-handler code here.
*
*   NOTE!!  Task 0 is the 'idle' task, which gets called when no other
* tasks can run. It can not be killed, and it cannot sleep. The 'state'
* information in task[0] is never used.
*/
代码简短而精彩

void schedule(void)
{
    int i,next,c;
    struct task_struct ** p;

/* check alarm, wake up any interruptible tasks that have got a signal */

    for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
        if (*p) {
            if ((*p)->alarm && (*p)->alarm < jiffies) {
                    (*p)->signal |= (1<<(SIGALRM-1));
                    (*p)->alarm = 0;
                }
            if (((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) &&
            (*p)->state==TASK_INTERRUPTIBLE)
                (*p)->state=TASK_RUNNING;
        }

/* this is the scheduler proper: */

    while (1) {
        c = -1;
        next = 0;
        i = NR_TASKS;
        p = &task[NR_TASKS];
        while (--i) {
            if (!*--p)
                continue;
            if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
                c = (*p)->counter, next = i;
        }
        if (c) break;
        for(p = &LAST_TASK ; p > &FIRST_TASK ; --p)
            if (*p)
                (*p)->counter = ((*p)->counter >> 1) +
                        (*p)->priority;
    }
    switch_to(next);
}
在第一个 for 循环里，程序从最后一个任务开始向前遍历，如果信号位图中除被阻塞的信号外还有其他信号，并且任务处于TASK_INTERRUPTIBLE，则将其置为TASK_RUNNING，此处为一个切换点。 在 wihle 循环里，会先选出时间片最大的进程，如果所有进程的时间片均用完，则会对所有进程更新其时间片。
最后，通过switch_to跳转到时间片最大（或者说优先级最高）的进程。

sys_pause
source code

int sys_pause(void)
{
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    return 0;
}
该函数是pause()函数的系统调用。在init/main.c中有

for(;;) pause();
表示在没有其他任务时，任务 0 会不断的执行主动让出的操作。调用sys_pause时，会先将自己的状态置为睡眠态（TASK_INTERRUPTIBLE），此处为一切换点。然后执行调度函数跳转到其他可执行的任务（当然如果没有可执行的任务，schdule 中的 next 仍为 0，又会跳回任务 0）

sleep_on
source code
void sleep_on(struct task_struct **p)
{
    struct task_struct *tmp;

    if (!p)
        return;
    if (current == &(init_task.task))
        panic("task[0] trying to sleep");
    tmp = *p;
    *p = current;
    current->state = TASK_UNINTERRUPTIBLE;
    schedule();
    *p = tmp; // 这行在源码里没有，但应该加上，因为当前进程已被唤醒，需将等待队列头指针指回之前的任务
    if (tmp)
        tmp->state=0;
}
这部分是把当前任务（即调用者）置为不可中断的等待状态，并让睡眠队列头指针指向当前任务
首先，函数参数传入的是指针的指针，正如初学c语言时写的 swap 函数，要想在调用函数里更改两个变量的值，需要传入变量的地址。而在内核代码中，定义的任务变量本身就是一个指针，要想在调用函数里改变其状态，就需要传递指针的地址。
接下来对当前进程进行判断，如果是任务 0，则予以警告，不允许任务 0进入睡眠状态
tmp = *p是将tmp指向已经在等待队列头的任务
*p = current是将睡眠队列头的指针指向当前任务，即把当前任务插入到了等待队列中
current->state = TASK_UNINTERRUPTIBLE是将当前任务的状态置为TASK_UNINTERRUPTIBLE，此处为一切换点
接着先执行schedule进程调度函数，当被唤醒时（wake_up，后面介绍），会将tmp指向的等待任务（即位于原等待队列头的任务）的状态置为TASK_RUNNING（tmp->state = 0），此处为一切换点
对于由tmp隐式构成的等待队列，可以对照《Linux内核完全注释》中的图来理解：


interruptible_sleep_on
source code

void interruptible_sleep_on(struct task_struct **p)
{
    struct task_struct *tmp;

    if (!p)
        return;
    if (current == &(init_task.task))
        panic("task[0] trying to sleep");
    tmp=*p;
    *p=current;
repeat:	current->state = TASK_INTERRUPTIBLE;
    schedule();
    if (*p && *p != current) {
        (**p).state=0;
        goto repeat;
    }
    *p=NULL;// 同理，这里应该是 *p = tmp
    if (tmp)
        tmp->state=0;
}
和sleep_on函数功能类似，只是这里是将当前任务的状态置为可中断等待状态（TASK_INTERRUPTIBLE），其他切换点的位置和sleep_on的基本相同，不过多了一处(**p).state = 0，意思是将在当前任务入队之后加进等待队列的任务唤醒。

wake_up
source code

void wake_up(struct task_struct **p)
{
    if (p && *p) {
        (**p).state=0;
        *p=NULL; // 此处应该删去
    }
}
该函数用来唤醒等待队列头指针指向的任务（即最后入队的任务），(**p).state=0是一个切换点

do_exit
source code

int do_exit(long code)
{
    int i;

    free_page_tables(get_base(current->ldt[1]),get_limit(0x0f));
    free_page_tables(get_base(current->ldt[2]),get_limit(0x17));
    for (i=0 ; i<NR_TASKS ; i++)
        if (task[i] && task[i]->father == current->pid) {
            task[i]->father = 1;
            if (task[i]->state == TASK_ZOMBIE)
                /* assumption task[1] is always init */
                (void) send_sig(SIGCHLD, task[1], 1);
        }

    ...

    current->state = TASK_ZOMBIE;
    current->exit_code = code;
    tell_father(current->father);
    schedule();
    return (-1);	/* just to suppress warnings */
}
首先释放调用该函数的进程的代码段和数据段所占的内存页。如果当前进程有子进程，则将这些子进程的父进程改为进程 1,即 init 进程

之后进行一系列资源释放操作，然后将当前进程的状态置为TASK_ZOMBIE，此处为一切换点

sys_waitpid
源码中有一处current->state = TASK_INTERRUPTIBLE，此处为一切换点
总结
在 Linux 0.11 内核版本中，进程状态的切换发生在如下文件中
就绪态和运行态的相互转换
schedule()
运行态到睡眠态
sleep_on
interruptible_sleep_on
sys_pause
sys_waitpid
睡眠态到就绪态
sleep_on
interruptible_sleep_on
wake_up
