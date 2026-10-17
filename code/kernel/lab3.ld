INCLUDE kernel/kernel.ld

SECTIONS
{
  . = ALIGN(0x1000);
  .trampsec : { KEEP(*(trampsec)) }
  . = ALIGN(0x1000);
  kernel_end = .;
  .userimage : { *(.userimage) }
}
