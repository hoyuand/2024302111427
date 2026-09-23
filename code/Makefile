# lab2 build: kernel plus flat user images embedded through userimg.S.
CC   = riscv64-unknown-elf-gcc
LD   = riscv64-unknown-elf-ld
OBJCOPY ?= riscv64-unknown-elf-objcopy
CFLAGS = -Wall -Werror -O -std=gnu99 -mcmodel=medany -ffreestanding \
         -nostdlib -fno-common -ggdb -march=rv64gc -fno-stack-protector -fno-pie
QEMU = qemu-system-riscv64

.NOTPARALLEL:

OBJS = kernel/entry.o kernel/start.o kernel/console.o kernel/printf.o \
       kernel/main.o kernel/proc.o kernel/syscall.o kernel/trap.o \
       kernel/trampoline.o kernel/trampoline_anchor.o kernel/userimg.o

UPROGS = sh hi spin badecall bufstorm
UFLAGS = -Wall -Werror -O -std=gnu99 -mcmodel=medany -ffreestanding \
         -nostdlib -fno-common -ggdb -march=rv64gc \
         -fno-stack-protector -fno-pie -mno-relax -I.

kernel/kernel: $(OBJS) kernel/kernel.ld kernel/lab2.ld
	$(LD) -T kernel/lab2.ld -o $@ $(OBJS)

%.o: %.c kernel/riscv.h kernel/course_sid.h
	$(CC) $(CFLAGS) -Ikernel -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -Ikernel -c -o $@ $<

user/usys.S: user/usys.pl
	perl user/usys.pl > user/usys.S

user-flat/%.bin: user/%.c user/user.h user/ulib.c user/printf.c user/usys.S
	mkdir -p user-flat
	$(CC) $(UFLAGS) -Iuser -c user/ulib.c   -o user-flat/ulib.o
	$(CC) $(UFLAGS) -Iuser -c user/printf.c -o user-flat/printf.o
	$(CC) $(UFLAGS) -Iuser -c user/usys.S   -o user-flat/usys.o
	$(CC) $(UFLAGS) -Iuser -c $<            -o user-flat/$*.o
	$(LD) -T user/user.ld -o user-flat/$*.elf \
	    user-flat/$*.o user-flat/ulib.o user-flat/printf.o user-flat/usys.o
	$(OBJCOPY) -O binary user-flat/$*.elf $@

kernel/userimg.S: Makefile $(addprefix user-flat/,$(UPROGS:=.bin))
	printf '.section .rodata\n .global _uprog_table\n_uprog_table:\n' > $@.tmp
	for p in $(UPROGS); do \
	  printf ' .quad _uprog_%s_start\n .quad _uprog_%s_end\n .asciz "%s"\n .balign 8\n' \
	    $$p $$p $$p >> $@.tmp; \
	  printf ' .global _uprog_%s_start\n_uprog_%s_start:\n .incbin "user-flat/%s.bin"\n .global _uprog_%s_end\n_uprog_%s_end:\n .balign 8\n' \
	    $$p $$p $$p $$p $$p >> $@.tmp; \
	done
	printf ' .quad 0\n .quad 0\n' >> $@.tmp
	mv $@.tmp $@

# 验收环境固定(排雷环节禁止改动本行以下内容)
qemu: kernel/kernel
	$(QEMU) -machine virt -bios none -kernel kernel/kernel -nographic -smp 1

clean:
	rm -f kernel/*.o kernel/kernel user/usys.S
	rm -rf user-flat
