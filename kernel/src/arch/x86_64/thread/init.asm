global thread_init_trampoline

thread_init_trampoline:
    mov rdi, rbx
    mov rsi, rbp
    mov rdx, r12
    mov rcx, r13
    mov r8,  r14
    mov r9,  r15
    ret
