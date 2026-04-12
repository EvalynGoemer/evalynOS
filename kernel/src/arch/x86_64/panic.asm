extern panic_interrupt
global panic

panic:
    cli
    push 0
    push 0
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

    mov rsi, rsp         ; irq_saved_regs_t* ; also same as lea rdi, [rsp + 0]
    lea rdx, [rsp + 128] ; irq_cpu_frame_t
    mov rcx, [rsp + 120] ; uint64_t vector
    call panic_interrupt

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

    halt:
    hlt
    jmp halt
