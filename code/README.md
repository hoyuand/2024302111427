# 代码与运行工具

本目录保存课程实验的可执行实现和验证工具：

* `kernel/`：启动、trap、系统调用、控制台和最小进程机制；
* `user/`：课程赠送用户库、Shell、示例程序和官方 Lab2 测试；
* `support/`：串口注入测试驱动；
* `tests/`：Lab0–Lab2 资产检查、官方用户测试和 QEMU 回归脚本；
* `tools/`：环境预检、终端截图和辅助观测工具；
* `Makefile`：课程内核的构建入口；
* `Makefile.upgrade`：课程随 Lab2 增量包提供的原样构建规则参考，已合并到 `Makefile`；
* `check_expect.py`、`expect_banner.txt`：个性化输出格式检查器和期望输出。

在仓库根目录运行 `make -C code clean && make -C code` 可以重新构建内核与内嵌用户镜像；`make -C code qemu` 使用 QEMU `virt` 单核启动。测试脚本从 `code/` 目录解析内核和期望输出路径。
