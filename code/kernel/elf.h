/* 预置支撑代码（请勿修改） —— ELF64 可执行文件格式定义
 * 供 lab6 阶段四 exec 加载器解析磁盘可执行文件使用。
 */
#ifndef ELF_H
#define ELF_H

#include "types.h"

#define ELF_MAGIC 0x464C457FU  // 小端序下的 "\x7FELF"

// ELF 文件头
struct elfhdr {
  uint magic;                  // 必须等于 ELF_MAGIC
  uchar elf[12];
  ushort type;
  ushort machine;
  uint version;
  uint64 entry;                // 程序入口虚拟地址
  uint64 phoff;                // 程序头表在文件中的字节偏移
  uint64 shoff;
  uint flags;
  ushort ehsize;
  ushort phentsize;
  ushort phnum;                // 程序头表项数量
  ushort shentsize;
  ushort shnum;
  ushort shstrndx;
};

// 程序段头（Program Header）
struct proghdr {
  uint32 type;
  uint32 flags;
  uint64 off;                  // 段内容在文件中的字节偏移
  uint64 vaddr;                // 段加载的目标虚拟地址
  uint64 paddr;
  uint64 filesz;               // 段在文件中的字节长度
  uint64 memsz;                // 段在内存中的字节长度（大于 filesz 的尾部为 .bss 全零区）
  uint64 align;
};

// proghdr.type 取值
#define ELF_PROG_LOAD       1

// proghdr.flags 权限位
#define ELF_PROG_FLAG_EXEC  1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ  4

#endif
