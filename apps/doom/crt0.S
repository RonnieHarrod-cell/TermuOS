    .section .text
    .global _start
    .extern main
    .extern exit

_start:
    xor %rbp, %rbp
    /* main(void) — argc/argv unused; main.c builds its own argv */
    call main
    mov %rax, %rdi
    call exit
1:
    hlt
    jmp 1b
