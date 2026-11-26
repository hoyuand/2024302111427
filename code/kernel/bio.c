#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "buf.h"
#include "course_sid.h"
static struct {struct spinlock lock; struct buf buf[LAB6_BUF_SLOTS];uint64 clock,reads,writes;} bc;
void binit(void){initlock(&bc.lock,"bcache");for(int i=0;i<LAB6_BUF_SLOTS;i++)initsleeplock(&bc.buf[i].lock,"buf");}
static struct buf *bget(uint dev,uint block){
 retry:
  acquire(&bc.lock);
  for(int i=0;i<LAB6_BUF_SLOTS;i++){
    struct buf *b=&bc.buf[i];
    if(b->dev==dev && b->blockno==block){b->refcnt++;b->freq++;b->last_use=++bc.clock;release(&bc.lock);acquiresleep(&b->lock);return b;}
  }
  struct buf *v=0;
  for(int i=0;i<LAB6_BUF_SLOTS;i++){
    struct buf *b=&bc.buf[i];if(b->refcnt)continue;
    if(!v || !b->dev || (v->dev && (b->freq<v->freq || (b->freq==v->freq && b->stamp<v->stamp))))v=b;
    if(!b->dev)break;
  }
  if(!v)panic("bget: no buffers");
  v->refcnt=1;
  if(v->valid && v->dirty){
    release(&bc.lock);acquiresleep(&v->lock);bwrite(v);releasesleep(&v->lock);
    acquire(&bc.lock);v->refcnt--;release(&bc.lock);goto retry;
  }
  v->dev=dev;v->blockno=block;v->valid=0;v->dirty=0;v->freq=1;v->stamp=v->last_use=++bc.clock;
  release(&bc.lock);acquiresleep(&v->lock);return v;
}
struct buf *bread(uint d,uint n){struct buf *b=bget(d,n);if(!b->valid){virtio_disk_rw(b,0);bc.reads++;b->valid=1;}return b;}
void bwrite(struct buf *b){if(!holdingsleep(&b->lock))panic("bwrite unlocked");virtio_disk_rw(b,1);bc.writes++;b->dirty=0;}
void brelse(struct buf *b){if(!holdingsleep(&b->lock))panic("brelse unlocked");releasesleep(&b->lock);acquire(&bc.lock);if(!b->refcnt)panic("brelse count");b->refcnt--;release(&bc.lock);}
void bpin(struct buf *b){acquire(&bc.lock);b->refcnt++;release(&bc.lock);}
void bunpin(struct buf *b){acquire(&bc.lock);if(!b->refcnt)panic("bunpin count");b->refcnt--;release(&bc.lock);}
void bio_selftest(void){
  uint64 r=bc.reads;struct buf *b=bread(ROOTDEV,1999);brelse(b);b=bread(ROOTDEV,1999);brelse(b);
  if(bc.reads!=r+1)panic("bio hit test");
  b=bread(ROOTDEV,1998);b->data[0]=0x6b;b->dirty=1;brelse(b);
  for(int i=0;i<LAB6_BUF_SLOTS+2;i++){b=bread(ROOTDEV,1997-i);brelse(b);}
  b=bread(ROOTDEV,1998);if(b->data[0]!=0x6b || !bc.writes)panic("bio dirty test");brelse(b);
  /* Reset only unused identities, then make block 1900 hotter than peers. */
  for(int i=0;i<LAB6_BUF_SLOTS;i++){if(bc.buf[i].refcnt)panic("bio test ref");bc.buf[i].dev=0;bc.buf[i].valid=0;}
  for(int i=0;i<LAB6_BUF_SLOTS;i++){b=bread(ROOTDEV,1900+i);brelse(b);}
  b=bread(ROOTDEV,1900);brelse(b);b=bread(ROOTDEV,1950);brelse(b);
  int seen0=0,seen1=0;for(int i=0;i<LAB6_BUF_SLOTS;i++){seen0|=bc.buf[i].blockno==1900;seen1|=bc.buf[i].blockno==1901;}
  if(!seen0 || seen1)panic("bio LFU test");
  printf("BIO-SELFTEST PASS hit dirty-writeback LFU victim=1901 slots=%d\n",LAB6_BUF_SLOTS);
}
