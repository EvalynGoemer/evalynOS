.intel_syntax noprefix
.altmacro

.global fred_ring3_entry_asm
.global fred_ring0_entry_asm

.type fred_ring3_entry_asm, @function
.type fred_ring0_entry_asm, @function

.extern dispatch_interrupt
.extern dispatch_syscall

.cfi_sections .debug_frame

.balign 4096
fred_ring3_entry_asm:
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_def_cfa 7, 0

    sti

    # push dummy vector for frame
    # 3 = CPL3
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

    .cfi_def_cfa_offset 192
    .cfi_offset 0,  -80      # rax
    .cfi_offset 3,  -88      # rbx
    .cfi_offset 2,  -96      # rcx
    .cfi_offset 1,  -104     # rdx
    .cfi_offset 4,  -112     # rsi
    .cfi_offset 5,  -120     # rdi
    .cfi_offset 6,  -128     # rbp
    .cfi_offset 8,  -136     # r8
    .cfi_offset 9,  -144     # r9
    .cfi_offset 10, -152     # r10
    .cfi_offset 11, -160     # r11
    .cfi_offset 12, -168     # r12
    .cfi_offset 13, -176     # r13
    .cfi_offset 14, -184     # r14
    .cfi_offset 15, -192     # r15
    .cfi_offset 7,  -32      # cpu_frame.rsp
    .cfi_offset 16, -56      # cpu_frame.ip

    cmp byte ptr [rsp + 174], 7 # check if the FRED event was a syscall
    jne .other                  # if not goto the generic branch
    mov  rcx, rax               # arg3 is passed by rax instead of rcx due to syscall
    mov  r11, rsp               # save rsp so it can be passed as arg8 per sysv
    push r11                    # push arg8 (rsp/saved registers frame)
    push r10                    # push arg7
    call dispatch_syscall
    add rsp, 16
    mov  [rsp + 112], rax       # put the retvals back on the stack to be restored
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

    add rsp, 8 # pop the dummy vector

    eretu
    .cfi_endproc

.fill 256 - (. - fred_ring3_entry_asm), 1, 0

fred_ring0_entry_asm:
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_def_cfa 7, 0

    sti

    # push dummy vector for frame
    # 0 = CPL0
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

    .cfi_def_cfa_offset 192
    .cfi_offset 0,  -80      # rax
    .cfi_offset 3,  -88      # rbx
    .cfi_offset 2,  -96      # rcx
    .cfi_offset 1,  -104     # rdx
    .cfi_offset 4,  -112     # rsi
    .cfi_offset 5,  -120     # rdi
    .cfi_offset 6,  -128     # rbp
    .cfi_offset 8,  -136     # r8
    .cfi_offset 9,  -144     # r9
    .cfi_offset 10, -152     # r10
    .cfi_offset 11, -160     # r11
    .cfi_offset 12, -168     # r12
    .cfi_offset 13, -176     # r13
    .cfi_offset 14, -184     # r14
    .cfi_offset 15, -192     # r15
    .cfi_offset 7,  -32      # cpu_frame.rsp
    .cfi_offset 16, -56      # cpu_frame.ip

    mov rdi, rsp
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
    add rsp, 8 # pop the dummy vector

    erets
    .cfi_endproc

.global fred_switch_to_user
.type fred_switch_to_user, @function
fred_switch_to_user:
    .cfi_startproc simple
    .cfi_def_cfa 7, 8
    .cfi_offset 16, -8

    push 0x20 | 3
    push rsi
    push 0x202
    push 0x28 | 3
    push rdi
    push 0

    .cfi_def_cfa_offset 56

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
    .cfi_endproc
