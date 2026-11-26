# 六阶段断电预测与实测

预测在编码及测试前保存；实测来自2026-11-26真实QEMU热复位后UART，测试脚本verify_lab6_qemu.py。配套课程crash.c原字节不改。

|阶段|预测头|预测恢复|预测内容|实测|
|---|---|---|---|---|
|0|0|不重放|OLD-CONTENT|header.n=0；OLD-CONTENT；一致|
|1|0|不重放|OLD-CONTENT|header.n=0；OLD-CONTENT；一致|
|2|0|不重放|OLD-CONTENT|header.n=0；OLD-CONTENT；一致|
|3|非零|重放|STAGE-3-COMMITTED|header.n=2；STAGE-3-COMMITTED；一致|
|4|非零|重放|STAGE-4-COMMITTED|header.n=2；STAGE-4-COMMITTED；一致|
|5|非零|重放|STAGE-5-COMMITTED|header.n=2；STAGE-5-COMMITTED；一致|

Stage1写日志到一半，2写完日志未写头，3非零头落盘后，4安装一半，5安装完未清头。恢复重放不触发Stage4钩子，也不bunpin。完成Stage5后额外无文件修改地复位，再读仍为STAGE-5-COMMITTED且头为0，验证恢复幂等性。
