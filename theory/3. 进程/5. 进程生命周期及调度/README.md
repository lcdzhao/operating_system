# 进程的生命周期(Linux 0.11)
> 本文章参考文章：
> - [linux4.8.4 进程状态](https://quant67.com/post/linux/taskstatus.html)
> - [Linux 0.11 下进程状态的切换](https://or1onx.github.io/Linux%E4%B8%8B%E8%BF%9B%E7%A8%8B%E7%8A%B6%E6%80%81%E7%9A%84%E5%88%87%E6%8D%A2/)
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
`SIGSTOP`、`SIGTSTP`、`SIGTTIN`、`SIGTTOUT`等信号会将进程暂时停止，进入`TASK_STOPPED`状态。这4种状态不可被忽略，不可被屏蔽，不能安装新的处理函数。在收到`SIGCONT`后进程可以恢复执行。
## Linux 0.11 状态切换点源码
> Post author: Or1onX
> 
> Post link: https://or1onx.github.io/Linux下进程状态的切换/
> 
> Copyright Notice: All articles in this blog are licensed under CC BY-NC-SA 3.0 unless stating additionally.

### 一些宏定义
#### 具体状态转换
![状态转换图](http://ws4.sinaimg.cn/large/a105112bly1g3n5lydlmrj215n0n8tdb.jpg)

#### Linux 0.11支持的任务总数(`include/linux/sched.h`)
```C
#define NR_TASK 64
```
#### 第一个任务和最后一个任务（遍历任务时会用到）(`include/linux/sched.h`)
```C
#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]
```
### 全局变量 (`include/linux/sched.h`)
#### 任务结构
```C
struct task_struct {
    ...
}
```
#### 存放任务结构的数组 (`include/linux/sched.h`)
```C
struct task_struct * task[NR_TASK] = {&(init_task.task), };
```

### 关键函数
让我们假设一切初始化工作运转良好，操作系统顺利执行了`init/main.c`的`main()`函数，内核开始继续进行所有硬件的初始化工作，随后启动任务 0，并通过：
```C
move_to_user_mode();
```
CPU 从`0`特权级转换为`3`特权级。

#### fork
我感觉最具神秘色彩的一段代码便是下面这几行，就好像上帝前五天创造了光、空气、动物等，完成了环境的初始化工作，在第六天开始，照着自己的模样创造（`fork`）出了人类（新进程）:
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
前面我们提到程序特权级变为 `3`，在运行到:
```C
if (!fork()) {
    init();
}
```
时进程是用户状态，而`fork()`程序涉及到内存页面复制等内核级操作，因此，执行`fork()`实际上是通过`int $0x80`进入系统调用，进而执行了`sys_fork`，这一点我们可以在`kernel/system_call.s`中找到：
```asm
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
```
可以看出，`sys_fork`首先调用`kernel/fork.c`中的`find_empty_process`函数，该函数旨在遍历任务数组中的任务，生成一个空闲的 `pid` 并分配给新进程，最后返回分配了新 `pid` 的任务号到`eax`中，之后`push`一堆参数入栈，以调用`copy_process`。

fork函数最重要的部分想必就是`copy_process`，而这也是进程状态发生切换的一个地方。
```C
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
    p->state = TASK_UNINTERRUPTIBLE;  // 初始化先将进程状态设为TASK_UNINTERRUPTIBLE，避免未初始化完成的进程被调度。
    p->pid = last_pid;
    p->father = current->pid;
    ...
    p->state = TASK_RUNNING;	 // 初始化完成后，将进程状态设置为TASK_RUNNING
    return last_pid;
}
```
`copy_process`的主要作用是对父进程进行拷贝并赋给新进程，参数`nr`就是`find_empty_process`的返回值，它是`task`数组的下标，在这段代码里，我们可以看到有两处切换点：
```C
p->state = TASK_UNINTERRUPTIBLE;
p->state = TASK_RUNNING;
```
> PS: 更详细的`fork`相关的内容见：[fork](https://github.com/lcdzhao/operating_system/tree/master/theory/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/fork)
#### schedule
`schedule`是进程调度函数，位于`kernel/sched.c`，它可以说是**进程切换的核心部分**，先来看一下 `Linus Torvalds` 巨佬对这个函数的描述，便可知其重要性:
```C
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
```
代码简短而精彩:
```C
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
```
在第一个 `for` 循环里，程序从最后一个任务开始向前遍历，如果信号位图中除被阻塞的信号外还有其他信号，并且任务处于`TASK_INTERRUPTIBLE`，则将其置为`TASK_RUNNING`，此处为一个切换点。 在 `wihle` 循环里，会先选出时间片最大的进程，如果所有进程的时间片均用完，则会对所有进程更新其时间片。
最后，通过`switch_t`o跳转到时间片最大（或者说优先级最高）的进程。

#### sys_pause
代码位置：`kernel/sched.c`,源码：
```C
int sys_pause(void)
{
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    return 0;
}
```
该函数是`pause()`函数的系统调用。在`init/main.c`中有
```
for(;;) pause();
```
表示在没有其他任务时，任务 0 会不断的执行主动让出的操作。调用`sys_pause`时，会先将自己的状态置为睡眠态（`TASK_INTERRUPTIBLE`），此处为一切换点。然后执行调度函数跳转到其他可执行的任务（当然如果没有可执行的任务，`schdule` 中的 `next` 仍为0，又会跳回任务 0）

#### sleep_on
代码位置：`kernel/sched.c`,源码：
```C
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
    if (tmp)
        tmp->state=0;
}
```
这部分是把当前任务（即调用者）置为不可中断的等待状态，并让睡眠队列头指针指向当前任务。
首先，函数参数传入的是指针的指针，正如初学c语言时写的 `swap` 函数，要想在调用函数里更改两个变量的值，需要传入变量的地址。而在内核代码中，定义的任务变量本身就是一个指针，要想在调用函数里改变其状态，就需要传递指针的地址。
接下来对当前进程进行判断，如果是任务 0，则予以警告，不允许任务 0进入睡眠状态。
- `tmp = *p`是将tmp指向已经在等待队列头的任务。
- `*p = current`是将睡眠队列头的指针指向当前任务，即把当前任务插入到了等待队列中。
- `current->state = TASK_UNINTERRUPTIBLE`是将当前任务的状态置为TASK_UNINTERRUPTIBLE，此处为一切换点。

接着先执行`schedule`进程调度函数，当被唤醒时（`wake_up`，后面介绍），会将`tmp`指向的等待任务（即位于原等待队列头的任务）的状态置为`TASK_RUNNING（tmp->state = 0）`，此处为一切换点。

对于由tmp隐式构成的等待队列，可以对照《Linux内核完全注释》中的图来理解：


#### interruptible_sleep_on
代码位置：`kernel/sched.c`,源码：
```C
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
    *p=NULL;
    if (tmp)
        tmp->state=0;
}
```
和sleep_on函数功能类似，只是这里是将当前任务的状态置为可中断等待状态（`TASK_INTERRUPTIBLE`），其他切换点的位置和sleep_on的基本相同，不过多了一处`(**p).state = 0`，意思是将在当前任务入队之后加进等待队列的任务唤醒(即：让唤醒一定从队头开始)【猜测是因为`TASK_INTERRUPTIBLE`的进程可以被信号唤醒，而这种唤醒不一定是从队头唤醒，故使用这种与sleep_on不同的方式唤醒】。

#### wake_up
代码位置：`kernel/sched.c`,源码：
```C
void wake_up(struct task_struct **p)
{
    if (p && *p) {
        (**p).state=0;
        *p=NULL; 
    }
}
```
该函数用来唤醒等待队列头指针指向的任务（即最后入队的任务），`(**p).state=0`是一个切换点。

#### do_exit
代码位置：`kernel/exit.c`,源码：
```C
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

	// 关闭当前进程打开着的所有文件
	for (i=0 ; i<NR_OPEN ; i++)
		if (current->filp[i])
			sys_close(i);
			
	// 对当前进程的工作目录pwd、根目录root以及程序文件的i节点进行同步操作，放回
	// 各个i节点并分别置空(释放)
	iput(current->pwd);
	current->pwd=NULL;
	iput(current->root);
	current->root=NULL;
	iput(current->executable);
	current->executable=NULL;
	// 如果当前进程是会话头领(leader)进程并且其有控制终端，则释放该终端。
	if (current->leader && current->tty >= 0)
		tty_table[current->tty].pgrp = 0;
	// 如果当前进程上次使用过协处理器，则将 last_task_used_math 置空
	if (last_task_used_math == current)
		last_task_used_math = NULL;
	// 如果当前进程是leader进程，则终止该会话的所有相关进程
	if (current->leader)
		kill_session();

    current->state = TASK_ZOMBIE;
    current->exit_code = code;
    tell_father(current->father);
    schedule();
    return (-1);	/* just to suppress warnings */
}
```
**首先释放调用该函数的进程的代码段和数据段所占的内存页。如果当前进程有子进程，则将这些子进程的父进程改为进程 1,即 init 进程**。

**之后进行一系列资源释放操作，然后将当前进程的状态置为TASK_ZOMBIE，此处为一切换点。可以注意到，此处并没有回收PCB(task_struct)所在的那一页内存。**

#### sys_waitpid
代码位置：`kernel/exit.c`,源码：
```C
int sys_waitpid(pid_t pid,unsigned long * stat_addr, int options)
{
	int flag, code;
	struct task_struct ** p;

	verify_area(stat_addr,4);
repeat:
	flag=0;
	for(p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!*p || *p == current)
			continue;
		if ((*p)->father != current->pid)
			continue;
		if (pid>0) {
			if ((*p)->pid != pid)
				continue;
		} else if (!pid) {
			if ((*p)->pgrp != current->pgrp)
				continue;
		} else if (pid != -1) {
			if ((*p)->pgrp != -pid)
				continue;
		}
		switch ((*p)->state) {
			case TASK_STOPPED:
				if (!(options & WUNTRACED))
					continue;
				put_fs_long(0x7f,stat_addr);
				return (*p)->pid;
			case TASK_ZOMBIE:
				current->cutime += (*p)->utime;
				current->cstime += (*p)->stime;
				flag = (*p)->pid;
				code = (*p)->exit_code;
				release(*p);  // 释放指定进程的占用的任务槽(task[nr])及其任务数据结构占用的内存页面。
				put_fs_long(code,stat_addr);
				return flag;
			default:
				flag=1;
				continue;
		}
	}
	if (flag) {
		if (options & WNOHANG)
			return 0;
		current->state=TASK_INTERRUPTIBLE;
		schedule();
		if (!(current->signal &= ~(1<<(SIGCHLD-1))))  //判断的同时将signal复位
			goto repeat;
		else
			return -EINTR;
	}
	return -ECHILD;
}
```
##### 参数分析：
pid: 
- 当pid>0表示等待回收该pid的子进程；
- pid=0时，回收**进程组号**等于当前**进程组号**的任何子进程；
- pid<-1时，回收**进程组号**为**pid绝对值**的任何子进程；
- pid=-1，回收任何子进程。

option：可以使该系统调用直接返回或者陷入阻塞等待；
- WUNTRACE： 即使子进程已经停止，也马上返回（无需追踪）
- WNOHANG： 表示如果没有子进程退出或终止，就马上返回
- 否则为阻塞调用，等待子进程结束。

stat_addr用来保存状态信息：
- `put_fs_long()`将状态信息也就是退出码写到用户态堆栈上面，之后返回flag也就是子进程pid。
   

## 总结
在 `Linux 0.11` 内核版本中，进程状态的切换发生在如下文件中:
### 就绪态和运行态的相互转换
- `schedule(kernel/sched.c)`
### 运行态到睡眠态
- `sleep_on(kernel/sched.c)`
- `interruptible_sleep_on(kernel/sched.c)`
- `sys_pause(kernel/sched.c)`
- `sys_waitpid(kernel/exit.c)`
### 睡眠态到就绪态
- `sleep_on(kernel/sched.c)`
- `interruptible_sleep_on(kernel/sched.c)`
- `wake_up(kernel/sched.c)`
- `fork`(kernel/fork.c,初始化时设为`TASK_UNINTERRUPTIBLE`，初始化完成后设为`TASK_RUNNING`)
