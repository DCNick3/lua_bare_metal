# Implementation of setjmp and longjmp functions

.section .text

.globl setjmp
.type setjmp, @function

setjmp:
mov 4(%esp), %edx
mov (%esp), %eax

mov %eax, 0(%edx)
mov %ebp, 4(%edx)
mov %ebx, 8(%edx)
mov %edi, 12(%edx)
mov %esi, 16(%edx)
mov %esp, 20(%edx)

xor %eax, %eax

ret

#.size setjmp, . - setjmp


.globl longjmp
.type longjmp, @function

longjmp:
mov 4(%esp), %edx  # Load jmp_buf addr
mov 8(%esp), %eax  # Set return value

mov 20(%edx), %esp # Return to stack frame

mov 0(%edx), %ebx  # Load return address
mov %ebx, 0(%esp)  # Set it

mov 4(%edx), %ebp
mov 8(%edx), %ebx
mov 12(%edx), %edi
mov 14(%edx), %esi

ret

#.size longjmp, . - longjmp

