.section .text
    .global _start
    .extern main

_start:
    mov (%rsp), %rdi
    lea 8(%rsp), %rsi
    call main
    mov %rax, %rdi
    mov $60, %rax
    syscall
1:
    jmp 1b
