#include <asm.h>

ENTRY(task_switch)
	push %ebp
	mov %esp, %ebp

    push %esi
    push %edi
    push %ebx
// llama a sched.c inner task_switch
    push 0x8(%ebp)
    call inner_task_switch
    add $4, %esp
// vuelve de task_switch_part2, llamado por inner_task_switch
    pop %ebx
    pop %edi
    pop %esi
    pop %ebp
    ret

ENTRY(task_switch_part2)
    push %ebp
    mov %esp, %ebp

    movl 0x8(%ebp), %eax
    movl %ebp, (%eax)
    movl 0xc(%ebp) , %esp
    pop %ebp
    ret





