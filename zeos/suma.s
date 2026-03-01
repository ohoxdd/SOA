# 0 "suma.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "suma.S"
include <asm.h>

ENTRY(addASM)
 push %ebp
 mov %esp, %ebp

 mov 8(%ebp), %eac
 add 12(%ebp), %eac

 pop %ebp
 ret
