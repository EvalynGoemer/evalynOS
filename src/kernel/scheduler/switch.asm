global thread_switch
global thread_switch_user
global switch_to_user

section .text
thread_switch:
    pushfq
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
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
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    popfq

    ret

USER_STACK_TOP equ 0x3000
switch_to_user:
    mov ax, 0x33
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x33
    push USER_STACK_TOP
    pushfq
    pop rax
    or rax, 0x200
    push rax
    push 0x2B
    push 0x4000
    iretq

