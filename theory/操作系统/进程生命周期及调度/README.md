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

#### 设置代码位置

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
