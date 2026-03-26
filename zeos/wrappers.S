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
	
	// definimos la direccion de retorno y llamamos a sysenter
	pushl $return_write
	pushl %ebp
	movl %esp, %ebp

	// push de los valores de edx y ecx
	//pushl %edx
	//pushl %ecx
	
	sysenter

return_write:

	popl %ebp
	addl $4, %esp

	cmpl $0, %eax
	jge write_end	// todocorrecto si eax >= 0

	negl %eax		// abs(eax)
	movl %eax, errno
	movl $-1, %eax	// actualizamos el errno y movemos -1 a eax
	call perror

write_end:
	popl %ebx
	popl %ebp
	
	// recuperamos los valores
	//popl %ecx
	//popl %edx

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

ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp

	movl $20, %eax // getpid id

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
