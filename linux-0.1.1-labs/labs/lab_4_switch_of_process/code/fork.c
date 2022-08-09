/*
 *  linux/kernel/fork.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *  'fork.c' contains the help-routines for the 'fork' system call
 * (see also system_call.s), and some misc functions ('verify_area').
 * Fork is rather simple, once you get the hang of it, but the memory
 * management can be a bitch. See 'mm/mm.c': 'copy_page_tables()'
 */
#include <errno.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>

extern void write_verify(unsigned long address);

long last_pid=0;

void verify_area(void * addr,int size)
{
	unsigned long start;

	start = (unsigned long) addr;
	size += start & 0xfff;
	start &= 0xfffff000;
	start += get_base(current->ldt[2]);
	while (size>0) {
		size -= 4096;
		write_verify(start);
		start += 4096;
	}
}

int copy_mem(int nr,struct task_struct * p)
{
	unsigned long old_data_base,new_data_base,data_limit;
	unsigned long old_code_base,new_code_base,code_limit;

	code_limit=get_limit(0x0f);
	data_limit=get_limit(0x17);
	old_code_base = get_base(current->ldt[1]);
	old_data_base = get_base(current->ldt[2]);
	if (old_data_base != old_code_base)
		panic("We don't support separate I&D");
	if (data_limit < code_limit)
		panic("Bad data_limit");
	new_data_base = new_code_base = nr * 0x4000000;
	p->start_code = new_code_base;
	set_base(p->ldt[1],new_code_base);
	set_base(p->ldt[2],new_data_base);
	if (copy_page_tables(old_data_base,new_data_base,data_limit)) {
		printk("free_page_tables: from copy_mem\n");
		free_page_tables(new_data_base,data_limit);
		return -ENOMEM;
	}
	return 0;
}


extern long first_run_after_fork(void);
/*
 *  Ok, this is the main fork-routine. It copies the system process
 * information (task[nr]) and sets up the necessary registers. It
 * also copies the data segment in it's entirety.
 */
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	struct task_struct *p;
	int i;
	struct file *f;

	p = (struct task_struct *) get_free_page();
	if (!p)
		return -EAGAIN;


	task[nr] = p;
	*p = *current;/* NOTE! this doesn't copy the supervisor stack */
	p->state = TASK_UNINTERRUPTIBLE;
	p->pid = last_pid;
	p->father = current->pid;
	p->counter = p->priority;
	p->signal = 0;
	p->alarm = 0;
	p->leader = 0;/* process leadership doesn't inherit */
	p->utime = p->stime = 0;
	p->cutime = p->cstime = 0;
	p->start_time = jiffies;

 	long *krnstack = (long *)(PAGE_SIZE + (long)p); 
	
	/* INT0x80，此处将在first_run_after_fork中通过iret使用从而切换回用户代码 */
 	*(--krnstack) = ss & 0xffff; 
	*(--krnstack) = esp;
	*(--krnstack) = eflags;
	*(--krnstack) = cs & 0xffff;
	*(--krnstack) = eip; 
	
	/* 此处将在first_run_after_fork中恢复到寄存器 */
 	*(--krnstack) = ds & 0xffff; 
	*(--krnstack) = es & 0xffff; 
	*(--krnstack) = fs & 0xffff; 
	*(--krnstack) = gs & 0xffff; 

	*(--krnstack) = esi;
	*(--krnstack) = edi;
	*(--krnstack) = edx; 
	
	/* switch_to 弹栈后执行 ret 
	 * ret会继续弹一次栈给 EIP 执行
	 * 所以这次弹出来的要求是函数地址
	 * 如果是经历过切换的老进程，不是新fork出来的，
	 * 那这里的地址会是 schedule() 中 switch_to 的下一条指令
	 * 但由于这里新 fork 出来的，和经历过切换的老进程并不一样，
	 * 即需要恢复寄存器的值，又需要恢复到用户态。
	 * 因此我们将上面这两个任务都放到first_run_after_fork中去执行
	 * 这里将first_run_after_fork的地址压栈，
	 * 那么将会在switch_to最后的ret执行时直接执行first_run_after_fork
	 */
 	*(--krnstack) = (long)first_run_after_fork;
	
	
	/* 此处在switch_to()方法的最后几行代码中恢复到寄存器 */
 	*(--krnstack) = ebp;	//用于用户栈
	*(--krnstack) = ecx;	//ecx与ebx为了switch_to()方法的栈，从用户态传过来
	*(--krnstack) = ebx;	//ecx与ebx为了switch_to()方法的栈，从用户态传过来
	*(--krnstack) = 0;	//在switch_to()方法中赋值给了eax，表示返回值，
				//将在iret指令执行时，返回给用户代码
	 
	/* 进程 PCB 的 内核栈指针 指向 内核栈*/
	p->kernelStack = krnstack; 


	if (last_task_used_math == current)
		__asm__("clts ; fnsave %0"::"m" (p->tss.i387));
	if (copy_mem(nr,p)) {
		task[nr] = NULL;
		free_page((long) p);
		return -EAGAIN;
	}
	for (i=0; i<NR_OPEN;i++)
		if ((f=p->filp[i]))
			f->f_count++;
	if (current->pwd)
		current->pwd->i_count++;
	if (current->root)
		current->root->i_count++;
	if (current->executable)
		current->executable->i_count++;
	set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
	set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
	p->state = TASK_RUNNING;	/* do this last, just in case */
	return last_pid;
}


int find_empty_process(void)
{
	int i;

	repeat:
		if ((++last_pid)<0) last_pid=1;
		for(i=0 ; i<NR_TASKS ; i++)
			if (task[i] && task[i]->pid == last_pid) goto repeat;
	for(i=1 ; i<NR_TASKS ; i++)
		if (!task[i])
			return i;
	return -EAGAIN;
}
