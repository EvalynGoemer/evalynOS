extern dispatch_interrupt

%macro ISR 1
global isr%1
isr%1:
    push 0
    push %1
    jmp dispatch_interupt_asm
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
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

    mov rdi, rsp         ; irq_saved_regs_t ; also same as lea rdi, [rsp + 0]
    lea rsi, [rsp + 128] ; irq_cpu_frame_t
    mov rdx, [rsp + 120] ; uint64_t vector

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

ISR     0x00 ; #DE   Division Error
ISR     0x01 ; #DB   Debug Exception
ISR     0x02 ; #NMI  Non Maskable Interrupt
ISR     0x03 ; #BP   Breakpoint Exception
ISR     0x04 ; #OF   Overflow Exception
ISR     0x05 ; #BR   Bound Range Exception
ISR     0x06 ; #UD   Invalid Opcode
ISR     0x07 ; #NM   FPU Not Found
ISR_ERR 0x08 ; #DF   Double Fault
ISR     0x09 ; #CSO  Coprocessor Segment Overrun
ISR_ERR 0x0A ; #TS   Invalid TSS
ISR_ERR 0x0B ; #NP   Missing Segemnt
ISR_ERR 0x0C ; #SS   Invalid Stack Segment
ISR_ERR 0x0D ; #GPF  General Protection Fault
ISR_ERR 0x0E ; #PF   Page Fault
ISR     0x0F ; #???  Reserved Exception
ISR     0x10 ; #MF   x87 FPU Exception
ISR_ERR 0x11 ; #AC   Alignment Check Exception
ISR     0x12 ; #MCE  Machine Check Exception
ISR     0x13 ; #XM   SIMD FPU Exception
ISR     0x14 ; #VE   Virtualization Exception
ISR_ERR 0x15 ; #CP   Control Protection Exception
ISR     0x16 ; #???  Reserved Exception
ISR     0x17 ; #???  Reserved Exception
ISR     0x18 ; #???  Reserved Exception
ISR     0x19 ; #???  Reserved Exception
ISR     0x1A ; #???  Reserved Exception
ISR     0x1B ; #???  Reserved Exception
ISR     0x1C ; #HV   Hypervisor Injection Exception
ISR_ERR 0x1D ; #HC   VMM Communication Exception
ISR_ERR 0x1E ; #DX   Security Exception
ISR     0x1F ; #???  Reserved Exception

%assign i 0x20
%rep    0xFF - 0x20 + 1
    ISR i
    %assign i i+1
%endrep

section .rodata
align 8
global isr_table
isr_table:
    dq isr0x00   ; #DE   Division Error
    dq isr0x01   ; #DB   Debug Exception
    dq isr0x02   ; #NMI  Non Maskable Interrupt
    dq isr0x03   ; #BP   Breakpoint Exception
    dq isr0x04   ; #OF   Overflow Exception
    dq isr0x05   ; #BR   Bound Range Exception
    dq isr0x06   ; #UD   Invalid Opcode
    dq isr0x07   ; #NM   FPU Not Found
    dq isr0x08   ; #DF   Double Fault
    dq isr0x09   ; #CSO  Coprocessor Segment Overrun
    dq isr0x0A   ; #TS   Invalid TSS
    dq isr0x0B   ; #NP   Missing Segemnt
    dq isr0x0C   ; #SS   Invalid Stack Segment
    dq isr0x0D   ; #GPF  General Protection Fault
    dq isr0x0E   ; #PF   Page Fault
    dq isr0x0F   ; #???  Reserved Exception
    dq isr0x10   ; #MF   x87 FPU Exception
    dq isr0x11   ; #AC   Alignment Check Exception
    dq isr0x12   ; #MCE  Machine Check Exception
    dq isr0x13   ; #XM   SIMD FPU Exception
    dq isr0x14   ; #VE   Virtualization Exception
    dq isr0x15   ; #CP   Control Protection Exception
    dq isr0x16   ; #???  Reserved Exception
    dq isr0x17   ; #???  Reserved Exception
    dq isr0x18   ; #???  Reserved Exception
    dq isr0x19   ; #???  Reserved Exception
    dq isr0x1A   ; #???  Reserved Exception
    dq isr0x1B   ; #???  Reserved Exception
    dq isr0x1C   ; #HV   Hypervisor Injection Exception
    dq isr0x1D   ; #HC   VMM Communication Exception
    dq isr0x1E   ; #DX   Security Exception
    dq isr0x1F   ; #???  Reserved Exception

    %assign i 0x20
    %rep    0xFF - 0x20 + 1
        dq isr %+ i
        %assign i i+1
    %endrep
