# 进程的生命周期
## 进程的所有状态

linux 0.11 在`include/linux/sched.h` 中定义进程的各个状态

```
#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4
```

### TASK_RUNNING —— 运行中

### TASK_INTERRUPTIBLE —— 可中断睡眠

### TASK_UNINTERRUPTIBLE —— 不可中断睡眠

### TASK_ZOMBIE —— 僵尸

### TASK_STOPPED —— 终止
