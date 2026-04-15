# 0 "wrappers.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrappers.S"
# 1 "include/asm.h" 1
# 2 "wrappers.S" 2

.globl write; .type write, @function; .align 0; write:
 pushl %ebp
 movl %esp, %ebp
 pushl %ebx


 movl 8(%ebp), %edx
 movl 12(%ebp), %ecx
 movl 16(%ebp), %ebx


 movl $4, %eax


 pushl $return_write
 pushl %ebp
 movl %esp, %ebp





 sysenter

return_write:

 popl %ebp
 addl $4, %esp

 cmpl $0, %eax
 jge write_end

 negl %eax
 movl %eax, errno
 movl $-1, %eax
 call perror

write_end:
 popl %ebx
 popl %ebp





 ret

.globl gettime; .type gettime, @function; .align 0; gettime:
 pushl %ebp
 movl %esp, %ebp

 movl $10, %eax
 int $0x80

 cmpl $0, %eax
 jge gettime_end

 negl %eax
 movl %eax, errno
 movl $-1, %eax

gettime_end:
 popl %ebp
 ret

.globl getpid; .type getpid, @function; .align 0; getpid:
 pushl %ebp
 movl %esp, %ebp

 movl $20, %eax

 pushl $return_getpid
 pushl %ebp
 movl %esp, %ebp

 sysenter

return_getpid:
 popl %ebp
 addl $4, %esp

 cmpl $0, %eax
 jge getpid_end

 negl %eax
 movl %eax, errno
 movl $-1, %eax

getpid_end:
 popl %ebp
 ret

.globl fork; .type fork, @function; .align 0; fork:
 pushl %ebp
 movl %esp, %ebp

 movl $2, %eax

 pushl $return_fork
 pushl %ebp
 movl %esp, %ebp

 sysenter

return_fork:
 popl %ebp
 addl $4, %esp

 cmpl $0, %eax
 jge fork_end

 negl %eax
 movl %eax, errno
 movl $-1, %eax

fork_end:
 popl %ebp
 ret

.globl exit; .type exit, @function; .align 0; exit:
 pushl %ebp
 movl %esp, %ebp

 movl $1, %eax
 pushl $ret_exit

 pushl %ebp
 movl %esp, %ebp

 sysenter

ret_exit:
 popl %ebp
 popl %ebp
 ret

.globl block; .type block, @function; .align 0; block:
 pushl %ebp
 movl %esp, %ebp

 movl $5, %eax
 pushl $ret_block

 pushl %ebp
 movl %esp, %ebp

 sysenter

ret_block:
 cmpl $0, %eax
 jge block_end
 neg %eax
 movl %eax, errno
 movl $-1, %eax

block_end:
 popl %ebp
 popl %ebp
 ret

.globl unblock; .type unblock, @function; .align 0; unblock:
 pushl %ebp
 movl %esp, %ebp

 movl 8(%ebp), %edx
 movl $6, %eax
 pushl $ret_unblock

 pushl %ebp
 movl %esp, %ebp

 sysenter

ret_unblock:
 popl %ebp
 popl %edx
 cmpl $0, %eax
 jge unblock_end
 neg %eax
 movl %eax, errno
 movl $-1, %eax

unblock_end:
 popl %ebp
 ret
