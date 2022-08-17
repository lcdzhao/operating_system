# 1. `0` 号进程

`0` 号进程是所有进程的老祖宗，所有的进程都由其或其子孙`fork()`而来，其在操作系统启动时在`main()`函数中进行初始化，如下：
```c
void main(void)		/* This really IS void, no error here. */
{			

  	//省略其他代码
  	...
  
	mem_init(main_memory_start,memory_end);
	trap_init();
	blk_dev_init();
	chr_dev_init();
	tty_init();
	time_init();
  
 	 //下面这行代码初始化0号进程
	sched_init();  
  
 	//下面的代码均以0号进程的身份运行
	buffer_init(buffer_memory_end);
	hd_init();
	floppy_init();
	sti();
	move_to_user_mode();
	if (!fork()) {		/* we count on this going ok */
		init();
	}
/*
 *   NOTE!!   For any other task 'pause()' would mean we have to get a
 * signal to awaken, but task0 is the sole exception (see 'schedule()')
 * as task 0 gets activated at every idle moment (when no other tasks
 * can run). For task0 'pause()' just means we go check if some other
 * task can run, and if not we return here.
 */
	for(;;) pause();
}
```

## 1.1 `0` 号进程如何初始化
从 [进程结构](https://github.com/lcdzhao/operating_system/blob/master/theory/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/3.%20%E8%BF%9B%E7%A8%8B/2.%20%E8%BF%9B%E7%A8%8B%E7%BB%93%E6%9E%84/README.md#%E8%BF%9B%E7%A8%8B%E7%BB%93%E6%9E%84-1) 部分我们了解到了一个进程的：
- [1. 进程核心数据结构](https://github.com/lcdzhao/operating_system/blob/master/theory/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/3.%20%E8%BF%9B%E7%A8%8B/2.%20%E8%BF%9B%E7%A8%8B%E7%BB%93%E6%9E%84/README.md#1-%E8%BF%9B%E7%A8%8B%E6%A0%B8%E5%BF%83%E6%95%B0%E6%8D%AE%E7%BB%93%E6%9E%84)
- [2. 进程如何与硬件保护模式寻址联合](https://github.com/lcdzhao/operating_system/blob/master/theory/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/3.%20%E8%BF%9B%E7%A8%8B/2.%20%E8%BF%9B%E7%A8%8B%E7%BB%93%E6%9E%84/README.md#2-%E8%BF%9B%E7%A8%8B%E5%A6%82%E4%BD%95%E4%B8%8E%E7%A1%AC%E4%BB%B6%E4%BF%9D%E6%8A%A4%E6%A8%A1%E5%BC%8F%E5%AF%BB%E5%9D%80%E8%81%94%E5%90%88)
- [3. 如何让CPU运行进程](https://github.com/lcdzhao/operating_system/blob/master/theory/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/3.%20%E8%BF%9B%E7%A8%8B/2.%20%E8%BF%9B%E7%A8%8B%E7%BB%93%E6%9E%84/README.md#3-%E5%A6%82%E4%BD%95%E8%AE%A9cpu%E8%BF%90%E8%A1%8C%E8%BF%9B%E7%A8%8B)。

了解到上面这三点，我们再来看看 `0` 号进程初始化时的源码：

##### 1. `0` 号进程核心数据结构
在`include/linux/sched.h`中定义了 `INIT_TASK`:
```c
#define INIT_TASK \
/* state etc */	{ 0,15,15, \
/* signals */	0,{{},},0, \
/* ec,brk... */	0,0,0,0,0,0, \
/* pid etc.. */	0,-1,0,0,0, \
/* uid etc */	0,0,0,0,0,0, \
/* alarm */	0,0,0,0,0,0, \
/* math */	0, \
/* fs info */	-1,0022,NULL,NULL,NULL,0, \
/* filp */	{NULL,}, \
/* ldt */	{ \
/* ldt[0] null */		            {0,0}, \
/* ldt[1] code segment desc */	{0x9f,0xc0fa00}, \
/* ldt[2] data segment desc */	{0x9f,0xc0f200}, \
	}, \
/*tss*/	{0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&pg_dir,\
	 0,0,0,0,0,0,0,0, \
	 0,0,0x17,0x17,0x17,0x17,0x17,0x17, \
	 _LDT(0),0x80000000, \
		{} \
	}, \
}
```
上面的代码初始化了:

- `task_struct` ：`INIT_TASK`本身`

- 页表目录CR3(虚拟内存的映射表)：`(long)&pg_dir`

- `LDT`(代码段、数据段的段描述符)：

```c
/* ldt */	{ 
/* ldt[0] null */		            {0,0}, 
/* ldt[1] code segment desc */	{0x9f,0xc0fa00}, 
/* ldt[2] data segment desc */	{0x9f,0xc0f200}, 
  }
```
- 内核栈的起始位置: `PAGE_SIZE+(long)&init_task`,即 `init_task`所在的那页内存的顶部

-  cs、ss、es...等寄存器的值：
```c
/*tss*/	{0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&pg_dir,
	 0,0,0,0,0,0,0,0, 
	 0,0,0x17,0x17,0x17,0x17,0x17,0x17, 
	 _LDT(0),0x80000000, 
		{} 
	}, 
}
```

##### 2. `0` 号进程与硬件保护模式寻址联合
在`kernel/sched.c`的中：
```c
//省略其他代码
...

//内核用current索引当前正在运行的进程
struct task_struct *current = &(init_task.task);


//省略其他代码
...
void sched_init(void)
{
	int i;
	struct desc_struct * p;

	if (sizeof(struct sigaction) != 16)
		panic("Struct sigaction MUST be 16 bytes");
    
	//将 LDT与TSS放入GDT中，用于为硬件保护模式寻址做准备
	set_tss_desc(gdt+FIRST_TSS_ENTRY,&(init_task.task.tss));
	set_ldt_desc(gdt+FIRST_LDT_ENTRY,&(init_task.task.ldt));
	
	//省略其他代码
	...
}
```
##### 3. 让CPU运行`0 号`进程
在`kernel/sched.c`的中：
```c

//省略其他代码
...
void sched_init(void)
{
	//省略其他代码
	...
  	
	// 将`TSS`在GDT中的偏移地址放到`TR`寄存器中去(用于索引内核栈起始位置)
	ltr(0);
  
	// 将LDT在GDT中的偏移地址通过lldt指令放到LDTR寄存器中去(用于硬件保护模式寻址)
	lldt(0);
  
	// 0号进程首次初始化及运行时不需要将`TSS`的各个值覆盖到相应的寄存器上去
	// 因为cs,eip等寄存器已经是其当前的代码位置,且`CR3`以在`setup`开启分页模式时已经指定
  
	// 省略其他代码
  ...
}
```
## 1.2 `0` 号进程作用
```c
void main(void)		/* This really IS void, no error here. */
{			

 	 //省略其他代码
  	...
  
 	 //下面这行代码初始化0号进程
	sched_init();  
  
 	//下面的代码均以0号进程的身份运行
	
	// 内存高速缓冲区的初始化
	buffer_init(buffer_memory_end);
	// 硬盘的初始化
	hd_init();
	// 软盘的初始化
	floppy_init();
	
	// 开启中断
	sti();
	
	// 首次进入到用户模式
	move_to_user_mode();
	
	// 生成进程1
	if (!fork()) {		/* we count on this going ok */
		init();
	}
/*
 *   NOTE!!   For any other task 'pause()' would mean we have to get a
 * signal to awaken, but task0 is the sole exception (see 'schedule()')
 * as task 0 gets activated at every idle moment (when no other tasks
 * can run). For task0 'pause()' just means we go check if some other
 * task can run, and if not we return here.
 */
 	// 在无进程占用CPU时，不断主动触发调度
	for(;;) pause();
}
```
通过上面的源码可以看出。其功能有：
- 内存高速缓冲区的初始化
- 硬盘的初始化
- 软盘的初始化
- 首次进入到用户模式
- 生成进程1
- 在无进程占用CPU时，不断主动触发调度

想了解其具体细节时，可以参看：https://blog.csdn.net/ac_dao_di/article/details/52144608 中的第五部分。

# 2. `1` 号进程
`1` 号进程又被称为 `init进程`, 由`0`号进入用户模式后创建。
## 2.1 `1` 号进程如何初始化
`1` 号进程通过 `0` 号进程`fork()` 而来, 关于`fork()`，参见：[4. fork](https://github.com/lcdzhao/operating_system/tree/master/theory/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/3.%20%E8%BF%9B%E7%A8%8B/4.%20fork)
## 2.2 `1` 号进程的作用
```c
void init(void)
{
	int pid,i;

	// 挂载根文件系统
	setup((void *) &drive_info);
	(void) open("/dev/tty0",O_RDWR,0);
	(void) dup(0);
	(void) dup(0);
	printf("%d buffers = %d bytes buffer space\n\r",NR_BUFFERS,
		NR_BUFFERS*BLOCK_SIZE);
	printf("Free mem: %d bytes\n\r",memory_end-main_memory_start);
	
	// 启动"/etc/rc"初始化进程2
	if (!(pid=fork())) {
		close(0);
		if (open("/etc/rc",O_RDONLY,0))
			_exit(1);
		execve("/bin/sh",argv_rc,envp_rc);
		_exit(2);
	}
	if (pid>0)
		while (pid != wait(&i))
			/* nothing */;
			
	
	while (1) {
		if ((pid=fork())<0) {
			printf("Fork failed in init\r\n");
			continue;
		}
		
		// 启动Shell
		if (!pid) {
			close(0);close(1);close(2);
			setsid();
			(void) open("/dev/tty0",O_RDWR,0);
			(void) dup(0);
			(void) dup(0);
			_exit(execve("/bin/sh",argv,envp));
		}
		
		// 回收僵尸进程
		while (1)
			if (pid == wait(&i))
				break;
		printf("\n\rchild %d died with code %04x\n\r",pid,i);
		sync();
	}
	_exit(0);	/* NOTE! _exit, not exit() */
}
```
通过上面的源码可以看出。其功能有：
- 挂载根文件系统
- 启动"/etc/rc"初始化进程2
- 启动Shell
- 回收僵尸进程

想了解其具体细节时，可以参看：https://blog.csdn.net/ac_dao_di/article/details/52144608 中的第六部分。

# 3. `0`号`1`号进程及其他进程流转

![processes](../../1.%20启动/README.assets/processes.png)
