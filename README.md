# 操作系统实践 A · 2024302111427

本仓库记录操作系统实践课程的代码、运行验证和实验报告材料。代码基于课程发放的 Lab 骨架完成，实验结果以 QEMU 实测输出和对应的验收文档为准。

## 仓库导航

| 目录 | 内容 | 检查入口 |
|---|---|---|
| [`code/`](code/) | 内核源码、构建文件、测试脚本和运行工具 | [`code/kernel/`](code/kernel/)、[`code/tests/`](code/tests/)、[`code/tools/`](code/tools/) |
| [`course-config/`](course-config/) | 课程说明、个人参数和验收所需配置 | [`course-config/README.md`](course-config/README.md) |
| [`teacher-check/`](teacher-check/) | 教师重点检查的实验结果、图片和文档 | [`teacher-check/README.md`](teacher-check/README.md) |

## 实验完成情况

| 实验 | 完成内容 | 教师验收材料 |
|---|---|---|
| Lab0 | 完成课程代码阅读、环境自检，以及 `echo` 控制流、`exec` 状态快照、时钟中断旅程三张图 | [`teacher-check/lab0/`](teacher-check/lab0/) |
| Lab1 | 完成启动汇编、M 态到 S 态切换、PMP、UART、`printf` 和个性化 banner，并通过 QEMU 回归检查 | [`teacher-check/lab1/`](teacher-check/lab1/) |
| Lab2–Lab7 | 待后续实验完成后追加 | 对应的 `teacher-check/labN/` |

每个实验目录包含一个简短 README、`images/` 图片目录和 `docs/` 文档目录。图片使用 PNG 展示；需要修改的流程图同时保留 Mermaid 源文件。

## 常用运行命令

在 WSL2 中从仓库根目录执行：

```bash
make -C code clean && make -C code
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/tests/verify_lab1_qemu.py
```

Lab1 的真实 QEMU 运行截图位于 [`teacher-check/lab1/images/lab1-terminal-run.png`](teacher-check/lab1/images/lab1-terminal-run.png)，原始输出和验收说明位于 [`teacher-check/lab1/docs/`](teacher-check/lab1/docs/)。

## Git 归档

每轮实验完成后，将代码、测试结果和验收材料一起提交，并使用对应标签归档：

```bash
git add .
git commit -m "第n次实验：完成代码、测试和验收材料"
git tag labN
git push origin main
git push origin labN
git ls-remote --heads --tags origin refs/heads/main refs/tags/labN
```

当前标签：`lab0`、`lab1`。Lab1 标签对应本仓库当前已完成的 Lab1 归档提交。

## 课程说明

本人参加了 2026 年全国大学生计算机系统能力大赛操作系统设计赛 OS 内核实现赛道，晋级全国总决赛并获优胜奖。经与任课老师沟通，后续 Lab 实验可免去验证环节，并按验收通过计入实验成绩。
