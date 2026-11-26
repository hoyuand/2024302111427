# Lab6 V3 要求对照

原件 SHA256：[输入清单](lab6-sources.sha256)；预置文件原字节：[接口清单](lab6-course-files.sha256)。[设计笔记](lab6-design-notes.md)在实现前保存，六阶段预测在测试前保存。[中文主报告](../doc/lab6/docs/lab6-experiment-report.md)。

|说明书要求|代码与验收入口|
|---|---|
|LFU / 11槽 / 27日志块|course_sid.h不变，bio.c freq/stamp；log.c pin cap=8|
|睡眠锁、单例缓存、脏淘汰|sleeplock.c、bio.c；LAB6_SELFTEST 内核自测|
|四步提交、吸收、恢复|log.c；LOG-SELFTEST与六次整机复位|
|预置驱动不改|virtio_disk.c、crash.c和接口原字节清单|
|硬件接线|vm.c两页MMIO；trap.c PLIC IRQ1使能与分发|
|inode、间接块、延迟unlink|fs.c；fstest三轮16KiB读写回收|
|目录与文件描述符|sysfile.c、file.c、proc.c；fstest错误回滚与fsstress并发|
|磁盘ELF exec|proc.c加载PT_LOAD、权限、入口、参数事务；Makefile去userimg.o|
|标准输入输出与生命周期|forkret挂载、CONSOLE设备fd；fork引用复制、exit关闭与cwd释放|
|前序回归|原verify_lab1/4/5与inject_uart接入同一VirtIO磁盘参数，验收断言保留|
|文档与归档|doc/lab6唯一主报告、PNG机制图、crashmap预测实测；lab6/lab6-submit与ZIP|

基线没有defs.h且syscall参数帮助函数为私有三参数形式，因此新增defs.h及fs_adapter.c适配，保留a7/a0–a2 ABI。脚手架uvm_check_range假定布尔返回，本仓库成功为0，已按负值失败适配。保护页延续Lab3的未映射洞，不将其加入COW。
