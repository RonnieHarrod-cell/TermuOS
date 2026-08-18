.section .text
    .global _start
    .extern main
    .extern _exit

_start:
    xor %rbp, %rbp
    call main
    mov %rax, %rdi
    call _exit
    hlt
