OUTPUT_FORMAT(elf64-loongarch)

ENTRY(kentry)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
    dynamic PT_DYNAMIC;
}

SECTIONS
{
    . = 0xffffffff80000000;

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .note.gnu.build-id : {
        __build_id_start = .;
        KEEP(*(.note.gnu.build-id))
        __build_id_end = .;
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
    } :data

    .dynamic : {
        *(.dynamic)
    } :data :dynamic

    .got : {
        *(.got)
        *(.got.plt)
        *(.igot.plt)
    }

    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .cpu_local : {
        __cpu_local_start = .;
        KEEP(*(.cpu_local_fixed))
        KEEP(*(.cpu_local))
        . = ALIGN(CONSTANT(MAXPAGESIZE));
        __cpu_local_end   = .;
    }
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
