/* 预置支撑代码（请勿修改） —— VirtIO MMIO 磁盘设备寄存器与环形队列定义
 * 供预置驱动 kernel/virtio_disk.c 使用。
 */
#ifndef VIRTIO_H
#define VIRTIO_H

#include "types.h"

// VirtIO MMIO 控制寄存器偏移（基址 VIRTIO0 = 0x10001000）
#define VIRTIO_MMIO_MAGIC_VALUE         0x000 // 0x74726976
#define VIRTIO_MMIO_VERSION             0x004 // 2（需 QEMU 开启 force-legacy=false）
#define VIRTIO_MMIO_DEVICE_ID           0x008 // 2 = 块设备
#define VIRTIO_MMIO_VENDOR_ID           0x00c // 0x554d4551
#define VIRTIO_MMIO_DEVICE_FEATURES     0x010
#define VIRTIO_MMIO_DRIVER_FEATURES     0x020
#define VIRTIO_MMIO_QUEUE_SEL           0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX       0x034
#define VIRTIO_MMIO_QUEUE_NUM           0x038
#define VIRTIO_MMIO_QUEUE_READY         0x044
#define VIRTIO_MMIO_QUEUE_NOTIFY        0x050
#define VIRTIO_MMIO_INTERRUPT_STATUS    0x060
#define VIRTIO_MMIO_INTERRUPT_ACK       0x064
#define VIRTIO_MMIO_STATUS              0x070
#define VIRTIO_MMIO_QUEUE_DESC_LOW      0x080
#define VIRTIO_MMIO_QUEUE_DESC_HIGH     0x084
#define VIRTIO_MMIO_DRIVER_DESC_LOW     0x090
#define VIRTIO_MMIO_DRIVER_DESC_HIGH    0x094
#define VIRTIO_MMIO_DEVICE_DESC_LOW     0x0a0
#define VIRTIO_MMIO_DEVICE_DESC_HIGH    0x0a4

// Status 寄存器标志位
#define VIRTIO_CONFIG_S_ACKNOWLEDGE     1
#define VIRTIO_CONFIG_S_DRIVER          2
#define VIRTIO_CONFIG_S_DRIVER_OK       4
#define VIRTIO_CONFIG_S_FEATURES_OK     8

// 设备特性位
#define VIRTIO_BLK_F_RO                 5     // 磁盘只读
#define VIRTIO_BLK_F_SCSI               7     // 支持 SCSI 命令透传
#define VIRTIO_BLK_F_FLUSH              9     // 支持写缓存刷盘命令
#define VIRTIO_BLK_F_CONFIG_WCE         11    // 支持回写模式配置
#define VIRTIO_BLK_F_MQ                 12    // 支持多队列
#define VIRTIO_F_ANY_LAYOUT             27
#define VIRTIO_RING_F_INDIRECT_DESC     28
#define VIRTIO_RING_F_EVENT_IDX         29
#define VIRTIO_F_VERSION_1              32

// 环形队列描述符数量（必须为 2 的幂）
#define NUM 8

struct virtq_desc {
  uint64 addr;
  uint32 len;
  uint16 flags;
  uint16 next;
};
#define VRING_DESC_F_NEXT  1 // 与下一描述符链接成链
#define VRING_DESC_F_WRITE 2 // 设备写入内存（0 表示设备读取内存）

struct virtq_avail {
  uint16 flags;
  uint16 idx;
  uint16 ring[NUM];
  uint16 unused;
};

struct virtq_used_elem {
  uint32 id;
  uint32 len;
};

struct virtq_used {
  uint16 flags;
  uint16 idx;
  struct virtq_used_elem ring[NUM];
};

#define VIRTIO_BLK_T_IN  0 // 读磁盘扇区
#define VIRTIO_BLK_T_OUT 1 // 写磁盘扇区

struct virtio_blk_req {
  uint32 type;
  uint32 reserved;
  uint64 sector;
};

#endif
