include <asm.h>

ENTRY(addASM)
	push %ebp
	mov %esp, %ebp

	mov 8(%ebp), %eac
	add 12(%ebp), %eac

	pop %ebp
	ret
