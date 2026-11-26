/* 预置支撑代码（请勿修改） —— 磁盘文件系统格式定义（内核与 mkfs 共享接口事实）
 *
 * 磁盘块布局（从第 0 号块起按顺序排列）：
 *   [ boot block (0) | super block (1) | log (logstart..logstart+nlog-1) |
 *     inode blocks (inodestart..) | free bit map (bmapstart..) | data blocks ]
 */
#ifndef FS_H
#define FS_H

#define ROOTINO 1      // 根目录 inode 编号
#define BSIZE   1024   // 磁盘逻辑块大小（字节）

// 超级块（固定位于第 1 号磁盘块）
struct superblock {
  uint magic;          // 文件系统魔数，必须等于 FSMAGIC
  uint size;           // 文件系统镜像总块数
  uint nblocks;        // 数据块数量
  uint ninodes;        // 磁盘 inode 总槽位数
  uint nlog;           // 日志区总块数（1 个日志头块 + LAB6_LOG_BLOCKS 个日志数据块）
  uint logstart;       // 第一个日志块的块号（日志头块）
  uint inodestart;     // 第一个 inode 块的块号
  uint bmapstart;      // 第一个空闲位图块的块号
};

#define FSMAGIC 0x10203040

#define NDIRECT   12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE   (NDIRECT + NINDIRECT)

// 磁盘 inode 结构（每个 64 字节，每块 BSIZE=1024 存放 IPB=16 个）
struct dinode {
  short type;                  // 文件类型（0=空闲，T_DIR，T_FILE，T_DEVICE）
  short major;                 // 主设备号（仅 T_DEVICE）
  short minor;                 // 次设备号（仅 T_DEVICE）
  short nlink;                 // 指向该 inode 的目录硬链接数
  uint size;                   // 文件字节大小
  uint addrs[NDIRECT + 1];     // 前 12 项为直接块号，第 13 项（addrs[12]）为一级间接块号
};

// 每个磁盘块包含的 dinode 数量
#define IPB (BSIZE / sizeof(struct dinode))

// 给定 inode 编号 i，计算其所在的磁盘块号
#define IBLOCK(i, sb) ((i) / IPB + (sb).inodestart)

// 每个位图块包含的比特数
#define BPB (BSIZE * 8)

// 给定数据块号 b，计算管理其空闲状态的位图磁盘块号
#define BBLOCK(b, sb) ((b) / BPB + (sb).bmapstart)

// 目录项最大文件名长度
#define DIRSIZ 14

// 目录项结构（16 字节）：目录文件的数据区由连续 struct dirent 数组组成
struct dirent {
  ushort inum;                 // inum == 0 表示空闲目录项槽位
#if defined(__GNUC__) && !defined(__clang__)
  char name[DIRSIZ] __attribute__((nonstring));
#else
  char name[DIRSIZ];
#endif
};

#endif
