/* Lab2-only linker additions; keep the course kernel.ld unchanged. */
INCLUDE kernel/kernel.ld

SECTIONS
{
  . = ALIGN(0x1000);
  .trampsec :
  {
    KEEP(*(trampsec))
    KEEP(*(.trampanchor))
  }

  /* Keep Lab2's trampoline pages out of the allocator range used later. */
  . = ALIGN(0x1000);
  kernel_end = .;

  .userimage (NOLOAD) : ALIGN(0x1000)
  {
    *(.userimage)
  }
}
