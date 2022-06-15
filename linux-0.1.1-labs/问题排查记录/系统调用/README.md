# m(address) 的寻址问题

> m(address) 中填写的变量寻址时，将按照 ds:address 的方式进行寻址，因此，如果修改了ds，要格外注意当前的 address 是否属于当前 ds 段。

如：将 用户空间数据 拷贝到 内核空间数据：

错误做法(buf 为内核空间)：

```c
__asm__(
		// 附值edi前进行了修改ds，导致了错误
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
```

正确做法(buf 为内核空间)：

```c
__asm__(
		// 附值edi后，再修改ds
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
		"m"(buf),"m"(name),"c"(name_size));	
```
或者也可以这样：
```
int sys_iam(const char * name,int name_size){
	printk("Step 1 : Sys_iam begin.  name_address: %d , buf_address: %d , name_size: %d,buf: %s.\r\n", name,buf, name_size,buf);
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
