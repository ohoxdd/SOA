#include <asm.h>
#include <segment.h>

ENTRY (sys_call_table)
	.long sys_ni_syscall//0
	.long sys_exit//1
	.long sys_fork//2
	.long sys_shmat//3
	.long sys_write		//4
	.long sys_block//5
	.long sys_unblock//6
	.long sys_read//7
	.long sys_set_color//8
	.long sys_gotoxy//9
	.long sys_gettime	//10
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_ni_syscall
	.long sys_getpid	//20
.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)/4
