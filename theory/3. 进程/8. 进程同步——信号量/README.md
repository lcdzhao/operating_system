# 信号量
> - [背景知识](https://www.bilibili.com/video/BV1uW411f72n?p=57&vd_source=afbe39567defad401c79f6fbb57691cf)
> - [临界区及几种方法](https://www.bilibili.com/video/BV1uW411f72n?p=61&vd_source=afbe39567defad401c79f6fbb57691cf)
## 什么是信号量
操作系统通常区分**计数信号量**与**二进制信号量**。

#### 计数信号量
计数信号量的值不受限制。计数信号量可以用于控制访问具有多个实例的某种资源。信号量的初值为可用资源数量。当进程需要使用资源时，需要对该信号量执行 `wait()` 操作（减少信号量的计数)。当进程释放资源时，需要对该信号量执行 `signal()` 操作（增加信号量的计数）。当信号量的计数为 `0` 时，所有资源都在使用中。之后，需要使用资源的进程将会阻塞，直到计数大于 `0`。

#### 二进制信号量
二进制信号量的值只能为 `0` 或 `1`。因此，二进制信号量类似于互斥锁。事实上，在没有提供互斥锁的系统上，可以使用二进制信号量来提供互斥。我们也可以使用信号量来解决各种同步问题。


## 信号量的工作原理
由于信号量只能进行两种操作等待和发送信号，即`P(sv)`和`V(sv)`,他们的行为是这样的：

- `P(sv)`：如果`sv`的值大于零，就给它减`1`；如果它的值为零，就挂起该进程的执行

- `V(sv)`：如果有其他进程因等待`sv`而被挂起，就让它恢复运行，如果没有进程因等待`sv`而挂起，就给它加`1`.

举个例子，就是两个进程共享信号量`sv(初始值为1)`，一旦其中一个进程执行了`P(sv)`操作，它将得到信号量，并可以进入临界区，使`sv`减`1`。而第二个进程将被阻止进入临界区，因为当它试图执行`P(sv)`时，`sv`为`0`，它会被挂起以等待第一个进程离开临界区域并执行`V(sv)`释放信号量，这时第二个进程就可以恢复执行。

## 信号量相关实验及其源码
### 实验
[点击跳转到：lab_5_semaphore](https://github.com/lcdzhao/operating_system/tree/master/linux-0.1.1-labs/labs/lab_5_semaphore)
### 源码
```c
#define __LIBRARY__  
#include <unistd.h>  
#include <linux/sched.h>  
#include <linux/kernel.h>  
#include <asm/segment.h>  
#include <asm/system.h>   

#define SEM_COUNT 5 
sem semaphores[SEM_COUNT] ={
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL}
}; 

void init_queue(t_queue* q)  
{  
    q->front = q->rear = 0; 
}

int is_empty(t_queue* q)
{
	return q->front == q->rear?1:0;
}

int is_full(t_queue* q)
{    
	return (q->rear+1)%QUE_LEN == q->front?1:0;
}

struct task_struct * get_first__task(t_queue* q)
{
	if(is_empty(q)) 
	{
	//	printk("Queue is empty!\n");
		return NULL;
	}
	struct task_struct *tmp = q->wait_tasks[q->front]; 
	q->front = (q->front+1)%QUE_LEN;
	return tmp;
}

int insert_task(struct task_struct *p,t_queue* q)
{
	printk("Insert %d",p->pid);
	if(is_full(q))
	{
		printk("Queue is full!\n");
		return -1;
	}
	q->wait_tasks[q->rear] = p;
	q->rear = (q->rear+1)%QUE_LEN;
	return 1;
}

int sem_location(const char* name)
{  
    int i;
    for(i = 0;i < SEM_COUNT; i++)  
    {  
        if(strcmp(name,semaphores[i].name) == 0 && semaphores[i].occupied == 1) 
        {     
            return i;  
        }  
    }  
    return -1; 
}  

sem* sys_sem_open(const char* name,unsigned int value)
{
	char tmp[16];
	char c;
	int i;
	for( i = 0; i<16; i++)
	{
		c = get_fs_byte(name+i);
		tmp[i] = c;
		if(c =='\0') break;
	}
	if(c >= 16) 
	{ 	
		printk("Semaphore name is too long!");
		return NULL;
	}
	if((i = sem_location(tmp)) != -1)
	{
		return &semaphores[i];
	}
	for(i = 0;i< SEM_COUNT; i++)
	{
		if(!semaphores[i].occupied)
		{
			strcpy(semaphores[i].name,tmp);
			semaphores[i].occupied = 1;
			semaphores[i].value = value;
			init_queue(&(semaphores[i].wait_queue));
			printk("%d %d %d %s\n",semaphores[i].occupied,i,semaphores[i].value,semaphores[i].name);
			printk("%p\n",&semaphores[i]); 
			return &semaphores[i];
		}
	}	
	printk("Numbers of semaphores are limited!\n");
	return NULL;
}  

int sys_sem_wait(sem* sem)
{
	cli();
	sem->value--;
	sti();
	if(sem->value < 0)
	{
		// 参见sleep_on
		current->state = TASK_UNINTERRUPTIBLE;
		insert_task(current,&(sem->wait_queue));
		schedule();
	}
	return 0;
}

int sys_sem_post(sem* sem)
{
	struct task_struct *p;
	cli();
	sem->value++;
	sti();
	if(sem->value <= 0)
	{
		p = get_first__task(&(sem->wait_queue));
		if(p != NULL)
		{
			(*p).state = TASK_RUNNING;
		}
	}
	return 0;
}

int sys_sem_unlink(const char *name)  
{  
    char tmp[16];
    char c;
	int i;
	for( i = 0; i<16; i++)
	{
		c = get_fs_byte(name+i);
		tmp[i] = c;
		if(c =='\0') break;
	}
	if(c >= 16) 
	{
		printk("Semphore name is too long!");
		return -1;
	}
    int ret = sem_location(tmp); 
    if(ret != -1)
    {
    	semaphores[ret].value = 0;
    	strcpy(semaphores[ret].name,"\0");
    	semaphores[ret].occupied = 0;
    	return 0;
    }   
    return -1;  
}  
```
该源码中，最巧妙也最难理解的部分便是对于`schedule()`的使用，这里不展开解释，感兴趣的同学可以自己阅读[ `schedule()` 相关的知识 及其在 `sleep_on()` 和 `wake_up()` 的使用](https://github.com/lcdzhao/operating_system/tree/master/theory/3.%20%E8%BF%9B%E7%A8%8B/5.%20%E8%BF%9B%E7%A8%8B%E7%94%9F%E5%91%BD%E5%91%A8%E6%9C%9F%E5%8F%8A%E8%B0%83%E5%BA%A6)。
## 参考文章

- [Linux进程间通信——使用信号量](https://blog.csdn.net/ljianhui/article/details/10243617)
