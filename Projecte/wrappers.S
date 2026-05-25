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
	
	movl $10, %eax//10 == sys_gettime
	pushl $return_gettime

	pushl %ebp // fake dynamic link
	movl %esp, %ebp

	sysenter

return_gettime:
	popl %ebp
	addl $4, %esp

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

ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp

	movl $2, %eax // sys_fork id

	pushl $return_fork // A aqui volverá solo el padre, ya q el hijo irá por clock y sys_handler
	pushl %ebp //Fake dynamic link
	movl %esp, %ebp

	sysenter //enters sys_fork

return_fork: // despues de sys_fork
	popl %ebp
	addl $4, %esp

	cmpl $0, %eax //si no hay ningun error, finaliza
	jge fork_end

	negl %eax	//si hay algun error, modifica errno
	movl %eax, errno
	movl $-1, %eax

fork_end:
	popl %ebp
	ret

ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	
	movl $1, %eax//1 == sys_exit syscall
	pushl $ret_exit

	pushl %ebp // fake dynamic link
	movl %esp, %ebp

	sysenter

ret_exit:
	popl %ebp
	popl %ebp
	ret

ENTRY(block)
	pushl %ebp
	movl %esp, %ebp

	movl $5, %eax //5 == sys_block syscall
	pushl $ret_block

	pushl %ebp
	movl %esp, %ebp

	sysenter

ret_block:
	pop %ebp
	addl $4, %esp //quitamos valor $ret_block
	cmpl $0, %eax
	jge block_end
	neg %eax
	movl %eax, errno
	movl $-1, %eax

block_end:
	popl %ebp
	ret

ENTRY(unblock)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx //PID pasado como parametro
	movl $6, %eax //6 == unblock syscall
	pushl $ret_unblock

	pushl %ebp
	movl %esp, %ebp

	sysenter

ret_unblock:
	popl %ebp
	addl $4, %esp //quitamos $ret_unlock
	cmpl $0, %eax
	jge unblock_end
	neg %eax
	movl %eax, errno
	movl $-1, %eax

unblock_end:
	popl %ebp
	ret

ENTRY(read)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx //char* b pasado como parametro
	movl 12(%ebp), %ecx //int maxchars

	movl $7, %eax// syscall sys_read
	pushl $ret_read

	pushl %ebp
	movl %esp, %ebp
	sysenter

ret_read:
	pop %ebp
	addl $4, %esp

	cmpl $0, %eax
	jge read_end
	neg %eax
	movl %eax, errno
	movl $-1, %eax

read_end:
	popl %ebp
	ret


ENTRY(set_color)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx //foreground
	movl 12(%ebp), %ecx //background

	movl $8, %eax //syscall sys_set_color
	pushl $ret_set_color

	pushl %ebp
	movl %esp, %ebp
	sysenter

ret_set_color:
	pop %ebp
	addl $4, %esp

	cmpl $0, %eax
	jge set_color_end
	neg %eax
	movl %eax, errno
	movl $-1, %eax

set_color_end:
	popl %ebp
	ret

ENTRY(gotoxy)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx //x
	movl 12(%ebp), %ecx //y

	movl $9, %eax //syscall sys_gotoxy
	pushl $ret_gotoxy

	pushl %ebp
	movl %esp, %ebp
	sysenter

ret_gotoxy:
	pop %ebp
	addl $4, %esp

	cmpl $0, %eax
	jge gotoxy_end
	neg %eax
	movl %eax, errno
	movl $-1, %eax

gotoxy_end:
	popl %ebp
	ret


ENTRY(shmat)
	pushl %ebp
	movl %esp, %ebp

	movl 8(%ebp), %edx //id
	movl 12(%ebp), %ecx //addr

	movl $3, %eax //syscall sys_shmat 
	pushl $ret_shmat

	pushl %ebp
	movl %esp, %ebp
	sysenter

ret_shmat:
	pop %ebp
	addl $4, %esp

	cmpl $0, %eax
	jge shmat_end 
	neg %eax
	movl %eax, errno
	movl $-1, %eax

shmat_end:
	popl %ebp
	ret
