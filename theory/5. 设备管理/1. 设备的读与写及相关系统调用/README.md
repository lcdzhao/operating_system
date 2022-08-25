# 设备的读与写及相关系统调用
##  读

> eg: 键盘或者网络读，网络读取与键盘读取的过程基本相同，不同的地方在于网络包多了一些控制(如：滑动窗口，拥塞控制，校验等等)。

### STEP1：从设备读到内核缓冲区
#### 流程

#### 键盘读取相关源码

##### 1） 注册中断
`kernel/chr_dev/console.c`：

```c
void con_init(void)
{
	//... 省略其他代码
  
	set_trap_gate(0x21,&keyboard_interrupt);
	
  //... 省略其他代码
}
```

##### 2） 将数据放入到队列，如有必要，则唤醒相关等待进程

`kernel/chr_dev/keyboard.S`:

```asm
keyboard_interrupt:
	/* ... 省略其他代码 */
	call key_table(,%eax,4)
	/* ... 省略其他代码 */
  

key_table:
	.long none,do_self,do_self,do_self	/* 00-03 s0 esc 1 2 */
	.long do_self,do_self,do_self,do_self	/* 04-07 3 4 5 6 */
	.long do_self,do_self,do_self,do_self	/* 08-0B 7 8 9 0 */
	.long do_self,do_self,do_self,do_self	/* 0C-0F + ' bs tab */
	.long do_self,do_self,do_self,do_self	/* 10-13 q w e r */
	.long do_self,do_self,do_self,do_self	/* 14-17 t y u i */
  /* ... 省略其他代码 */
  
do_self:
  /* ... 省略其他代码 */
	call put_queue
  /* ... 省略其他代码 */
  
put_queue:
  /* ... 省略其他代码 */
  /* 入队列 */
  movl %ecx,head(%edx)
  /* 唤醒队首进程，唤醒队首进程即唤醒了所有的进程，详见：[sleep_on 与 wake_up] */
	movl proc_list(%edx),%ecx	# edx 为 table_list(其在"linux/kernel/tty_io.c"被定义为读写队列的集合)，proc_list(%edx)即为队首进程的状态。
	testl %ecx,%ecx			# 测试 ecx 是否为0,即：队首进程是否运行，0为运行。
	je 3f				# 如果运行则直接跳到3。
	movl $0,(%ecx)			# 如果没有运行则将0赋值队首进程的状态，即唤醒其。
3:	popl %edx
	popl %ecx
	ret
  /* ... 省略其他代码 */
```
> PS: 唤醒队首进程即唤醒了所有的进程，详见：[sleep_on 与 wake_up](https://github.com/lcdzhao/operating_system/tree/master/theory/3.%20%E8%BF%9B%E7%A8%8B/5.%20%E8%BF%9B%E7%A8%8B%E7%94%9F%E5%91%BD%E5%91%A8%E6%9C%9F%E5%8F%8A%E8%B0%83%E5%BA%A6#sleep_on)



### STEP2：用户程序从内核缓冲区将数据读出
#### 流程

#### 相关源码
