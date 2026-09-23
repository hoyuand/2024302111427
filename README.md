# 2024302111427

操作系统实践 A 的课程代码、运行验证和实验报告仓库。代码基于课程发放的 Lab 骨架完成，实验结果以 QEMU 实测输出和对应的验收文档为准。

GitHub 仓库：<https://github.com/hoyuand/2024302111427>（克隆地址：`https://github.com/hoyuand/2024302111427.git`）。

## 仓库导航

| 目录 | 内容 | 检查入口 |
|---|---|---|
| [`code/`](code/) | 内核源码、构建文件、测试脚本和运行工具 | [`code/kernel/`](code/kernel/)、[`code/tests/`](code/tests/)、[`code/tools/`](code/tools/) |
| [`course-config/`](course-config/) | 课程说明、个人参数和各轮要求对照 | [`course-config/README.md`](course-config/README.md) |
| [`doc/`](doc/) | 教师重点检查的实验结果、图片和文档 | [`doc/README.md`](doc/README.md) |

## 实验完成情况

| 实验 | 完成内容 | 教师验收材料 |
|---|---|---|
| Lab0 | 完成课程代码阅读、环境自检，以及 `echo` 控制流、`exec` 状态快照、时钟中断旅程三张图 | [`doc/lab0/`](doc/lab0/) |
| Lab1 | 完成启动汇编、M 态到 S 态切换、PMP、UART、`printf` 和个性化 banner，并通过 QEMU 回归检查 | [`doc/lab1/`](doc/lab1/) |
| Lab2 | 完成 U/S 态陷阱、系统调用、UART 输入环形缓冲、内嵌程序加载和最小进程生命周期 | [`doc/lab2/`](doc/lab2/) |
| Lab3–Lab7 | 待后续实验完成后追加 | 对应的 `doc/labN/` |

每个实验目录包含一个精简索引 README、`images/` 图片目录和 `docs/` 中文主报告。教师可从对应的 `doc/labN/README.md` 进入主报告；PNG 是展示图，需要编辑时可使用同目录的 Mermaid 源文件。说明书要求映射集中保存在 [`course-config/`](course-config/)。

## 常用运行命令

在 WSL2 中从仓库根目录执行：

```bash
make -C code clean && make -C code
python3 code/check_expect.py 2024302111427 code/expect_banner.txt
python3 -m unittest discover -s code/tests -p "test_*.py" -v
python3 code/tests/verify_lab1_qemu.py
python3 code/support/inject_uart.py --tree code --script code/support/lab2-smoke.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-bufstorm.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-overflow.script
python3 code/support/inject_uart.py --tree code --script code/support/lab2-spin.script
```

Lab1 的真实 QEMU 截图、启动图及中文报告位于 [`doc/lab1/`](doc/lab1/)。

Lab2 的真实 QEMU 截图、trap 全景图、控制台时序图和中文报告位于 [`doc/lab2/`](doc/lab2/)；启动后可在 `sh>` 运行 `hi`、`badecall`、`spin` 和 `bufstorm`。本轮已实测 UART 外部中断、非法 syscall、超长输入和忙循环输入。

## Git 归档

每轮实验完成后，将代码、测试结果和验收材料一起提交，并使用对应标签归档：

```bash
git add -A
git commit -m "第n次实验：完成代码、测试和验收材料"
git tag labN
git push origin main
git push origin labN
git ls-remote --heads --tags origin refs/heads/main refs/tags/labN
```

Lab1 按说明书另使用 `lab1-submit` 标签和 ZIP 归档：

```bash
git tag lab1-submit
git push origin lab1-submit
git archive --format=zip -o 提交-lab1-2024302111427.zip lab1-submit
git ls-remote --heads --tags origin refs/heads/main refs/tags/lab1 refs/tags/lab1-submit
```

截至 Lab1 的历史标签为 `lab0`、`lab1`、`lab1-submit`；对应提交、ZIP 归档和核验方式汇总于 [`doc/lab1/docs/lab1-experiment-report.md`](doc/lab1/docs/lab1-experiment-report.md)。

Lab2 使用 `lab2-start`、`lab2`、`lab2-submit` 三个标签，并导出本地归档 `提交-lab2-2024302111427.zip`；相对 `lab1-submit` 的代码增量包为 `code/archives/lab2-code-delta-2024302111427.zip`。归档状态见 [`doc/lab2/docs/lab2-experiment-report.md`](doc/lab2/docs/lab2-experiment-report.md)。

Lab2 归档命令：

```bash
git push origin main
git push origin lab2-start lab2 lab2-submit
git archive --format=zip -o 提交-lab2-2024302111427.zip lab2-submit
git ls-remote origin refs/heads/main refs/tags/lab2-start^{} refs/tags/lab2^{} refs/tags/lab2-submit^{}
```

## 课程说明

本人参加了 2026 年全国大学生计算机系统能力大赛操作系统设计赛 OS 内核实现赛道，晋级全国总决赛并获优胜奖。经与任课老师沟通，后续 Lab 实验可免去验证环节，并按验收通过计入实验成绩。
