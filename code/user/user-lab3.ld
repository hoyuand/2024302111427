OUTPUT_ARCH(riscv)
ENTRY(main)
SECTIONS
{
  . = 0x0;
  .text : { *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  __lab3_code_end = .;
  . = ALIGN(0x1000);
  __lab3_data_start = .;
  .data : { *(.data .data.*) }
  .bss : { *(.bss .bss.*) *(.sbss*) }
  end = .;
}
