global arch_thread_switch
global idt_switch_to_user
extern fixed_cpu_local
extern schedule_finalize

section .rodata

x87fpu dw 0x0C3F
ssefpu dd 0x1F80

section .text

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

; rdi = new ip
; rsi = new sp
idt_switch_to_user:
    cli
    fninit
    fldcw [rel x87fpu]
    ldmxcsr [rel ssefpu]

    mov r11, 0x202
    mov rcx, rdi
    mov rsp, rsi

    xor eax, eax
    xor ebx, ebx
    xor edx, edx
    xor esi, esi
    xor edi, edi
    xor ebp, ebp
    xor r8d,  r8d
    xor r9d,  r9d
    xor r10d, r10d
    xor r12d, r12d
    xor r13d, r13d
    xor r14d, r14d
    xor r15d, r15d

    swapgs
    o64 sysret
