extern dispatch_interrupt

%macro ISR 1
isr%1:
    push 0
    push %1
    jmp dispatch_interrupt_asm
%endmacro

%macro ISR_ERR 1
isr%1:
    push %1
    jmp dispatch_interrupt_asm
%endmacro

dispatch_interrupt_asm:
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

    cld
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

    add rsp, 16
    iretq

ISR     0  ; #DE   Division Error
ISR     1  ; #DB   Debug Exception
ISR     2  ; #NMI  Non Maskable Interrupt
ISR     3  ; #BP   Breakpoint Exception
ISR     4  ; #OF   Overflow Exception
ISR     5  ; #BR   Bound Range Exception
ISR     6  ; #UD   Invalid Opcode
ISR     7  ; #NM   FPU Not Found
ISR_ERR 8  ; #DF   Double Fault
ISR     9  ; #CSO  Coprocessor Segment Overrun
ISR_ERR 10 ; #TS   Invalid TSS
ISR_ERR 11 ; #NP   Missing Segment
ISR_ERR 12 ; #SS   Invalid Stack Segment
ISR_ERR 13 ; #GPF  General Protection Fault
ISR_ERR 14 ; #PF   Page Fault
ISR     15 ; #???  Reserved Exception
ISR     16 ; #MF   x87 FPU Exception
ISR_ERR 17 ; #AC   Alignment Check Exception
ISR     18 ; #MCE  Machine Check Exception
ISR     19 ; #XM   SIMD FPU Exception
ISR     20 ; #VE   Virtualization Exception
ISR_ERR 21 ; #CP   Control Protection Exception
ISR     22 ; #???  Reserved Exception
ISR     23 ; #???  Reserved Exception
ISR     24 ; #???  Reserved Exception
ISR     25 ; #???  Reserved Exception
ISR     26 ; #???  Reserved Exception
ISR     27 ; #???  Reserved Exception
ISR     28 ; #HV   Hypervisor Injection Exception
ISR_ERR 29 ; #HC   VMM Communication Exception
ISR_ERR 30 ; #DX   Security Exception
ISR     31 ; #???  Reserved Exception

%assign i 32
%rep    255 - 32 + 1
    ISR i
    %assign i i+1
%endrep

section .rodata
align 16
global rel_isr_table
rel_isr_table:
    %assign i 0
    %rep 255 + 1
        dd isr %+ i - (rel_isr_table + i * 4)
        %assign i i+1
    %endrep
