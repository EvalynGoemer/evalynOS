.intel_syntax noprefix
.altmacro

.extern dispatch_interrupt
.type dispatch_interrupt_asm, @function

.macro ISR num
isr\num:
    push 0
    push \num
    jmp dispatch_interrupt_asm
.endm

.macro ISR_ERR num
isr\num:
    push \num
    jmp dispatch_interrupt_asm
.endm

.cfi_sections .debug_frame

dispatch_interrupt_asm:
    .cfi_startproc simple
    .cfi_signal_frame
    .cfi_def_cfa 7, 0

    test byte ptr [rsp + 24], 3
    jz .Lkernel_entry
    swapgs
    .Lkernel_entry:

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

    .cfi_def_cfa_offset 176
    .cfi_offset 0,  -64      # rax
    .cfi_offset 3,  -72      # rbx
    .cfi_offset 2,  -80      # rcx
    .cfi_offset 1,  -88      # rdx
    .cfi_offset 4,  -96      # rsi
    .cfi_offset 5,  -104     # rdi
    .cfi_offset 6,  -112     # rbp
    .cfi_offset 8,  -120     # r8
    .cfi_offset 9,  -128     # r9
    .cfi_offset 10, -136     # r10
    .cfi_offset 11, -144     # r11
    .cfi_offset 12, -152     # r12
    .cfi_offset 13, -160     # r13
    .cfi_offset 14, -168     # r14
    .cfi_offset 15, -176     # r15
    .cfi_offset 7,  -16      # cpu_frame.rsp
    .cfi_offset 16, -40      # cpu_frame.ip

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

    test byte ptr [rsp + 24], 3
    jz .Lkernel_exit
    swapgs
    .Lkernel_exit:

    add rsp, 16
    iretq
    .cfi_endproc

ISR     0  #DE   Division Error
ISR     1  #DB   Debug Exception
ISR     2  #NMI  Non Maskable Interrupt
ISR     3  #BP   Breakpoint Exception
ISR     4  #OF   Overflow Exception
ISR     5  #BR   Bound Range Exception
ISR     6  #UD   Invalid Opcode
ISR     7  #NM   FPU Not Found
ISR_ERR 8  #DF   Double Fault
ISR     9  #CSO  Coprocessor Segment Overrun
ISR_ERR 10 #TS   Invalid TSS
ISR_ERR 11 #NP   Missing Segment
ISR_ERR 12 #SS   Invalid Stack Segment
ISR_ERR 13 #GPF  General Protection Fault
ISR_ERR 14 #PF   Page Fault
ISR     15 #???  Reserved Exception
ISR     16 #MF   x87 FPU Exception
ISR_ERR 17 #AC   Alignment Check Exception
ISR     18 #MCE  Machine Check Exception
ISR     19 #XM   SIMD FPU Exception
ISR     20 #VE   Virtualization Exception
ISR_ERR 21 #CP   Control Protection Exception
ISR     22 #???  Reserved Exception
ISR     23 #???  Reserved Exception
ISR     24 #???  Reserved Exception
ISR     25 #???  Reserved Exception
ISR     26 #???  Reserved Exception
ISR     27 #???  Reserved Exception
ISR     28 #HV   Hypervisor Injection Exception
ISR_ERR 29 #HC   VMM Communication Exception
ISR_ERR 30 #DX   Security Exception
ISR     31 #???  Reserved Exception

.set i, 32
.rept 255 - 32 + 1
    ISR %i
    .set i, i + 1
.endr

.macro rel_isr_entry n
    .long isr\n - (rel_isr_table + \n * 4)
.endm

.section .rodata
.align 16
.global rel_isr_table
rel_isr_table:
i = 0
.rept 256
    rel_isr_entry %i
i = i + 1
.endr
