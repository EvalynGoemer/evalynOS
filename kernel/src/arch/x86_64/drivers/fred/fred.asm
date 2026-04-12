global fred_ring3_entry_asm_stub
global fred_ring0_entry_asm_stub
global fred_ring3_entry_asm
global fred_ring0_entry_asm

extern fred_ring3_entry
extern fred_ring0_entry

section .text
align 4096

fred_ring3_entry_asm_stub:
    jmp fred_ring3_entry_asm

times 256 - ($ - fred_ring3_entry_asm_stub) db 0

fred_ring0_entry_asm_stub:
    jmp fred_ring0_entry_asm

fred_ring3_entry_asm:
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
    call fred_ring3_entry
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

    eretu

fred_ring0_entry_asm:
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
    call fred_ring0_entry
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

    erets

global fred_switch_to_user
fred_switch_to_user:
    push qword 0x20 | 3
    push qword rsi
    push qword 0x202
    push qword 0x28 | 3
    push qword rdi
    push qword 0

    eretu
