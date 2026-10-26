# Lab3：物理页分配与 SV39 虚拟内存

本轮完成 Lab3 V3.1：位图物理页分配器、SV39 三级页表、用户地址空间装配、受控拷贝、EAGER `sbrk`、`fork` 深拷贝、`exec` 原子重建，以及 Lab2 回归路径。验收固定使用 QEMU `virt`、单核 `-smp 1`。

## 目录

```text
doc/lab3/
├── README.md
├── docs/
│   └── lab3-experiment-report.md
└── images/
    ├── lab3-address-space.mmd / .png
    ├── lab3-syscall-copy.mmd / .png
    ├── lab3-rollback.mmd / .png
    └── lab3-terminal-run.png
```

主报告见 [`docs/lab3-experiment-report.md`](docs/lab3-experiment-report.md)。实现位于 [`code/kernel/vm.c`](../../code/kernel/vm.c)、[`code/kernel/proc.c`](../../code/kernel/proc.c) 和 [`code/kernel/syscall.c`](../../code/kernel/syscall.c)；官方测试为 [`badaccess.c`](../../code/user/badaccess.c) 与 [`memcap.c`](../../code/user/memcap.c)。

图片索引：`lab3-address-space.png`、`lab3-address-space.mmd`、`lab3-syscall-copy.png`、`lab3-syscall-copy.mmd`、`lab3-rollback.png`、`lab3-rollback.mmd`、`lab3-terminal-run.png`。可复现实测交互脚本为 [`code/support/lab3-smoke.script`](../../code/support/lab3-smoke.script)。

## 查验命令

```bash
make -C code clean && make -C code -j2
qemu-system-riscv64 -machine virt -bios none -kernel code/kernel/kernel -nographic -smp 1
python3 code/support/inject_uart.py --tree code --script code/support/lab3-smoke.script
```

启动后可在 `sh>` 输入 `hi`、`badaccess`、`memcap`。`badaccess` 应报告两个非法访问被杀，`memcap` 应报告 OOM 返回错误且已有分配保持可用。分页观测输出必须以 `DUMP-PAGETABLE begin/end` 包围，权限字段顺序为 `R/W/X/U`。

## 归档

本轮提交前检查 `git diff`、构建、QEMU、官方测试、Lab2 回归和工作区状态；历史 `lab3`、`lab3-submit` 标签保持不变。归档 ZIP 和代码增量包均为本地产物，不提交进源码仓库。
