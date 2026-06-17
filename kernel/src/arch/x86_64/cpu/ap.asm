extern x86_ap_entry
global x86_ap_trampoline
global x86_ap_trampoline_dataptr
global x86_ap_trampoline_config
global x86_ap_trampoline_cr3
global x86_ap_trampoline_gdtr_base
global x86_ap_trampoline_farjmp
global x86_ap_trampoline_farjmp64
global x86_ap_trampoline_gdt
global x86_ap_trampoline32
global x86_ap_trampoline64

; if the config bit is set, set the result bit
; esi = config source
; eax = result
; arg1 = config bit
; arg2 = result bit
%macro CSET_CONF_BIT 2
    bt esi, %1
    jnc %%skip
    bts eax, %2
    %%skip:
%endmacro

section .data
align 4096
bits 16
x86_ap_trampoline:
    ; eax = scratch
    ; ebx = base address (calculated in real mode; used for protected mode)

    cli

    ; put the load address into EBX
    mov ebx, cs
    shl ebx, 4

    ; load gdt & idt
    o32 lgdt cs:[x86_ap_trampoline_gdtr - x86_ap_trampoline]
    o32 lidt cs:[x86_ap_trampoline_bad_idtr - x86_ap_trampoline]

    ; enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; far jump to protected mode
    o32 jmp far [cs:(x86_ap_trampoline_farjmp - x86_ap_trampoline)]

bits 32
x86_ap_trampoline32:
    ; eax = scratch (CRx / MSR)
    ; edx = scratch (forced due to rdmsr/wrmsr)
    ; ecx = msr selector
    ; ebx = base address
    ; esi = config bitfield

    ; setup segment regsters
    mov ax, 0x18
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; setup CR3
    mov eax, [x86_ap_trampoline_cr3 - x86_ap_trampoline + ebx]
    mov cr3, eax

    ; get config bitfield
    mov esi, [x86_ap_trampoline_config - x86_ap_trampoline + ebx]

    ; load cr4
    mov eax, cr4
    ; enable PAE
    or al, (1 << 5)
    ; enable LA57 if needed
    CSET_CONF_BIT 1, 12
    ; write cr4
    mov cr4, eax

    ; load EFER
    mov ecx, 0xC0000080
    rdmsr
    ; enable LME
    or ax, (1 << 8)
    ; enable NX if needed
    CSET_CONF_BIT 0, 11
    ; write EFER
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; far jump to long mode
    jmp far [x86_ap_trampoline_farjmp64 - x86_ap_trampoline + ebx]

bits 64
x86_ap_trampoline64:
    ; rax = global ap data ptr
    ; rbx = per ap data ptr
    ; rdi = internal core ID

    ; load the global ap data ptr
    mov rax, [rel x86_ap_trampoline_dataptr]

    ; get the internal core id atomicly
    mov edi, 1
    lock xadd dword [rax], edi

    ; retrieve the per ap data ptr
    mov rbx, [rax + (rdi * 8) + 64]

    ; get ap stack pointer
    mov rsp, [rbx + 0]

    ; jump to C
    cld
    mov rax, x86_ap_entry
    jmp rax

; constants
x86_ap_trampoline_bad_idtr:
    dw 0x0000
    dd 0x00000000
x86_ap_trampoline_gdt:
    dq 0x0000000000000000  ; null    ; 0x00
    dq 0x00cf9a000000ffff  ; 32 code ; 0x08
    dq 0x00af9b000000ffff  ; 64 code ; 0x10
    dq 0x00cf93000000ffff  ; data    ; 0x18

; patched at runtime by BSP
x86_ap_trampoline_gdtr:
x86_ap_trampoline_gdtr_limit:
    dw (x86_ap_trampoline_gdtr - x86_ap_trampoline_gdt - 1)
x86_ap_trampoline_gdtr_base:
    dd 0
x86_ap_trampoline_farjmp:
    dd 0
    dw 0x08
x86_ap_trampoline_farjmp64:
    dd 0
    dw 0x10

x86_ap_trampoline_dataptr:
    dq 0
x86_ap_trampoline_config:
    dd 0
x86_ap_trampoline_cr3:
    dd 0
