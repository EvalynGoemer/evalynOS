global thread_switch
global thread_switch_user
global switch_to_user

section .text
thread_switch:
    pushfq
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    mov [rdi], rsp
    mov rsp, rsi

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    popfq

    ret

USER_STACK_TOP equ 0x0000000080000000

switch_to_user:
    mov ax, 0x20 | 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x20 | 3
    mov rax, USER_STACK_TOP
    push rax
    push 0x200
    push 0x28 | 3
    push 0x4000
    iretq
