/*
 *  linux/kernel/who.c
 *
 *  (C) 2022  Without
 */

#include <linux/kernel.h>

static char buf[24];

int sys_iam(const char * name,int name_size){
	printk("Step 1 : Sys_iam begin.  name_address: %d , buf_address: %d , name_size: %d,buf: %s.\r\n", name,buf, name_size,buf);
	__asm__(
		//mov before change eds
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

	printk("Step 2 : Sys_iam end.  buf: %s .\r\n", buf);
	return 1;
}

int sys_whoami(unsigned int size){
	printk("sys_whoami: %s.\r\n",  buf);
	return 1;
}
