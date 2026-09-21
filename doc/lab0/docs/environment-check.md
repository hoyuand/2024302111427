# Lab0 环境自检证据

检查时间：2026-09-07（Asia/Shanghai）  
环境：Windows 11 + WSL2 + Ubuntu 24.04.3 LTS  
内核：`6.6.87.2-microsoft-standard-WSL2`  
QEMU：`8.2.2`  
Git：`2.43.0`  
Python：`3.12.3`  
GNU Make：`4.3`

已按《学生环境安装指引》补装：

```text
gcc-riscv64-unknown-elf 13.2.0-11ubuntu1+12
binutils-riscv64-unknown-elf 2.42-1ubuntu1+6
```

课程原版 `preflight.py` 完整输出：

```text
== osLab 开课前环境自检 ==

[ ok ] riscv64-unknown-elf-gcc
[ ok ] riscv64-unknown-elf-ld
[ ok ] riscv64-unknown-elf-objdump
[ ok ] qemu-system-riscv64
[ ok ] python3
[warn] riscv gdb 不可用——三处导学演示改用《教师版-无gdb替代演示手册》三板斧(-d int / -d in_asm / monpeek.py)

[warn] S 态探针结果异常(无PMP=?,有PMP=?)——请人工复核;lab1 黑屏排查从 PMP 与 mtvec 对齐开始。

结论:环境自检通过(警告项按提示处理)
```

个性化 Lab1 初始骨架构建成功，最后一行是课程 README 明确说明的预期警告：

```text
riscv64-unknown-elf-ld -T code/kernel/kernel.ld -o code/kernel/kernel code/kernel/entry.o code/kernel/start.o code/kernel/console.o code/kernel/printf.o code/kernel/main.o
riscv64-unknown-elf-ld: warning: cannot find entry symbol _entry; defaulting to 0000000080000000
```

使用课程提供的完整 xv6 参考内核、`fs.img` 和固定单核参数 `-smp 1` 进行启动测试，实际输出为：

```text
xv6 kernel is booting

init: starting sh
$
```

出现 Shell 提示符证明 QEMU 能加载内核与磁盘，并已完成内核初始化、首个用户进程、init 和 sh 的运行。

`preflight.py` 的 `priv` 自动解析警告来自 QEMU Monitor 输出格式差异。使用课程附带 `monpeek.py` 对完整 xv6 内核独立读取寄存器成功，得到：

```text
CPU#0
pc       0000000080000882
stvec    0000000080005460
satp     8000000000087fff
scause   0000000000000000
```

非零 `satp` 且最高位模式字段为 8，证明内核已启用 Sv39 页表；`pc` 与 `stvec` 位于内核地址区。课程明确允许 RISC-V gdb 缺失，后续统一使用 QEMU 的 `-d int`、`-d in_asm`、Monitor 与内核观测输出调试。

复验命令：

```bash
python3 code/tools/preflight.py
```
