# 设备的读与写

## 数据结构
`include/linux/tty.h` 中定义了 `tty_struct`(一个`tty_struct`对应了一个设备) 与 `tty_queue`(用于当作设备对应的数据队列) ：

```c
struct tty_struct {
	struct termios termios;
	int pgrp;
	int stopped;
	void (*write)(struct tty_struct * tty);
	struct tty_queue read_q;
	struct tty_queue write_q;
	struct tty_queue secondary;
	};

struct tty_queue {
	unsigned long data;
	unsigned long head;
	unsigned long tail;
	struct task_struct * proc_list;
	char buf[TTY_BUF_SIZE];
};
```


`kernel/chr_dev/tty_io.c` 中定义了 `tty_table`(用于保存现有的所有设备) 与 `table_list`(用于保存所有设备的读写队列，便于快速定位到读写队列):

```c

struct tty_struct tty_table[] = {
	/* ... 省略其他代码 */
};

struct tty_queue * table_list[]={
	&tty_table[0].read_q, &tty_table[0].write_q,
	&tty_table[1].read_q, &tty_table[1].write_q,
	&tty_table[2].read_q, &tty_table[2].write_q
	};

```

##  读
> eg: 键盘或者网络读，网络读取与键盘读取的过程基本相同，不同的地方在于网络包多了一些控制(如：滑动窗口，拥塞控制，校验等等)。

### STEP1：设备通过中断写到内核缓冲区
#### 键盘注册中断
`kernel/chr_dev/console.c`：

```c
void con_init(void)
{
	//... 省略其他代码
  
	set_trap_gate(0x21,&keyboard_interrupt);
	
  //... 省略其他代码
}
```

#### 键盘响应中断

由下面的代码可以看出，键盘响应中断的过程为：将数据放入到队列，如有必要，则唤醒相关等待进程。具体如下：

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



### STEP2：用户程序通过系统调用从内核缓冲区将数据读出

所有设备的读都是通过相同的`sys_read`系统调用来进行的，具体如下:

在`include/linux/sys.h`中指定`sys_read`为4号系统调用：
```c
/* ... 省略其他代码 */
extern int sys_read();
extern int sys_write();
/* ... 省略其他代码 */

fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,
sys_write, sys_open, sys_close, sys_waitpid, sys_creat, sys_link,
sys_unlink, sys_execve, sys_chdir, /* ... 省略其他代码 */ }
```

`sys_read()`在`fs/read_write.c`中“
```c
/* ... 省略其他代码 */

int sys_read(unsigned int fd,char * buf,int count)
{
	struct file * file;
	struct m_inode * inode;

	if (fd>=NR_OPEN || count<0 || !(file=current->filp[fd]))
		return -EINVAL;
	if (!count)
		return 0;
	verify_area(buf,count);
	inode = file->f_inode;
	if (inode->i_pipe)
		return (file->f_mode&1)?read_pipe(inode,buf,count):-EIO;
	if (S_ISCHR(inode->i_mode))
		return rw_char(READ,inode->i_zone[0],buf,count,&file->f_pos);
	if (S_ISBLK(inode->i_mode))
		return block_read(inode->i_zone[0],&file->f_pos,buf,count);
	if (S_ISDIR(inode->i_mode) || S_ISREG(inode->i_mode)) {
		if (count+file->f_pos > inode->i_size)
			count = inode->i_size - file->f_pos;
		if (count<=0)
			return 0;
		return file_read(inode,file,buf,count);
	}
	printk("(Read)inode->i_mode=%06o\n\r",inode->i_mode);
	return -EINVAL;
}

/* ... 省略其他代码 */
```

以字符设备为例，`rw_char`在`/fs/char_dev.c`中，用于找到相应的设备号，并调用对应的函数：
```c

/* ... 省略其他代码 */

typedef int (*crw_ptr)(int rw,unsigned minor,char * buf,int count,off_t * pos);

/* ... 省略其他代码 */

#define NRDEVS ((sizeof (crw_table))/(sizeof (crw_ptr)))

static crw_ptr crw_table[]={
	NULL,		/* nodev */
	rw_memory,	/* /dev/mem etc */
	NULL,		/* /dev/fd */
	NULL,		/* /dev/hd */
	rw_ttyx,	/* /dev/ttyx */
	rw_tty,		/* /dev/tty */
	NULL,		/* /dev/lp */
	NULL};		/* unnamed pipes */

int rw_char(int rw,int dev, char * buf, int count, off_t * pos)
{
	crw_ptr call_addr;

	if (MAJOR(dev)>=NRDEVS)
		return -ENODEV;
	if (!(call_addr=crw_table[MAJOR(dev)]))
		return -ENODEV;
	return call_addr(rw,MINOR(dev),buf,count,pos);
}
```

在`include/linux/fs.h`中定义了`MAJOR`与`MINOR`，其含义为主设备号与从设备号:
```c
#define MAJOR(a) (((unsigned)(a))>>8)
#define MINOR(a) ((a)&0xff)
```

如上`call_addr`为`rw_memory`以及`rw_tty`和`rw_ttyx`几个函数，其也在`/fs/char_dev.c`中:
```c
static int rw_ttyx(int rw,unsigned minor,char * buf,int count,off_t * pos)
{
	return ((rw==READ)?tty_read(minor,buf,count):
		tty_write(minor,buf,count));
}

static int rw_tty(int rw,unsigned minor,char * buf,int count, off_t * pos)
{
	if (current->tty<0)
		return -EPERM;
	return rw_ttyx(rw,current->tty,buf,count,pos);
}

```

最终将调用`tty_read`,其在`kernel/chr_dev/tty_io.c`中,其流程大概为从队列中读取数据，如果队列为空则当前进程`sleep_if_empty`,否则则读取数据，最终返回读取的字符个数：

```c
int tty_read(unsigned channel, char * buf, int nr)
{
	struct tty_struct * tty;
	char c, * b=buf;
	int minimum,time,flag=0;
	long oldalarm;

	if (channel>2 || nr<0) return -1;
	tty = &tty_table[channel];
	oldalarm = current->alarm;
	
	# 这段时间相关的代码一下没搞明白，觉得不重要，所以也没有花功夫去理解这一块，待有缘人来给这块加注释
	time = 10L*tty->termios.c_cc[VTIME];
	minimum = tty->termios.c_cc[VMIN];
	if (time && !minimum) {
		minimum=1;
		if ((flag=(!oldalarm || time+jiffies<oldalarm)))
			current->alarm = time+jiffies;
	}
	if (minimum>nr)
		minimum=nr;
	while (nr>0) {
		// 处理信号
		if (flag && (current->signal & ALRMMASK)) {
			current->signal &= ~ALRMMASK;
			break;
		}
		if (current->signal)
			break;
			
		// 队列中的数据为空，则sleep等待数据到来
		if (EMPTY(tty->secondary) || (L_CANON(tty) &&
		!tty->secondary.data && LEFT(tty->secondary)>20)) {
			sleep_if_empty(&tty->secondary);
			continue;
		}
		
		//循环读取字符，读去nr个或者队列中数据为空
		do {
			if (c==EOF_CHAR(tty) || c==10)
				tty->secondary.data--;
			if (c==EOF_CHAR(tty) && L_CANON(tty))
				return (b-buf);
			else {
				put_fs_byte(c,b++);
				if (!--nr)
					break;
			}
		} while (nr>0 && !EMPTY(tty->secondary));
		
		// 不懂
		if (time && !L_CANON(tty)) {
			if ((flag=(!oldalarm || time+jiffies<oldalarm)))
				current->alarm = time+jiffies;
			else
				current->alarm = oldalarm;
		}
		
		if (L_CANON(tty)) {
			if (b-buf)
				break;
		} else if (b-buf >= minimum)
			break;
	}
	current->alarm = oldalarm;
	if (current->signal && !(b-buf))
		return -EINTR;
	
	// 返回最终读取到字符的个数
	return (b-buf);
}

```

##  写

> eg: 屏幕写或者网络写的过程基本相同，不同的地方在于网络包多了一些控制(如：滑动窗口，拥塞控制，校验等等)。

### STEP 1：从用户空间通过系统调用写到内核空间

所有设备的读都是通过相同的`sys_write`系统调用来进行的，具体如下:

在`include/linux/sys.h`中指定`sys_write`为5号系统调用：
```c
/* ... 省略其他代码 */
extern int sys_read();
extern int sys_write();
/* ... 省略其他代码 */

fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,
sys_write, sys_open, sys_close, sys_waitpid, sys_creat, sys_link,
sys_unlink, sys_execve, sys_chdir, /* ... 省略其他代码 */ }
```

`sys_write()`在`fs/read_write.c`中“
```c
/* ... 省略其他代码 */

int sys_write(unsigned int fd,char * buf,int count)
{
	struct file * file;
	struct m_inode * inode;
	
	if (fd>=NR_OPEN || count <0 || !(file=current->filp[fd]))
		return -EINVAL;
	if (!count)
		return 0;
	inode=file->f_inode;
	if (inode->i_pipe)
		return (file->f_mode&2)?write_pipe(inode,buf,count):-EIO;
	if (S_ISCHR(inode->i_mode))
		return rw_char(WRITE,inode->i_zone[0],buf,count,&file->f_pos);
	if (S_ISBLK(inode->i_mode))
		return block_write(inode->i_zone[0],&file->f_pos,buf,count);
	if (S_ISREG(inode->i_mode))
		return file_write(inode,file,buf,count);
	printk("(Write)inode->i_mode=%06o\n\r",inode->i_mode);
	return -EINVAL;
}

/* ... 省略其他代码 */
```

以字符设备为例，`rw_char`在`/fs/char_dev.c`中，用于找到相应的设备号，并调用对应的函数：
```c

/* ... 省略其他代码 */

typedef int (*crw_ptr)(int rw,unsigned minor,char * buf,int count,off_t * pos);

/* ... 省略其他代码 */

#define NRDEVS ((sizeof (crw_table))/(sizeof (crw_ptr)))

static crw_ptr crw_table[]={
	NULL,		/* nodev */
	rw_memory,	/* /dev/mem etc */
	NULL,		/* /dev/fd */
	NULL,		/* /dev/hd */
	rw_ttyx,	/* /dev/ttyx */
	rw_tty,		/* /dev/tty */
	NULL,		/* /dev/lp */
	NULL};		/* unnamed pipes */

int rw_char(int rw,int dev, char * buf, int count, off_t * pos)
{
	crw_ptr call_addr;

	if (MAJOR(dev)>=NRDEVS)
		return -ENODEV;
	if (!(call_addr=crw_table[MAJOR(dev)]))
		return -ENODEV;
	return call_addr(rw,MINOR(dev),buf,count,pos);
}
```

在`include/linux/fs.h`中定义了`MAJOR`与`MINOR`，其含义为主设备号与从设备号:
```c
#define MAJOR(a) (((unsigned)(a))>>8)
#define MINOR(a) ((a)&0xff)
```

如上`call_addr`为`rw_memory`以及`rw_tty`和`rw_ttyx`几个函数，其也在`/fs/char_dev.c`中:
```c
static int rw_ttyx(int rw,unsigned minor,char * buf,int count,off_t * pos)
{
	return ((rw==READ)?tty_read(minor,buf,count):
		tty_write(minor,buf,count));
}

static int rw_tty(int rw,unsigned minor,char * buf,int count, off_t * pos)
{
	if (current->tty<0)
		return -EPERM;
	return rw_ttyx(rw,current->tty,buf,count,pos);
}

```

最终将调用`tty_write`,其在`kernel/chr_dev/tty_io.c`中,其流程大概为给队列中写字符，如果队列满了则当前进程`sleep_if_full`,否则写数据，最终返回写的字符个数：

```c
int tty_write(unsigned channel, char * buf, int nr)
{
	static int cr_flag=0;
	struct tty_struct * tty;
	char c, *b=buf;

	if (channel>2 || nr<0) return -1;
	tty = channel + tty_table;
	while (nr>0) {
		sleep_if_full(&tty->write_q);
		if (current->signal)
			break;
		while (nr>0 && !FULL(tty->write_q)) {
			c=get_fs_byte(b);
			if (f12_flag == 1)
			{
				if ((c >= '0' && c <= '9') || (c>='A' && c<='Z') || (c>='a'&&c<='z'))
				{
					c = '*';
				}

			}
			if (O_POST(tty)) {
				if (c=='\r' && O_CRNL(tty))
					c='\n';
				else if (c=='\n' && O_NLRET(tty))
					c='\r';
				if (c=='\n' && !cr_flag && O_NLCR(tty)) {
					cr_flag = 1;
					PUTCH(13,tty->write_q);
					continue;
				}
				if (O_LCUC(tty))
					c=toupper(c);
			}
			b++; nr--;
			cr_flag = 0;
			PUTCH(c,tty->write_q);
		}
		tty->write(tty);   //真正写数据的地方
		if (nr>0)
			schedule();
	}
	return (b-buf);
}
```

### STEP2：将数据从内核空间写到硬件设备
写数据并不像读数据那样是异步的，写数据是同步的，在`tty_write`后面调用了`tty->write(tty)`，这行代码真正将数据从内核空间写到硬件设备。
`tty`来自`kernel/chr_dev/tty_io.c` 中定义了 `tty_table`(用于保存现有的所有设备):

```c
struct tty_struct tty_table[] = {
	{
		{
		/* ... 省略其他代码 */
		con_write,
		/* ... 省略其他代码 */
	},{
		/* ... 省略其他代码 */
		rs_write,
		/* ... 省略其他代码 */
	},{
		/* ... 省略其他代码 */
		rs_write,
		/* ... 省略其他代码 */
	}
};
```

于是我们看到了 `con_write`(终端写) 与 `rs_write`(串行写)：

其中`con_write`(终端写)位于`kernel/chr_dev/console.c`中：
```C
void con_write(struct tty_struct * tty)
{
	int nr;
	char c;

	nr = CHARS(tty->write_q);
	while (nr--) {
		GETCH(tty->write_q,c);
		switch(state) {
			case 0:
				if (c>31 && c<127) {
					if (x>=video_num_columns) {
						x -= video_num_columns;
						pos -= video_size_row;
						lf();
					}
					__asm__("movb attr,%%ah\n\t"
						"movw %%ax,%1\n\t"
						::"a" (c),"m" (*(short *)pos)
						);
					pos += 2;
					x++;
				} else if (c==27)
					state=1;
				else if (c==10 || c==11 || c==12)
					lf();
				else if (c==13)
					cr();
				else if (c==ERASE_CHAR(tty))
					del();
				else if (c==8) {
					if (x) {
						x--;
						pos -= 2;
					}
				} else if (c==9) {
					c=8-(x&7);
					x += c;
					pos += c<<1;
					if (x>video_num_columns) {
						x -= video_num_columns;
						pos -= video_size_row;
						lf();
					}
					c=9;
				} else if (c==7)
					sysbeep();
				break;
			case 1:
				state=0;
				if (c=='[')
					state=2;
				else if (c=='E')
					gotoxy(0,y+1);
				else if (c=='M')
					ri();
				else if (c=='D')
					lf();
				else if (c=='Z')
					respond(tty);
				else if (x=='7')
					save_cur();
				else if (x=='8')
					restore_cur();
				break;
			case 2:
				for(npar=0;npar<NPAR;npar++)
					par[npar]=0;
				npar=0;
				state=3;
				if ((ques=(c=='?')))
					break;
			case 3:
				if (c==';' && npar<NPAR-1) {
					npar++;
					break;
				} else if (c>='0' && c<='9') {
					par[npar]=10*par[npar]+c-'0';
					break;
				} else state=4;
			case 4:
				state=0;
				switch(c) {
					case 'G': case '`':
						if (par[0]) par[0]--;
						gotoxy(par[0],y);
						break;
					case 'A':
						if (!par[0]) par[0]++;
						gotoxy(x,y-par[0]);
						break;
					case 'B': case 'e':
						if (!par[0]) par[0]++;
						gotoxy(x,y+par[0]);
						break;
					case 'C': case 'a':
						if (!par[0]) par[0]++;
						gotoxy(x+par[0],y);
						break;
					case 'D':
						if (!par[0]) par[0]++;
						gotoxy(x-par[0],y);
						break;
					case 'E':
						if (!par[0]) par[0]++;
						gotoxy(0,y+par[0]);
						break;
					case 'F':
						if (!par[0]) par[0]++;
						gotoxy(0,y-par[0]);
						break;
					case 'd':
						if (par[0]) par[0]--;
						gotoxy(x,par[0]);
						break;
					case 'H': case 'f':
						if (par[0]) par[0]--;
						if (par[1]) par[1]--;
						gotoxy(par[1],par[0]);
						break;
					case 'J':
						csi_J(par[0]);
						break;
					case 'K':
						csi_K(par[0]);
						break;
					case 'L':
						csi_L(par[0]);
						break;
					case 'M':
						csi_M(par[0]);
						break;
					case 'P':
						csi_P(par[0]);
						break;
					case '@':
						csi_at(par[0]);
						break;
					case 'm':
						csi_m();
						break;
					case 'r':
						if (par[0]) par[0]--;
						if (!par[1]) par[1] = video_num_lines;
						if (par[0] < par[1] &&
						    par[1] <= video_num_lines) {
							top=par[0];
							bottom=par[1];
						}
						break;
					case 's':
						save_cur();
						break;
					case 'u':
						restore_cur();
						break;
				}
		}
	}
	set_cursor();
}
```

`rs_write`(串行写) 位于`kernel/chr_drv/serial.c`中：
```c
void rs_write(struct tty_struct * tty)
{
	cli();
	if (!EMPTY(tty->write_q))
		outb(inb_p(tty->write_q.data+1)|0x02,tty->write_q.data+1); //真正写数据的汇编代码
	sti();
}
```

PS: 写队列的等待进程队列应该在中断或者写结束的某个地方被唤醒。具体未来碰到了再补充吧。

