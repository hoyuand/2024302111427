# 代码与运行工具

本目录保存课程实验的可执行实现和验证工具：

* `kernel/`：启动、trap、系统调用、控制台、每进程锁和上下文调度机制；
* `user/`：课程赠送用户库、Shell、示例程序和 Lab2–Lab5 官方测试；
* `support/`：串口注入测试驱动；
* `tests/`：Lab0–Lab5 资产检查、官方用户测试和 QEMU 回归脚本；
* `tools/`：环境预检、终端截图和辅助观测工具；
* `Makefile`：课程内核的构建入口；
* `Makefile.upgrade`：课程随 Lab2 增量包提供的原样构建规则参考，已合并到 `Makefile`；
* `check_expect.py`、`expect_banner.txt`：个性化输出格式检查器和期望输出。

在仓库根目录运行 `make -C code clean && make -C code` 可以重新构建内核与内嵌用户镜像；`make -C code qemu` 使用 QEMU `virt` 单核启动。测试脚本从 `code/` 目录解析内核和期望输出路径。

Lab4 官方程序是 `bench`、`stride_test`，自编 `alt`、`lab4check`、`policychange` 分别观察带pause的交错输出、接口边界和改票后的份额变化。主报告见 [`doc/lab4/docs/lab4-experiment-report.md`](../doc/lab4/docs/lab4-experiment-report.md)，自动实测入口为 `python3 code/tests/verify_lab4_qemu.py`。

Lab5增加 kernel/kalloc.c（引用计数分配器）、kernel/lab5.h（扩展声明）、user/sh_input.c（预置Shell换行兼容）。官方zerocopy/cowpair/echo保持增量包字节；自编cowtest/cowoom/vmtest验证边界及真实OOM。实测入口 `python3 code/tests/verify_lab5_qemu.py`，静态检查 `python3 -m unittest discover -s code/tests -p "test_*.py" -v`，主报告 [Lab5实验报告](../doc/lab5/docs/lab5-experiment-report.md)。

Lab6新增bio/log/fs/file/sysfile/sleeplock/fs_adapter及磁盘ELF加载；mkfs/生成fs.img，user/_*为精简ELF，均为忽略的构建产物。tests/verify_lab6_qemu.py以临时镜像进行验收。
