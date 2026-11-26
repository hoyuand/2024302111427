# Lab6: kernel plus disk filesystem containing stripped user ELF images.
CC   = riscv64-unknown-elf-gcc
LD   = riscv64-unknown-elf-ld
OBJCOPY ?= riscv64-unknown-elf-objcopy
CFLAGS = $(EXTRA_CFLAGS) -Wall -Werror -O -std=gnu99 -mcmodel=medany -ffreestanding \
         -nostdlib -fno-common -ggdb -march=rv64gc -fno-stack-protector -fno-pie
QEMU = qemu-system-riscv64

.NOTPARALLEL:

OBJS = kernel/entry.o kernel/start.o kernel/console.o kernel/printf.o \
       kernel/main.o kernel/proc.o kernel/swtch.o kernel/syscall.o kernel/trap.o kernel/vm.o \
       kernel/trampoline.o kernel/trampoline_anchor.o kernel/bio.o kernel/log.o kernel/fs.o kernel/file.o kernel/sysfile.o kernel/sleeplock.o kernel/virtio_disk.o kernel/crash.o kernel/fs_adapter.o kernel/kalloc.o

UPROGS = sh hi spin badecall bufstorm badaccess memcap alt lab4check policychange bench lottery_test stride_test
UPROGS += zerocopy cowpair echo cowtest cowoom vmtest
UPROGS += fsstress cat crashtest fstest
UFLAGS = -Wall -Werror -O -std=gnu99 -mcmodel=medany -ffreestanding \
         -nostdlib -fno-common -ggdb -march=rv64gc \
         -fno-stack-protector -fno-pie -mno-relax -DSID=2024302111427 -I.

all: kernel/kernel fs.img

.DEFAULT_GOAL := all

kernel/kernel: $(OBJS) kernel/kernel.ld kernel/lab3.ld
	$(LD) -T kernel/lab3.ld -o $@ $(OBJS)

%.o: %.c kernel/riscv.h kernel/course_sid.h kernel/proc.h kernel/vm.h kernel/lab5.h kernel/defs.h kernel/fs.h kernel/buf.h kernel/spinlock.h
	$(CC) $(CFLAGS) -Ikernel -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -Ikernel -c -o $@ $<

user/usys.S: user/usys.pl
	perl user/usys.pl > user/usys.S

USER_LIB_OBJS = user-flat/ulib.o user-flat/printf.o user-flat/usys.o user-flat/common.o user-flat/sh_input.o

user-flat/ulib.o: user/ulib.c user/user.h
	mkdir -p user-flat
	$(CC) $(UFLAGS) -Iuser -c $< -o $@

user-flat/printf.o: user/printf.c user/user.h
	mkdir -p user-flat
	$(CC) $(UFLAGS) -Iuser -c $< -o $@

user-flat/usys.o: user/usys.S user/usys.pl
	mkdir -p user-flat
	$(CC) $(UFLAGS) -Iuser -c $< -o $@

user-flat/common.o: user/common.c user/common.h user/user.h
	mkdir -p user-flat
	$(CC) $(UFLAGS) -Iuser -c $< -o $@

user-flat/sh_input.o: user/sh_input.c user/user.h
	mkdir -p user-flat
	$(CC) $(UFLAGS) -Iuser -c $< -o $@

user-flat/%.bin: user/%.c user/user.h user/user-lab3.ld $(USER_LIB_OBJS)
	mkdir -p user-flat
	$(CC) $(UFLAGS) $(if $(filter sh,$*),-Dgets=lab5_gets) -Iuser -c $< -o user-flat/$*.o
	$(LD) -T user/user-lab3.ld -o user-flat/$*.elf \
	    user-flat/$*.o $(USER_LIB_OBJS)
	$(OBJCOPY) -O binary user-flat/$*.elf $@

HOSTCC ?= cc

QEMUOPTS_FS = -global virtio-mmio.force-legacy=false \
              -drive file=fs.img,if=none,format=raw,id=x0 \
              -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMU += $(QEMUOPTS_FS)

# 保留中间产物 user-flat/%.bin，防止先执行 make fs.img 时被 GNU Make 自动清理
.PRECIOUS: user-flat/%.bin

# 从 user-flat/%.elf 剥离调试段生成精简 ELF 可执行文件 user/_%（匹配 mkfs 的 "user/_name" 规则）
user/_%: user-flat/%.bin
	$(OBJCOPY) -S user-flat/$*.elf $@

mkfs/mkfs: mkfs/mkfs.c kernel/fs.h kernel/param.h kernel/types.h kernel/stat.h kernel/course_sid.h
	mkdir -p mkfs
	$(HOSTCC) -Wall -Werror -I. -o mkfs/mkfs mkfs/mkfs.c

fs.img: mkfs/mkfs $(patsubst %,user/_%,$(UPROGS))
	./mkfs/mkfs fs.img $(patsubst %,user/_%,$(UPROGS))

fsimg: fs.img

qemu: fs.img

# 验收环境固定(排雷环节禁止改动本行以下内容)
qemu: kernel/kernel
	$(QEMU) -machine virt -bios none -kernel kernel/kernel -nographic -smp 1

clean:
	rm -f kernel/*.o kernel/kernel user/usys.S
	rm -rf user-flat
	rm -f user/_* mkfs/mkfs fs.img
