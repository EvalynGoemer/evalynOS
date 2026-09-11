global syscall_handler
extern dispatch_syscall
extern fixed_cpu_local

syscall_handler:
    swapgs

    mov [rel gs:fixed_cpu_local + 16], r15  ; save r15 into cpu local scratch
    mov r15, [rel gs:fixed_cpu_local + 8]   ; get the current thread
    mov [r15 + 32], rsp                     ; save the users stack pointer
    mov rsp, [r15 + 8]                      ; load the kernel stack pointer
    mov r15, [rel gs:fixed_cpu_local + 16]  ; restore r15

    sti

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

    mov  rcx, rax ; arg3 is passed by rax instead of rcx due to syscall
    mov  r11, rsp ; save rsp so it can be passed as arg8 per sysv
    push r11      ; push arg8 (rsp/saved registers frame)
    push r10      ; push arg7
    call dispatch_syscall
    add rsp, 16

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
    add rsp, 8 ; skip rdx since its a retval
    pop rcx
    pop rbx
    add rsp, 8 ; skip rax since its a retval

    cli

    mov [rel gs:fixed_cpu_local + 16], r15 ; save r15 into cpu local scratch
    mov r15, [rel gs:fixed_cpu_local + 8]  ; get the current thread
    mov rsp, [r15 + 32]                    ; restore the users stack pointer
    mov r15, [rel gs:fixed_cpu_local + 16] ; restore r15

    swapgs
    o64 sysret
