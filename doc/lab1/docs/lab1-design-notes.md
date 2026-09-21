# Lab1 设计笔记

## 目标与个人参数

本轮目标是让 QEMU `virt` 在 `-bios none` 下从 M 态完成最小启动配置，降权进入 S 态，初始化 16550 UART 轮询输出，并由无 libc 的 `printf` 输出个性化 banner 与边界自检。学号为 `2024302111427`，因此 `LAB1_BANNER_PROTOCOL=2`、`COURSE_SID % 97 = 0x17`，启动栈容量为 `12 KiB`。

## 上电到 main 的时序

1. QEMU 将内核加载到 RAM 起点 `0x80000000`，入口 `_entry` 从这里执行。`-bios none` 表示不运行固件，由 QEMU 直接把 PC 置为内核入口；链接脚本必须与这个物理加载地址一致，链接到 `0x0` 会使取指地址与实际装载地址不一致。
2. `start.c` 以页对齐方式定义 `LAB1_STACK_KB*1024` 字节的 `bootstack`，同时满足说明书要求的 16 字节对齐。`_entry` 清除 `mie`，读取 `mhartid` 到 `tp`，非零 hart 进入 `wfi`/跳转自旋；启动 hart 将 `sp` 设为 `bootstack` 高地址端，随后调用仍处于 M 态的 `start()`。
3. `start()` 将 `mtvec` 写为 4 字节对齐的 `m_trap_vector`；配置 `pmpaddr0=0x3fffffffffffff` 与 `pmpcfg0=0xf`，用 NAPOT 放行全物理地址的 R/W/X 访问，保证降权后的 S 态能取指和访问 UART；随后配置异常与中断委托。
4. 启动阶段没有页表，`start()` 以 `satp=0` 选择 Bare 模式，并执行 `sfence.vma`。它设置 `mepc=main`、把 `mstatus.MPP` 设为 S，执行 `mret` 后从 `main` 的第一条指令继续。
5. `main()` 已运行在 S 态，不再读取 M 态 CSR；它关闭 S 态设备中断并调用 `consoleinit()` 初始化 UART，然后经 `consputc()` 输出 Banner 和边界自检。`main` 返回后没有合法调用者，因此最终实现保持输出完成后的等待状态。

## UART 与输出协议

UART 基址是 `0x10000000`。`consoleinit()` 调用 `uartinit()`，通过 IER(偏移 1)、LCR(偏移 3)、FIFO 控制寄存器 FCR(偏移 2) 配置 8N1、关闭中断并清 FIFO。`consputc()` 是 printf 和字符串输出的统一入口，内部调用 `uartputc_sync()`；后者发送前反复读取 LSR(偏移 5) 的 THRE bit5，只有该位为 1 才向 THR(偏移 0) 写入一个字节，并用 `fence iorw, iorw` 保证 MMIO 顺序。

协议 2 采用普通 ASCII 正文，banner 和边界自检输出均参与字节和校验；`console_checksum_end` 关闭累加后，最后输出 `[chk=数值]` 并以换行结束，之后不再输出正文。校验和为此前正文（含换行）各字节 ASCII 值之和对 10000 取模。输出满 `16 + COURSE_SID % 16 = 19` 字节后执行 `1 + COURSE_SID % 8 = 4` 次 `nop`，节流只影响等待，不改变字节流。

## printf 边界与错误路径

`printf` 支持 `%c/%s/%d/%u/%x/%p/%%` 及可选 `l` 长度修饰；`%x` 使用小写、无前导零并自动添加 `0x` 前缀，负数先输出 `-` 再输出绝对值，特殊处理 `INT64_MIN` 避免取反溢出；空指针字符串显示 `(null)`。未知格式按原样输出，便于定位测试拼写错误。

本轮自测用例：

* 用例 A：`printf-boundary` 行覆盖 0、负数、最大/最小 32 位整数、空字符串、`%x` 前缀和 `%%`，检查结果为 `zero=0 neg=-42 max=2147483647 min=-2147483648 empty="" hex=0x2a percent=%`。
* 用例 B：连续四段数字与小写字母长字符串后接换行，验证循环输出、19 字节节流边界和 checksum 的稳定性；QEMU 复位后重复启动，输出必须逐字节一致。
