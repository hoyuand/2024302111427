# Lab1 结果摘要

目标：完成启动汇编、M 态到 S 态切换、PMP 授权、UART 轮询输出、最小 `printf` 和个性化 banner。

环境：WSL2；QEMU `virt`，`-nographic -smp 1`；学号 `2024302111427`，协议版本 `2`。

结果：`make clean && make` 通过；自检与回归测试通过；两次冷启动输出一致；串口输出 259 字节并与 `expect_banner.txt` 逐字节匹配；QEMU 中断日志为空。

证据：真实终端截图为 `images/lab1-terminal-run.png`，原始记录为 `docs/lab1-qemu-real-run.txt` 和 `docs/lab1-qemu-output.txt`；Git 提交/标签核验见 `docs/lab1-git-proof.txt`。
