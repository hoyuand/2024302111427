#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "buf.h"
#include "course_sid.h"
#define LOG_PIN_CAP ((LAB6_BUF_SLOTS-3<LAB6_LOG_BLOCKS)?LAB6_BUF_SLOTS-3:LAB6_LOG_BLOCKS)
#define OP_MAX_BLOCKS ((LOG_PIN_CAP<MAXOPBLOCKS)?LOG_PIN_CAP:MAXOPBLOCKS)
static struct {struct spinlock lock;int start,dev,outstanding,committing;struct {int n;int block[LAB6_LOG_BLOCKS];} lh;} log;
static void head(int write){struct buf *b=bread(log.dev,log.start);if(write){memmove(b->data,&log.lh,sizeof(log.lh));bwrite(b);}else memmove(&log.lh,b->data,sizeof(log.lh));brelse(b);}
static void install(int recovering){for(int i=0;i<log.lh.n;i++){struct buf *l=bread(log.dev,log.start+1+i),*d=bread(log.dev,log.lh.block[i]);memmove(d->data,l->data,BSIZE);bwrite(d);if(!recovering)bunpin(d);brelse(d);brelse(l);if(!recovering && i+1==(log.lh.n+1)/2 && crash_should_fire(4))crash_at(4);}}
void initlog(int dev,struct superblock *sb){initlock(&log.lock,"log");log.dev=dev;log.start=sb->logstart;if(sb->nlog!=LAB6_LOG_BLOCKS+1)panic("log size");head(0);if(log.lh.n<0 || log.lh.n>LAB6_LOG_BLOCKS)panic("log header");printf("WAL recover header.n=%d\n",log.lh.n);for(int i=0;i<log.lh.n;i++)if(log.lh.block[i]<sb->inodestart || log.lh.block[i]>=sb->size)panic("log target");if(log.lh.n){install(1);log.lh.n=0;head(1);}}
void begin_op(void){acquire(&log.lock);while(log.committing || log.lh.n+(log.outstanding+1)*OP_MAX_BLOCKS>LOG_PIN_CAP)sleep(&log,&log.lock);log.outstanding++;release(&log.lock);}
void log_write(struct buf *b){acquire(&log.lock);if(log.outstanding<1)panic("log outside op");int i;for(i=0;i<log.lh.n;i++)if(log.lh.block[i]==b->blockno)break;if(i==log.lh.n){if(i>=LOG_PIN_CAP || i>=LAB6_LOG_BLOCKS)panic("log capacity");log.lh.block[i]=b->blockno;log.lh.n++;bpin(b);}b->dirty=1;release(&log.lock);}
static void commit(void){if(!log.lh.n)return;for(int i=0;i<log.lh.n;i++){struct buf *l=bread(log.dev,log.start+1+i),*d=bread(log.dev,log.lh.block[i]);memmove(l->data,d->data,BSIZE);bwrite(l);brelse(d);brelse(l);if(i+1==(log.lh.n+1)/2 && crash_should_fire(1))crash_at(1);}if(crash_should_fire(2))crash_at(2);head(1);if(crash_should_fire(3))crash_at(3);install(0);if(crash_should_fire(5))crash_at(5);log.lh.n=0;head(1);}
void end_op(void){acquire(&log.lock);if(log.outstanding<1)panic("end_op count");int last=--log.outstanding==0;if(last)log.committing=1;else wakeup(&log);release(&log.lock);if(last){commit();acquire(&log.lock);log.committing=0;wakeup(&log);release(&log.lock);}}
void log_selftest(void){begin_op();struct buf *b=bread(ROOTDEV,1999);for(int i=0;i<5;i++){b->data[0]=i;log_write(b);}if(log.lh.n!=1)panic("log absorb");brelse(b);end_op();b=bread(ROOTDEV,1999);if(b->data[0]!=4 || log.lh.n)panic("log final");brelse(b);printf("LOG-SELFTEST PASS writes=5 entries=1 pin-cap=%d\n",LOG_PIN_CAP);}
