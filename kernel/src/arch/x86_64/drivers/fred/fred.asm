global fred_ring3_entry_asm
global fred_ring0_entry_asm

extern dispatch_interrupt
extern dispatch_syscall

section .text
align 4096

fred_ring3_entry_asm:
    sti

    ; push dummy vector for frame
    ; 3 = CPL3
    push 3

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

    cmp byte [rsp + 174], 7 ; check if the FRED event was a syscall
    jne .other              ; if not goto the generic branch
    mov  rcx, rax           ; arg3 is passed by rax instead of rcx due to syscall
    mov  r11, rsp           ; save rsp so it can be passed as arg8 per sysv
    push r11                ; push arg8 (rsp/saved registers frame)
    push r10                ; push arg7
    call dispatch_syscall
    add rsp, 16
    mov  [rsp + 112], rax   ; put the retvals back on the stack to be restored
    mov  [rsp + 88], rdx
    jmp .common

.other:
    mov rdi, rsp
    call dispatch_interrupt

.common:
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

    add rsp, 8 ; pop the dummy vector

    eretu

times 256 - ($ - fred_ring3_entry_asm) db 0

fred_ring0_entry_asm:
    ; push dummy vector for frame
    ; 0 = CPL0
    push 0

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

    mov rdi, rsp
    sti
    call dispatch_interrupt
    cli

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
    add rsp, 8 ; pop the dummy vector

    erets

global fred_switch_to_user
fred_switch_to_user:
    push qword 0x20 | 3
    push qword rsi
    push qword 0x202
    push qword 0x28 | 3
    push qword rdi
    push qword 0

    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    xor esi, esi
    xor edi, edi
    xor ebp, ebp
    xor r8d,  r8d
    xor r9d,  r9d
    xor r10d, r10d
    xor r11d, r11d
    xor r12d, r12d
    xor r13d, r13d
    xor r14d, r14d
    xor r15d, r15d

    eretu
