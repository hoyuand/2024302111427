# Lab6 文件系统实践

[唯一中文主报告](docs/lab6-experiment-report.md)；[要求对照](../../course-config/lab6-v3-requirements.md)；[先期设计](../../course-config/lab6-design-notes.md)；[六阶段预测与实测](crashmap.md)。

|目录或文件|用途|
|---|---|
|docs/lab6-experiment-report.md|目标、设计、结果、错误路径与归档|
|crashmap.md|课程要求的六阶段预测和实测表|
|images/lab6-wal.png、lab6-wal.mmd|四步WAL提交与恢复机制图和Mermaid源|
|images/lab6-disk-exec.png、lab6-disk-exec.mmd|磁盘ELF加载路径图和Mermaid源|

源码在code/kernel、code/user；mkfs工具在code/mkfs；构建与验收脚本在code/Makefile、code/tests。图片为机制图，真实证据为主报告UART实测节选；渲染脚本code/tools/render_lab6_diagrams.py。

仓库根目录在WSL2执行：

```bash
make -C code clean && make -C code -j2
python3 -m unittest discover -s code/tests -p 'test_*.py' -v
python3 code/tests/verify_lab6_qemu.py
python3 code/tests/verify_lab1_qemu.py
python3 code/tests/verify_lab4_qemu.py
python3 code/tests/verify_lab5_qemu.py
git diff --check
# 专用缓存/日志自测版本；测试完恢复普通构建
make -C code clean && make -C code EXTRA_CFLAGS=-DLAB6_SELFTEST
python3 code/tests/verify_lab6_qemu.py --selftests
make -C code clean && make -C code -j2
```

完整Lab2/3串口回归命令见根README。fs.img为生成的真实镜像，clean会重置它；需保留运行文件时先自行备份镜像。Lab6自动测试使用临时副本，不破坏工作镜像。
