#include <asm/segment.h>
#include <linux/kernel.h>
#include <unistd.h>
#include <string.h>
#include <linux/sched.h>

#define SHM_LIMITED 20
#define SHM_NAME_SIZE_LIMITED 20

struct share_memory
{
	char name[SHM_NAME_SIZE_LIMITED];
	long physic_addr;
	int occupied;
} share_mamerys[SHM_LIMITED];

int shm_index(char *name_in_kernal)
{
	int i;
	for(i=0; i<SHM_LIMITED; i++)
	{
		if(!strcmp(name_in_kernal,share_mamerys[i].name) && share_mamerys[i].occupied ==1)
		{
			return i;
		}
	}
	return -1;
}

char * move_str_from_user_to_kernal(char * name_in_user){
	char name_in_kernal[SHM_NAME_SIZE_LIMITED];
	int i;
	for(i=0; i<SHM_NAME_SIZE_LIMITED; i++)
	{
		name_in_kernal[i] = get_fs_byte(name_in_user+i);
		if(name_in_kernal[i] == '\0') break;
	}
	return name_in_kernal;
}

int sys_shmget(char * name_in_user)
{
	int i,shmid;
	char *name_in_kernal = move_str_from_user_to_kernal(name_in_user);
	shmid = shm_index(name_in_kernal);
	if( shmid != -1) 
	{
		return shmid;
	}
	for(i=0; i<SHM_LIMITED; i++)
	{
		if(share_mamerys[i].occupied == 0)
		{
			strcpy(share_mamerys[i].name,name_in_kernal);
			share_mamerys[i].occupied = 1;
			share_mamerys[i].physic_addr = get_free_page();
			return i;
		}
	}
	printk("SHM Number limited!\n");
	return -1;
}

void * sys_shmat(int shmid)
{
	unsigned long v_addr_start;
	if(share_mamerys[shmid].occupied != 1)
	{
		printk("SHM not exists!\n");
		return -1;
	}
	
	// map share_memory to virtual address of current,
	// and the start of the virtual is current->brk
	put_page(share_mamerys[shmid].physic_addr,current->brk + current->start_code);	
	v_addr_start = current->brk;

	// add 4K(size of page) to current->brk
	current->brk = current->brk + 4 * 1024L;
	
	return (void*)v_addr_start;
}
