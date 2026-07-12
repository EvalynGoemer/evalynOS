global arch_thread_switch
extern fixed_cpu_local
extern schedule_finalize

; rdi = prev thread_t*
; rsi = next thread_t*
arch_thread_switch:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    ; swap stacks
    mov [rdi], rsp
    mov rsp, [rsi]

    ; finish the switch on new stack
    call schedule_finalize

    ; unlock the scheduler spinlock
    mov dword [rel gs:fixed_cpu_local + 48], 0

    ; enable interrupts
    sti

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret
