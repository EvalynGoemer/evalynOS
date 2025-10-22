ENTRY(main)

SECTIONS
{
  . = 0x4000;

  .text :
  {
    KEEP(*(.entry))
    *(.text)
  }

  .rodata :
  {
    *(.rodata)
  }

  .data :
  {
    *(.data)
  }
}
