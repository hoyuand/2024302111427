# 代码与运行工具

本目录保存课程实验的可执行实现和验证工具：

* `kernel/`：Lab1 内核启动、UART、`printf` 和主程序源码；
* `tests/`：Lab0/Lab1 资产检查、期望输出检查和 QEMU 回归脚本；
* `tools/`：环境预检、终端截图和辅助观测工具；
* `Makefile`：课程内核的构建入口；
* `check_expect.py`、`expect_banner.txt`：个性化输出格式检查器和期望输出。

在仓库根目录运行 `make -C code clean && make -C code` 可以重新构建内核。测试脚本从 `code/` 目录解析内核和期望输出路径。
