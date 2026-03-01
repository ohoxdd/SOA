#include <asm.h>

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx
	
	// pasamos parametros
	movl 8(%ebp), %edx	// edx <- fd
	movl 12(%ebp), %ecx	// ecx <- buffer
	movl 16(%ebp), %ebx	// ebx <- size
	
	// id de la syscall (write = 4)
	movl $4, %eax
	
	// generamos el trap
	int $0x80

	cmpl $0, %eax
	jge write_end	// todocorrecto si eax >= 0

	negl %eax		// abs(eax)
	movl %eax, errno
	movl $-1, %eax	// actualizamos el errno y movemos -1 a eax

write_end:
	popl %ebx
	popl %ebp
	ret

ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp

	movl $10, %eax // id gettime
	int $0x80
	
	cmpl $0, %eax
	jge gettime_end

	negl %eax
	movl %eax, errno
	movl $-1, %eax

gettime_end:
	popl %ebp
	ret
