# Lab1 设计笔记

## 目标与个人参数

本轮目标是让 QEMU `virt` 在 `-bios none` 下从 M 态完成最小启动配置，降权进入 S 态，初始化 16550 UART 轮询输出，并由无 libc 的 `printf` 输出个性化 banner 与边界自检。学号为 `2024302111427`，因此 `LAB1_BANNER_PROTOCOL=2`、`COURSE_SID % 97 = 0x17`，启动栈容量为 `12 KiB`。

## 上电到 main 的时序

1. QEMU 将内核加载到 RAM 起点 `0x80000000`，入口 `_entry` 从这里执行。`-bios none` 表示不运行固件，由 QEMU 直接把 PC 置为内核入口；链接脚本必须与这个物理加载地址一致，链接到 `0x0` 会使取指地址与实际装载地址不一致。
2. `_entry` 清除 `mie`，读取 `mhartid` 到 `tp`，非零 hart 进入 `wfi`/跳转自旋，避免多个 hart 同时初始化 UART、栈和全局状态。启动 hart 将 `sp` 设为 `bootstack + LAB1_STACK_KB*1024`，栈位于 `.bss` 的高端并按页对齐。
3. `_entry` 将 `mtvec` 写为 4 字节对齐的 `m_trap_vector`；配置 `pmpaddr0=0x3fffffffffffff` 与 `pmpcfg0=0xf`，用 NAPOT 放行全物理地址的 R/W/X 访问，保证降权后的 S 态能取指和访问 UART；随后把异常/中断委托寄存器写为全 1。
4. 启动阶段没有页表，`satp=0` 选择 Bare 模式，并执行 `sfence.vma` 清理可能残留的地址转换状态。设置 `mepc=start`、清除 `mstatus.MIE`、把 `mstatus.MPP` 设为 S，执行 `mret` 后从 `start` 的第一条指令继续。
5. `start` 不再读取 `mhartid`（该 CSR 属于 M 态）；它直接保留 `_entry` 已放入 `tp` 的 hart 编号，关闭 S 态中断，设置 `stvec`，初始化 UART，然后调用 `main`。`main` 返回后用 `wfi` 停机。

## UART 与输出协议

UART 基址是 `0x10000000`。初始化时通过 IER(偏移 1)、LCR(偏移 3)、FIFO 控制寄存器 FCR(偏移 2) 配置 8N1、关闭中断并清 FIFO。发送前反复读取 LSR(偏移 5) 的 THRE bit5，只有该位为 1 才向 THR(偏移 0) 写入一个字节，并用 `fence iorw, iorw` 保证 MMIO 顺序。

协议 2 采用普通 ASCII 正文，banner 和边界自检输出均参与字节和校验；`console_checksum_end` 关闭累加后，最后输出 `[chk=数值]` 并以换行结束，之后不再输出正文。校验和为此前正文（含换行）各字节 ASCII 值之和对 10000 取模。输出满 `16 + COURSE_SID % 16 = 19` 字节后执行 `1 + COURSE_SID % 8 = 4` 次 `nop`，节流只影响等待，不改变字节流。

## printf 边界与错误路径

`printf` 支持 `%c/%s/%d/%u/%x/%p/%%` 及可选 `l` 长度修饰；`%x` 使用小写、无前导零并自动添加 `0x` 前缀，负数先输出 `-` 再输出绝对值，特殊处理 `INT64_MIN` 避免取反溢出；空指针字符串显示 `(null)`。未知格式按原样输出，便于定位测试拼写错误。

本轮自测用例：

* 用例 A：`printf-boundary` 行覆盖 0、负数、最大/最小 32 位整数、空字符串、`%x` 前缀和 `%%`，检查结果为 `zero=0 neg=-42 max=2147483647 min=-2147483648 empty="" hex=0x2a percent=%`。
* 用例 B：连续四段数字与小写字母长字符串后接换行，验证循环输出、19 字节节流边界和 checksum 的稳定性；QEMU 复位后重复启动，输出必须逐字节一致。
