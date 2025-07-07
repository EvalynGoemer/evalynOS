ENTRY(kernel_main)

SECTIONS
{
  . = 0x1000;

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
