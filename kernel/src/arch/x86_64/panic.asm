extern panic_interrupt
global panic

panic:
    cli

    ; fake interrupt frame
    push 0
    push 0
    push 0
    push rsp
    pushfq
    push 0
    push 0
    push 0
    push 999

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

    mov rsi, rsp
    call panic_interrupt

    halt:
    hlt
    jmp halt
