# C语言变量内存位置
## 全局变量
该变量在数据段中(即为全局变量)，需要按照 ds:address 的方式进行寻址。由GCC在编译时，自动识别然后设定寻址方式。
## 方法内的变量
该变量在栈中(即在方法中定义)，需要按照ss:address的方式进行寻址。由GCC在编译时，自动识别然后设定寻址方式。
# m指令寻址方式
> m(address) 中填写的变量寻址时，具体的寻址方式由GCC在编译时，自动识别然后设定寻址方式：
> - 如果该引用在栈中(即在方法中定义)，则按照ss:address的方式进行寻址。
> 
> - 如果该内存在数据段中(即为全局变量)，则将按照 ds:address 的方式进行寻址。
> 
> 因此，如果要对数据段中的数据进行操作时，如果修改了ds，要格外注意当前的 address 是否属于当前 ds 段。

## 示例
如：将 用户空间数据 拷贝到 内核空间数据：

#### 错误做法，赋值edi前，就将ds修改，导致错误(buf 为内核空间)：

```c
static char buf[24];

int sys_iam(const char * name,int name_size){
	printk("Step 1 : Sys_iam begin.  name_address: %d , buf_address: %d , name_size: %d,buf: %s.\r\n", name,buf, name_size,buf);
	__asm__(
		// 赋值edi前，就将ds修改，导致错误
		"movl $0x17,%%edx\n\t"
		"mov %%dx,%%ds\n\t"
		"movl %0,%%edi\n\t"
		"movl %1,%%esi\n\t"
		// copy
		"cld\n\t"
		"rep\n\t"
		"movsb\n\t"
		"movl $0x10,%%edx\n\t"
		"mov %%dx,%%ds\n\t"
		::
		"m"(buf),"m"(name),"c"(name_size));	

	printk("Step 2 : Sys_iam end.  buf: %s .\r\n", buf);
	return 1;
}
```

##### GCC ...c -S 汇编后的代码
```
```

#### 正确做法，赋值edi后，再修改ds，没有错误(buf 为内核空间)：

```c
static char buf[24];

int sys_iam(const char * name,int name_size){
	printk("Step 1 : Sys_iam begin.  name_address: %d , buf_address: %d , name_size: %d,buf: %s.\r\n", name,buf, name_size,buf);
	__asm__(
		// 赋值edi后，再修改ds，没有错误
		"movl %0,%%edi\n\t"
		"movl $0x17,%%edx\n\t"
		"mov %%dx,%%ds\n\t"
		"movl %1,%%esi\n\t"
		// copy
		"cld\n\t"
		"rep\n\t"
		"movsb\n\t"
		"movl $0x10,%%edx\n\t"
		"mov %%dx,%%ds\n\t"
		::
		"m"(buf_address),"m"(name),"c"(name_size));	

	printk("Step 2 : Sys_iam end.  buf: %s .\r\n", buf);
	return 1;
}
```

##### GCC ...c -S 汇编后的代码
```
```

#### 或者也可以这样(即将`buf_address`先拷贝到栈中)：
```c
int sys_iam(const char * name,int name_size){
	printk("Step 1 : Sys_iam begin.  name_address: %d , buf_address: %d , name_size: %d,buf: %s.\r\n", name,buf, name_size,buf);
	//将`buf_address`先拷贝到栈中
	int buf_address = buf;
	__asm__(
		// src
		"movl $0x17,%%edx\n\t"
		"mov %%dx,%%ds\n\t"
		"movl %0,%%edi\n\t"
		"movl %1,%%esi\n\t"
		// copy
		"cld\n\t"
		"rep\n\t"
		"movsb\n\t"
		"movl $0x10,%%edx\n\t"
		"mov %%dx,%%ds\n\t"
		::
		"m"(buf_address),"m"(name),"c"(name_size));	

	printk("Step 2 : Sys_iam end.  buf: %s .\r\n", buf);
	return 1;
}
```
##### GCC ...c -S 汇编后的代码
```asm
	.file	"who.c"
	.local	buf
	.comm	buf,24,16
	.section	.rodata

.align 8
...

sys_iam:
.LFB0:
	!...省略pritk部分
	!将buf的值入栈
	movl	$buf, %eax
	movl	%eax, -4(%rbp)
	movl	-28(%rbp), %eax
	movl	%eax, %ecx
#APP
# 16 "who.c" 1
	movl $0x17,%edx
	mov %dx,%ds
	!从栈中给edi赋值buf
	movl -4(%rbp),%edi
	!从栈中给esi赋值name
	movl -24(%rbp),%esi
	cld
	rep
	movsb
	movl $0x10,%edx
	mov %dx,%ds
	!...省略其他代码
```
