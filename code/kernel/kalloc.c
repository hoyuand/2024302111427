/* Lab5: allocator-owned counts, preserving Lab3 midpoint allocation order. */
#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "course_sid.h"
#include "lab5.h"
#include "proc.h"

extern char kernel_end[];
extern void printf(const char *, ...);
#define TOTAL_PAGES ((PHYSTOP - KERNBASE) / PGSIZE)
#define BIT_BYTES ((TOTAL_PAGES + 7) / 8)
static struct {
  spinlock_t lock;
  uchar bits[BIT_BYTES];
  uint16 refcnt[TOTAL_PAGES];
  uint64 start, end;
  uint total, free, shared;
  uint64 copies;
} kmem;

static void fail(const char *msg) __attribute__((noreturn));
static void fail(const char *msg)
{
  printf("panic: %s\n", msg);
  for (;;) asm volatile("wfi");
}
static uint index(uint64 pa)
{
  if (pa < kmem.start || pa >= kmem.end || (pa & (PGSIZE - 1)))
    fail("kref: invalid physical page");
  return (uint)((pa - KERNBASE) >> PGSHIFT);
}
static int bit_get(uint i) { return (kmem.bits[i >> 3] >> (i & 7)) & 1; }
static void bit_set(uint i, int v)
{
  if (v) kmem.bits[i >> 3] |= (uchar)(1U << (i & 7));
  else kmem.bits[i >> 3] &= (uchar)~(1U << (i & 7));
}
void kinit(void)
{
  kmem.start = PGROUNDUP((uint64)kernel_end);
  kmem.end = PHYSTOP - (uint64)LAB3_MEMCAP_DELTA * PGSIZE;
  if (kmem.start > kmem.end) fail("kinit: no physical page pool");
  kmem.total = (uint)((kmem.end - kmem.start) / PGSIZE);
  /* Dedicated initial insertion: refs=0/free=1, no kfree underflow. */
  for (uint i = 0; i < kmem.total; i++) bit_set(i, 1);
  kmem.free = kmem.total;
}
void *kalloc(void)
{
  acquire(&kmem.lock);
  if (!kmem.free) { release(&kmem.lock); return 0; }
  uint wanted = 0;
#if LAB3_ALLOC_ORDER == 1
  for (uint i = kmem.total; i; i--)
    if (bit_get(i - 1)) { wanted = i - 1; goto found; }
#elif LAB3_ALLOC_ORDER == 2
  uint rank = kmem.free / 2;
  for (uint i = 0; i < kmem.total; i++)
    if (bit_get(i) && rank-- == 0) { wanted = i; goto found; }
#else
  for (uint i = 0; i < kmem.total; i++)
    if (bit_get(i)) { wanted = i; goto found; }
#endif
  fail("kalloc: bitmap mismatch");
found:
  bit_set(wanted, 0);
  kmem.free--;
  uint64 pa = kmem.start + (uint64)wanted * PGSIZE;
  kmem.refcnt[index(pa)] = 1;
  release(&kmem.lock);
  for (uint i = 0; i < PGSIZE; i++) ((uchar *)pa)[i] = 0;
  return (void *)pa;
}
void kfree(void *page)
{
  uint64 pa = (uint64)page;
  uint idx = index(pa);
  acquire(&kmem.lock);
  uint ref = kmem.refcnt[idx];
  if (!ref) fail("kfree: refcnt underflow");
  if (ref == 2) kmem.shared--;
  if (--kmem.refcnt[idx] == 0) {
    for (uint i = 0; i < PGSIZE; i++) ((uchar *)pa)[i] = 0x5a;
    bit_set((uint)((pa - kmem.start) >> PGSHIFT), 1);
    kmem.free++;
  }
  release(&kmem.lock);
}
void kref_inc(uint64 pa)
{
  uint idx = index(pa);
  acquire(&kmem.lock);
  if (!kmem.refcnt[idx] || kmem.refcnt[idx] == 65535) fail("kref_inc: invalid refcnt");
  if (kmem.refcnt[idx]++ == 1) kmem.shared++;
  release(&kmem.lock);
}
uint kref_get(uint64 pa)
{
  uint idx = index(pa);
  acquire(&kmem.lock);
  uint n = kmem.refcnt[idx];
  release(&kmem.lock);
  return n;
}
uint kfreecount(void)
{
  acquire(&kmem.lock); uint n = kmem.free; release(&kmem.lock); return n;
}
uint64 kshared_count(void)
{
  acquire(&kmem.lock); uint n = kmem.shared; release(&kmem.lock); return n;
}
uint64 kcow_copy_count(void)
{
  acquire(&kmem.lock); uint64 n = kmem.copies; release(&kmem.lock); return n;
}
void kcow_record_copy(void)
{
  acquire(&kmem.lock); kmem.copies++; release(&kmem.lock);
}
void kref_selftest(void)
{
  uint free0 = kfreecount();
  uchar *p = kalloc();
  if (!p || kref_get((uint64)p) != 1) fail("kref_selftest: alloc");
  p[0] = 0x37; p[PGSIZE - 1] = 0x49;
  kref_inc((uint64)p); kref_inc((uint64)p);
  if (kref_get((uint64)p) != 3 || kshared_count() != 1) fail("kref_selftest: share");
  for (int n = 2; n >= 1; n--) {
    kfree(p);
    if (kref_get((uint64)p) != (uint)n || p[0] != 0x37 || p[PGSIZE-1] != 0x49)
      fail("kref_selftest: premature poison");
  }
  kfree(p);
  if (kfreecount() != free0 || kshared_count() || kref_get((uint64)p))
    fail("kref_selftest: conservation");
  printf("KREF-SELFTEST PASS refs=1,3,2,1,0 free=%d shared=0\n", free0);
}
