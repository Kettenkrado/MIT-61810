// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

// 关于内存的核心结构体 kmem
struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// 内核初始化时，分配一个自旋锁，并且释放 end 到 PHYSTOP 之间的所有内存页
// (end 是内核代码和数据段的末尾，即第一个可用的内存地址，PHYSTOP 是物理内存的上限)
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

// 对齐，并且把每一个页面都加入 freelist 
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  // 如果并不是一个页面，或者超出范围，panic
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  // 1 填充
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // 获取内核内存的锁，并且将这个空闲页头插入空闲链表
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  // 获取锁，取出头页面
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  // 注意这里是用 5 填充新的页面
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
