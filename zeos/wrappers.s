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
