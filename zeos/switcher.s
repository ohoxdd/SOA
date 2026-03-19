# 0 "switcher.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "switcher.S"
# 1 "include/asm.h" 1
# 2 "switcher.S" 2

.globl task_switch; .type task_switch, @function; .align 0; task_switch:
 push %ebp
 mov %esp, %ebp

    push %esi
    push %edi
    push %ebx

    push 0x8(%ebp)
    call inner_task_switch
    add $4, %esp

    pop %ebx
    pop %edi
    pop %esi
    pop %ebp
    ret

.globl task_switch_part2; .type task_switch_part2, @function; .align 0; task_switch_part2:
    push %ebp
    mov %esp, %ebp

    movl 0x8(%ebp), %eax
    movl %ebp, (%eax)
    movl 0xc(%ebp) , %esp
    pop %ebp
    ret
