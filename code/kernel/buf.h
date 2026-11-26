#ifndef LAB6_BUF_H
#define LAB6_BUF_H
#include "sleeplock.h"
#include "fs.h"
struct buf {int valid,disk,dirty;uint dev,blockno,refcnt;struct sleeplock lock;uint64 freq,stamp,last_use;uchar data[BSIZE];};
#endif
