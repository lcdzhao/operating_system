# 进程的生命周期
## 进程的所有状态

linux 0.11 在`include/linux/sched.h` 中定义进程的各个状态

```C
#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4
```

### TASK_RUNNING —— 运行中
#### 意义
表示进程处于运行中，运行中并不代表一定正在使用CPU，`schedule`会在所有的`RUNNING`的进程中选择一个来使用CPU。所以`RUNNING`状态的所有的进程其实又被分为了:`就绪态`和`运行态`，其区别为是否正在CPU上运行，并不是真正对应了一种状态。
#### 设置代码位置
##### `fork`
初始化完成后(初始化过程中为了防止`fork`产生的子进程被调度，其状态为`TASK_UNINTERRUPTIBLE`)，`fork`产生的子进程将被设置为`RUNNING`。
代码位置：linix 0.11中的`kernel/fork.c`中的`copy_process`方法。
```C
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	... // 初始化过程
	p->state = TASK_UNINTERRUPTIBLE;
	... // 初始化过程
	p->state = TASK_RUNNING;
	return last_pid;
}
```

##### 中断
当进程调用一些阻塞操作时，进程将会处于其他状态，操作系统将通过中断来通知该阻塞进程已经执行结束从而将进程重新设置为`TASK_RUNNING`状态。如: 

### TASK_INTERRUPTIBLE —— 可中断睡眠

#### 设置代码位置

### TASK_UNINTERRUPTIBLE —— 不可中断睡眠

#### 设置代码位置

### TASK_ZOMBIE —— 僵尸

#### 设置代码位置

### TASK_STOPPED —— 终止

#### 设置代码位置

## 相关调用

### fork

### sleep_on 

### interruptible_sleep_on

### sys_pause

### exit

### sys_waitpid


# 进程调度
## schedule


## 进程调度状态图

# 设备操作与进程状态
## 字符设备操作
### 写

### 读


## 块设备操作

### 写

### 读
