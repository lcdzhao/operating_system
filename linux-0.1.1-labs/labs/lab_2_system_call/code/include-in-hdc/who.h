/*
 *  linux/lib/who.c
 *
 *  (C) 2022  Without
 */

#define __LIBRARY__
#include <unistd.h>

_syscall2(int,iam,int,fd,int,size)

_syscall1(int,whoami,int,size)

