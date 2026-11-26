#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "course_sid.h"
#include "lab5.h"
#include "proc.h"

extern void printf(const char *fmt, ...);
extern char kernel_end[];
extern void trampoline(void);

static pagetable_t kernel_pagetable;
static uint64 kernel_satp;
static int map_track_active;
#define MAP_TRACK_MAX 1024
static pagetable_t map_new_pages[MAP_TRACK_MAX];
static pte_t *map_parent_slots[MAP_TRACK_MAX];
static uint map_new_count;

pte_t *
walk(pagetable_t pagetable, uint64 va, int alloc)
{
  if (va >= MAXVA)
    return 0;
  for (int level = 2; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if (*pte & PTE_V) {
      if (*pte & (PTE_R | PTE_W | PTE_X))
        return 0;
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      if (!alloc)
        return 0;
      pagetable_t next = (pagetable_t)kalloc();
      if (next == 0)
        return 0;
      if (map_track_active) {
        /* Every transaction-owned intermediate page must be recorded so a
         * later leaf failure can release it.  Refuse the mapping rather than
         * silently creating an untracked page when the journal is full. */
        if (map_new_count >= MAP_TRACK_MAX) {
          kfree(next);
          return 0;
        }
        map_new_pages[map_new_count] = next;
        map_parent_slots[map_new_count] = pte;
        map_new_count++;
      }
      *pte = PA2PTE(next) | PTE_V;
      pagetable = next;
    }
  }
  return &pagetable[PX(0, va)];
}

pte_t *
vm_walk(pagetable_t pagetable, uint64 va, int alloc)
{
  return walk(pagetable, va, alloc);
}

static int
mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 first = PGROUNDDOWN(va);
  uint64 last = PGROUNDDOWN(va + size - 1);
  uint mapped = 0;
  int owner = !map_track_active;
  if (owner) {
    map_track_active = 1;
    map_new_count = 0;
  }
  for (;;) {
    pte_t *pte = walk(pagetable, first, 1);
    if (pte == 0 || (*pte & PTE_V))
      goto bad;
    *pte = PA2PTE(pa) | perm | PTE_V;
    mapped++;
    if (first == last) {
      if (owner)
        map_track_active = 0;
      return 0;
    }
    first += PGSIZE;
    pa += PGSIZE;
  }
bad:
  first = PGROUNDDOWN(va);
  for (uint i = 0; i < mapped; i++, first += PGSIZE) {
    pte_t *pte = walk(pagetable, first, 0);
    if (pte)
      *pte = 0;
  }
  if (owner) {
    for (uint i = map_new_count; i != 0; i--) {
      *map_parent_slots[i - 1] = 0;
      kfree(map_new_pages[i - 1]);
    }
    map_new_count = 0;
    map_track_active = 0;
  }
  return -1;
}

static void
rollback_map_transaction(void)
{
  for (uint i = map_new_count; i != 0; i--) {
    *map_parent_slots[i - 1] = 0;
    kfree(map_new_pages[i - 1]);
  }
  map_new_count = 0;
  map_track_active = 0;
}

static void
freewalk(pagetable_t pagetable, int free_leaves)
{
  for (uint i = 0; i < 512; i++) {
    pte_t pte = pagetable[i];
    if (!(pte & PTE_V))
      continue;
    if (pte & (PTE_R | PTE_W | PTE_X)) {
      if (free_leaves && (pte & PTE_U))
        kfree((void *)PTE2PA(pte));
      pagetable[i] = 0;
    } else {
      freewalk((pagetable_t)PTE2PA(pte), free_leaves);
      pagetable[i] = 0;
      kfree((void *)PTE2PA(pte));
    }
  }
}

static void
unmap_one(pagetable_t pt, uint64 va, int freepa)
{
  pte_t *pte = walk(pt, va, 0);
  if (!pte || !(*pte & PTE_V) || !(*pte & (PTE_R | PTE_W | PTE_X)))
    return;
  int user = (*pte & PTE_U) != 0;
  uint64 pa = PTE2PA(*pte);
  *pte = 0;
  if (freepa && user)
    kfree((void *)pa);
}

void
kvminit(void)
{
  kernel_pagetable = (pagetable_t)kalloc();
  if (!kernel_pagetable)
    return;
  uint64 bytes = PHYSTOP - KERNBASE;
  if (mappages(kernel_pagetable, KERNBASE, bytes, KERNBASE,
               PTE_R | PTE_W | PTE_X) < 0)
    return;
  mappages(kernel_pagetable, CLINT_BASE, 0x10000, CLINT_BASE,
           PTE_R | PTE_W);
  mappages(kernel_pagetable, PLIC, 0x400000, PLIC, PTE_R | PTE_W);
  mappages(kernel_pagetable, UART0, 0x100, UART0, PTE_R | PTE_W);
  mappages(kernel_pagetable,0x10001000,PGSIZE,0x10001000,PTE_R|PTE_W);
  mappages(kernel_pagetable,0x100000,PGSIZE,0x100000,PTE_R|PTE_W);
  uint64 tramp_pa = PGROUNDDOWN((uint64)trampoline);
  mappages(kernel_pagetable, TRAMPOLINE, PGSIZE, tramp_pa, PTE_R | PTE_X);
  kernel_satp = MAKE_SATP(kernel_pagetable);
}

void
kvminithart(void)
{
  if (kernel_satp) {
    w_satp(kernel_satp);
    sfence_vma();
  }
}

uint64
kvmsatp(void)
{
  return kernel_satp;
}

pagetable_t
uvmcreate(struct proc *p)
{
  (void)p;
  pagetable_t pt = (pagetable_t)kalloc();
  if (!pt)
    return 0;
  uint64 tramp_pa = PGROUNDDOWN((uint64)trampoline);
  if (mappages(pt, TRAMPOLINE, PGSIZE, tramp_pa, PTE_R | PTE_X) < 0) {
    freewalk(pt, 0);
    kfree(pt);
    return 0;
  }
  return pt;
}

int
uvmmap(pagetable_t pagetable, uint64 va, uint64 pa, uint64 size, int perm)
{
  if (size == 0 || va + size < va || va + size > TRAPFRAME)
    return -1;
  return mappages(pagetable, va, size, pa, perm | PTE_U);
}

uint64
walkaddr(pagetable_t pagetable, uint64 va, int write)
{
  if (va >= MAXVA)
    return 0;
  pte_t *pte = walk(pagetable, va, 0);
  if (!pte || !(*pte & PTE_V) || !(*pte & PTE_U))
    return 0;
  if (!(*pte & PTE_R))
    return 0;
  if (write && !(*pte & PTE_W))
    return 0;
  return PTE2PA(*pte) + (va & (PGSIZE - 1));
}

int
copyin(pagetable_t pagetable, void *dst, uint64 srcva, uint64 len)
{
  if (srcva >= MAXVA || len > MAXVA - srcva) return -1;
  uchar *d = (uchar *)dst;
  while (len) {
    uint64 pa = walkaddr(pagetable, srcva, 0);
    if (!pa)
      return -1;
    uint n = PGSIZE - (srcva & (PGSIZE - 1));
    if (n > len)
      n = (uint)len;
    for (uint i = 0; i < n; i++)
      d[i] = ((uchar *)pa)[i];
    d += n;
    srcva += n;
    len -= n;
  }
  return 0;
}

int
copyout(pagetable_t pagetable, uint64 dstva, const void *src, uint64 len)
{
  if (uvm_check_range(pagetable, dstva, len, PTE_R | PTE_W) < 0) return -1;
  const uchar *s = (const uchar *)src;
  while (len) {
    uint64 pa = walkaddr(pagetable, dstva, 1);
    if (!pa)
      return -1;
    uint n = PGSIZE - (dstva & (PGSIZE - 1));
    if (n > len)
      n = (uint)len;
    for (uint i = 0; i < n; i++)
      ((uchar *)pa)[i] = s[i];
    s += n;
    dstva += n;
    len -= n;
  }
  return 0;
}

int
uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int perm)
{
  if (newsz < oldsz || newsz > TRAPFRAME - LAB3_GUARD_PAGES * PGSIZE)
    return -1;
  uint64 a = PGROUNDUP(oldsz);
  uint64 start = a;
  uint mapped = 0;
  map_track_active = 1;
  map_new_count = 0;
  for (; a < newsz; a += PGSIZE) {
    void *mem = kalloc();
    if (!mem)
      goto bad;
    if (uvmmap(pagetable, a, (uint64)mem, PGSIZE, perm) < 0) {
      kfree(mem);
      goto bad;
    }
    mapped++;
  }
  map_track_active = 0;
  map_new_count = 0;
  return 0;
bad:
  for (a = start; a < start + (uint64)(mapped + 1) * PGSIZE; a += PGSIZE)
    unmap_one(pagetable, a, 1);
  rollback_map_transaction();
  return -1;
}

uint64
uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
  if (newsz >= oldsz)
    return oldsz;
  uint64 a = PGROUNDUP(newsz);
  for (; a < oldsz; a += PGSIZE)
    unmap_one(pagetable, a, 1);
  return newsz;
}

int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  for (uint64 va = 0; va < sz; va += PGSIZE) {
    pte_t *pte = walk(old, va, 0);
    if (!pte || !(*pte & PTE_V) || !(*pte & PTE_U))
      continue; /* guard holes and supervisor-only mappings are not COW. */
    uint64 pa = PTE2PA(*pte);
    uint flags = PTE_FLAGS(*pte);
    if (flags & PTE_W) flags = (flags & ~PTE_W) | PTE_COW;
#if LAB5_REFCNT_SCHEME == 0
    if ((flags & PTE_COW) && kref_get(pa) >= 2) flags |= PTE_RSW1;
#endif
    *pte = PA2PTE(pa) | flags;
    if (mappages(new, va, PGSIZE, pa, flags & ~PTE_V) < 0)
      goto bad;
    kref_inc(pa);
  }
  sfence_vma();
  return 0;
bad:
  uvmfree(new, sz);
  /* A parent page can remain COW with ref=1 after rollback; its next write
   * restores W in place without allocating. */
  sfence_vma();
  return -1;
}

static spinlock_t cow_lock;

int
cow_alloc(pagetable_t pagetable, uint64 va)
{
  if (va >= MAXVA) return -1;
  acquire(&cow_lock);
  pte_t *pte = walk(pagetable, PGROUNDDOWN(va), 0);
  if (!pte || !(*pte & PTE_V) || !(*pte & PTE_U) || !(*pte & PTE_R)) goto bad;
  if (*pte & PTE_W) { release(&cow_lock); return 0; }
  if (!(*pte & PTE_COW)) goto bad;
  uint64 oldpa = PTE2PA(*pte);
  uint flags = (PTE_FLAGS(*pte) & ~(PTE_COW | PTE_RSW1)) | PTE_W;
  uint refs = kref_get(oldpa);
  if (refs == 0) goto bad;
  if (refs == 1) {
    *pte = PA2PTE(oldpa) | flags;
  } else {
    uchar *page = kalloc();
    if (!page) goto bad;
    for (uint i = 0; i < PGSIZE; i++) page[i] = ((uchar *)oldpa)[i];
    *pte = PA2PTE(page) | flags;
    kcow_record_copy();
    if (current_proc && ++current_proc->cow_copies == LAB5_COW_BUDGET)
      printf("[cow] pid=%d reached LAB5_COW_BUDGET=%d, downgrade: continue copying\n",
             current_proc->pid, LAB5_COW_BUDGET);
    kfree((void *)oldpa);
  }
  sfence_vma();
  release(&cow_lock);
  return 0;
bad:
  release(&cow_lock);
  return -1;
}

int
uvm_check_range(pagetable_t pt, uint64 va, uint64 len, int want)
{
  if (va >= MAXVA || len > MAXVA - va) return -1;
  if (!len) return 0;
  uint64 end = va + len;
  for (uint64 a = PGROUNDDOWN(va); a < end; a += PGSIZE) {
    pte_t *pte = walk(pt, a, 0);
    if (!pte || !(*pte & PTE_V) || !(*pte & PTE_U)) return -1;
    if ((want & PTE_W) && !(*pte & PTE_W)) {
      if (!(*pte & PTE_COW) || cow_alloc(pt, a) < 0) return -1;
      pte = walk(pt, a, 0);
    }
    if ((*pte & want) != (uint)want) return -1;
  }
  return 0;
}

int
copyinstr(pagetable_t pt, char *dst, uint64 va, uint64 max)
{
  for (uint64 i = 0; i < max; i++) {
    if (va + i < va || copyin(pt, dst + i, va + i, 1) < 0) return -1;
    if (!dst[i]) return (int)i + 1;
  }
  return -1;
}

void
uvmfree(pagetable_t pagetable, uint64 sz)
{
  (void)sz;
  if (!pagetable)
    return;
  freewalk(pagetable, 1);
  kfree(pagetable);
}

static void
dump_level(pagetable_t pt, int level, uint64 base)
{
  for (uint i = 0; i < 512; i++) {
    pte_t pte = pt[i];
    if (!(pte & PTE_V))
      continue;
    uint64 va = base | ((uint64)i << PXSHIFT(level));
    uint64 pa = PTE2PA(pte);
    int leaf = pte & (PTE_R | PTE_W | PTE_X);
    if (leaf) {
      char perm[5];
      perm[0] = (pte & PTE_R) ? 'r' : '-';
      perm[1] = (pte & PTE_W) ? 'w' : '-';
      perm[2] = (pte & PTE_X) ? 'x' : '-';
      perm[3] = (pte & PTE_U) ? 'u' : '-';
      perm[4] = 0;
      printf("LEAF va=%lx pa=%lx perm=%s\n", va, pa, perm);
    } else {
      printf("L%d va=%lx pa=%lx perm=----\n", level, va, pa);
      dump_level((pagetable_t)pa, level - 1, va);
    }
  }
}

void
dump_pagetable(pagetable_t pagetable)
{
  printf("DUMP-PAGETABLE begin\n");
  dump_level(pagetable, 2, 0);
  printf("DUMP-PAGETABLE end\n");
}
