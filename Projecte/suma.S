#include <asm.h>

ENTRY(addASM)
	push %ebp
	mov %esp, %ebp

	mov 8(%ebp), %eax
	add 12(%ebp), %eax

	pop %ebp
	ret
