# Lab3：物理页分配与 SV39 虚拟内存

本轮完成 Lab3 V3.1：位图物理页分配器、SV39 三级页表、用户地址空间装配、受控拷贝、EAGER `sbrk`、`fork` 深拷贝、`exec` 原子重建，以及 Lab2 回归路径。验收固定使用 QEMU `virt`、单核 `-smp 1`。

## 目录

```text
doc/lab3/
├── README.md
├── docs/lab3-experiment-report.md
└── images/
    ├── lab3-address-space.mmd / .png
    ├── lab3-syscall-copy.mmd / .png
    ├── lab3-rollback.mmd / .png
    └── lab3-terminal-run.png
```

主报告见 [`docs/lab3-experiment-report.md`](docs/lab3-experiment-report.md)。实现位于 [`code/kernel/vm.c`](../../code/kernel/vm.c)、[`code/kernel/proc.c`](../../code/kernel/proc.c) 和 [`code/kernel/syscall.c`](../../code/kernel/syscall.c)；官方测试为 [`badaccess.c`](../../code/user/badaccess.c) 与 [`memcap.c`](../../code/user/memcap.c)。课程要求原文与 ABI 留在 [`course-config/`](../../course-config/)。

## 查验命令

```bash
make -C code clean && make -C code -j2
qemu-system-riscv64 -machine virt -bios none -kernel code/kernel/kernel -nographic -smp 1
```

启动后可在 `sh>` 输入 `hi`、`badaccess`、`memcap`。`badaccess` 应报告两个非法访问被杀，`memcap` 应报告 OOM 返回错误且已有分配保持可用。分页观测输出必须以 `DUMP-PAGETABLE begin/end` 包围，权限字段顺序为 `R/W/X/U`。

## 归档

本轮提交前检查 `git diff`、构建、QEMU、官方测试、Lab2 回归和工作区状态；最终 `main`、`lab3`、`lab3-submit` 指向同一验收提交，并另生成本地 `提交-lab3-2024302111427.zip`。代码增量包为 `code/archives/lab3-code-delta-2024302111427.zip`，均不提交到源码仓库。
