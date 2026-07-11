extern x86_ap_entry

global ap_trampoline
global ap_trampoline32
global ap_trampoline64
global ap_trampoline_gdt
global ap_trampoline_data

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
ap_trampoline:
    ; eax = scratch
    ; ebx = base address (calculated in real mode; used for protected mode)

    cli

    ; put the load address into EBX
    mov ebx, cs
    shl ebx, 4

    ; load gdt & idt
    o32 lgdt cs:[ap_trampoline_gdtr - ap_trampoline]
    o32 lidt cs:[ap_trampoline_bad_idtr - ap_trampoline]

    ; enable protected mode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; far jump to protected mode
    o32 jmp far [cs:(ap_trampoline_farjmp - ap_trampoline)]

bits 32
ap_trampoline32:
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
    mov eax, [ap_trampoline_cr3 - ap_trampoline + ebx]
    mov cr3, eax

    ; get config bitfield
    mov esi, [ap_trampoline_config - ap_trampoline + ebx]

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
    jmp far [ap_trampoline_farjmp64 - ap_trampoline + ebx]

bits 64
ap_trampoline64:
    ; rax = global ap data ptr
    ; rbx = per ap data ptr
    ; rdi = internal core ID

    ; load the global ap data ptr
    mov rax, [rel ap_trampoline_dataptr]

    ; get the internal core id atomicly
    mov edi, 1
    lock xadd dword [rax + 0], edi

    ; retrieve the per ap data ptr
    mov rbx, [rax + (rdi * 8) + 64]

    ; get ap stack pointer
    mov rsp, [rbx + 0]

    ; jump to C
    cld
    mov rax, x86_ap_entry
    jmp rax

; constants
ap_trampoline_bad_idtr:
    dw 0x0000
    dd 0x00000000
ap_trampoline_gdt:
    dq 0x0000000000000000  ; null    ; 0x00
    dq 0x00cf9a000000ffff  ; 32 code ; 0x08
    dq 0x00af9b000000ffff  ; 64 code ; 0x10
    dq 0x00cf93000000ffff  ; data    ; 0x18

; patched at runtime by BSP
ap_trampoline_data:
ap_trampoline_gdtr:
    dw (ap_trampoline_gdtr - ap_trampoline_gdt - 1)
    dd 0
ap_trampoline_farjmp:
    dd 0
    dw 0x08
ap_trampoline_farjmp64:
    dd 0
    dw 0x10

_pad:
    times 6 db 0

ap_trampoline_dataptr:
    dq 0
ap_trampoline_cr3:
    dd 0
ap_trampoline_config:
    dd 0
