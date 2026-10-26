OUTPUT_ARCH(riscv)
ENTRY(main)
SECTIONS
{
  . = 0x0;
  /* The kernel starts each flat image at VA 0; Lab4's policychange marks
     main as .text.aaa so helper functions cannot become the raw entry. */
  .text : { *(.text.aaa) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  __lab3_code_end = .;
  . = ALIGN(0x1000);
  __lab3_data_start = .;
  .data : { *(.data .data.*) }
  .bss : { *(.bss .bss.*) *(.sbss*) }
  end = .;
}
