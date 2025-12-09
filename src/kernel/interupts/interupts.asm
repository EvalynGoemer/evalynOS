extern dispatch_interupt

%macro ISR 1
global isr%1
isr%1:
    cli
    push 0
    push %1
    jmp dispatch_interupt_asm
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push %1
    jmp dispatch_interupt_asm
%endmacro

dispatch_interupt_asm:
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
    call dispatch_interupt

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

    add rsp, 16
    iretq

; Exceptions
ISR_ERR 0x08           ; DF
ISR_ERR 0x0D           ; GPF
ISR_ERR 0x0E           ; PF

; Hardware
ISR     0x20           ; PIT
ISR     0x21           ; PS/2
ISR     0x24           ; Serial #1 & #3

; Spurious Interupts
ISR     0x27           ; Spurious PIC1
ISR     0x2F           ; Spurious PIC2

; System Call
ISR     0x69           ; Non POSIX System Calls

; Others
ISR     0x16           ; Reserved (Used as placeholder for generic)
