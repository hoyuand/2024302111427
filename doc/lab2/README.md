# Lab2：陷入、中断与系统调用

教师查验入口只有一条主线：**本页导航 → 唯一实验报告 → 图示与实测证据**。本轮以 QEMU `virt`、单核 `-smp 1` 验收；学号为 `2024302111427`。

## 验收材料层级

```text
doc/lab2/
├── README.md                         # 本页：导航与逐文件索引
├── docs/
│   └── lab2-experiment-report.md     # 唯一主报告
└── images/
    ├── lab2-trap-flow.png            # write syscall 往返图（展示）
    ├── lab2-trap-flow.mmd            # 同图可编辑源文件
    ├── lab2-console-sequence.png     # UART 输入中断时序图（展示）
    ├── lab2-console-sequence.mmd     # 同图可编辑源文件
    └── lab2-terminal-run.png         # 真实 QEMU 终端截图
```

### 主报告

- [`docs/lab2-experiment-report.md`](docs/lab2-experiment-report.md)：目标与结果、数据结构/不变式/回滚、三道 V2 思考题、两项边界/错误路径自测设计及实测、代码位置、要求符合情况、查验命令、真实运行输出摘要、PNG 图示、已知限制和 Git 归档记录。文档中的 WFI 等待语义与完整 sleep/wakeup 调度器明确区分。

### 图片与源文件

- [`images/lab2-trap-flow.png`](images/lab2-trap-flow.png)：`write(fd, buf, n)` 从 U 态 `ecall` 经 trapframe、内核栈、`sys_write`、UART THR 写入，再通过 `trapframe.a0` 和 `sret` 返回的完整路径。
- [`images/lab2-trap-flow.mmd`](images/lab2-trap-flow.mmd)：上一张流程图的可编辑 Mermaid 源文件。
- [`images/lab2-console-sequence.png`](images/lab2-console-sequence.png)：空缓冲 `read`、S 态 WFI、UART/PLIC 外部中断、内核陷阱处理、环形缓冲入队与用户态返回的时序图；不把 WFI 误画为进程 sleep/wakeup。
- [`images/lab2-console-sequence.mmd`](images/lab2-console-sequence.mmd)：上一张时序图的可编辑 Mermaid 源文件。
- [`images/lab2-terminal-run.png`](images/lab2-terminal-run.png)：真实 QEMU `virt` 单核运行截图，记录启动 banner、`hi`、`badecall` 和短行 `bufstorm`。

## 实现位置

内核实现位于 [`code/kernel/`](../../code/kernel/)；用户程序位于 [`code/user/`](../../code/user/)；QEMU 自动串口注入脚本位于 [`code/support/`](../../code/support/)；课程增量包原样构建参考为 [`code/Makefile.upgrade`](../../code/Makefile.upgrade)，已合并规则在 [`code/Makefile`](../../code/Makefile)；个性化参数定义在 [`code/kernel/course_sid.h`](../../code/kernel/course_sid.h)。V2 要求与代码映射见 [`course-config/lab2-requirements.md`](../../course-config/lab2-requirements.md)。课程赠送 `trampoline.S`、用户接口和 `user.ld` 保持原样；当前用户态使用最小 SV39 页表，不是 `satp=0`。

## 查验命令

在仓库根目录运行：

```bash
make -C code clean && make -C code -j2
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 code/tests/verify_lab1_qemu.py
python3 code/support/inject_uart.py --tree code --script code/support/lab2-smoke.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-bufstorm.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-overflow.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-spin.script
```

报告给出对应结果摘要：构建、banner 比对、Lab1 双冷启动回归及四个 Lab2 驱动检查；同时保留未知 syscall、短输入/超长输入及 spin 中注入串口的边界结果。

## Git 归档

Lab2 归档标签为 `lab2-start`、`lab2`、`lab2-submit`；验收完成时 `main`、`lab2` 和 `lab2-submit` 指向同一最终提交。完整归档 `提交-lab2-2024302111427.zip` 和相对 `lab1-submit` 的代码增量包 `code/archives/lab2-code-delta-2024302111427.zip` 均为本地提交产物，不提交进源码目录。GitHub 项目名为 [`2024302111427`](https://github.com/hoyuand/2024302111427)，克隆地址为 `https://github.com/hoyuand/2024302111427.git`。
