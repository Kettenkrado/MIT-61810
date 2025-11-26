#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fcntl.h"
#include "file.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Helper: find next available VMA slot
static struct vma*
vma_alloc(struct proc *p)
{
  for(int i = 0; i < NVMA; i++){
    if(p->vmas[i].valid == 0){
      p->vmas[i].valid = 1;
      return &p->vmas[i];
    }
  }
  return 0;
}

// its signature for reference:
// void *mmap(void *addr, size_t len, int prot, int flags,
// int fd, off_t offset);
uint64
sys_mmap(void)
{
  uint64 addr, len, offset;
  int prot, flags, fd;
  struct file *f = 0;
  struct proc *p = myproc();
  struct vma *v;

  argaddr(0, &addr);
  argaddr(1, &len);
  argint(2, &prot);
  argint(3, &flags);
  argint(4, &fd);
  argaddr(5, &offset);

  len = PGROUNDUP(len);
  if (len == 0) return (uint64)-1;

  if (fd >= 0) {
    f = p->ofile[fd];
    if (f == 0) return (uint64)-1;
    else {
      // mmaptest: test mmap read-only
      // 应该先检查文件的权限是否满足映射要求
      if ((flags & MAP_SHARED) && (prot & PROT_WRITE) && !f->writable) {
        return (uint64)-1;
      }
      if ((prot & PROT_READ) && !f->readable) {
        return (uint64)-1;
      }
    }
  }

  v = vma_alloc(p);
  v->addr = p->mmap_base;
  p->mmap_base += len;
  v->len = len;
  v->prot = prot;
  v->flags = flags;
  v->f = f;
  v->offset = offset;

  // 增加该文件的引用计数
  if (v->f) {
    filedup(v->f);
  }

  return v->addr;
}

uint64
sys_munmap(void)
{
  uint64 addr, len;
  struct proc *p = myproc();
  struct vma *v = 0;

  argaddr(0, &addr);
  argaddr(1, &len);

  len = PGROUNDUP(len);

  for(int i = 0; i < NVMA; i++){
    if(p->vmas[i].valid && addr >= p->vmas[i].addr && addr < p->vmas[i].addr + p->vmas[i].len){
      v = &p->vmas[i];
      break;
    }
  }

  if(!v) return -1; 

  uint64 unmap_start = addr;
  uint64 unmap_end = addr + len;
  if(unmap_end > v->addr + v->len) {
    // 超出 VMA 范围，截断
    unmap_end = v->addr + v->len; 
  }

  // mmaptest: test dirty
  // 写回不应该更改文件大小
  pte_t *pte;
  for(uint64 a = unmap_start; a < unmap_end; a += PGSIZE) {
    pte = walk(p->pagetable, a, 0);
    if(pte == 0 || (*pte & PTE_V) == 0) {
      continue; // 该页未映射，跳过
    }

    // 如果是共享且脏的，写回
    if((v->flags & MAP_SHARED) && (*pte & PTE_D)) {
      uint64 file_offset = v->offset + (a - v->addr);

      begin_op();
      ilock(v->f->ip);
      uint64 file_size = v->f->ip->size;
      uint n_to_write = PGSIZE;
      if(file_offset + PGSIZE > file_size) {
        n_to_write = file_size - file_offset;
      }
      writei(v->f->ip, 1, a, file_offset, n_to_write);
      iunlock(v->f->ip);
      end_op();
    }
  }

  uint64 npages = (unmap_end - unmap_start) / PGSIZE;
  if (npages > 0) {
    lazy_uvmunmap(p->pagetable, unmap_start, npages, 1);
  }
  
  // 调整 VMA 结构
  if(unmap_start == v->addr && unmap_end == v->addr + v->len) {
    if(v->f) fileclose(v->f);
    v->valid = 0;
  }
  else if (unmap_start == v->addr) {
    v->offset += (unmap_end - v->addr);
    v->len -= (unmap_end - v->addr);
    v->addr = unmap_end;
  }
  else if (unmap_end == v->addr + v->len) {
    v->len = unmap_start - v->addr;
  }
  // 从中间 unmap (lab 假设不会发生，但一个完整实现需要分裂VMA)
  else {
    // Split the VMA into two. This requires another vma_alloc.
    // For this lab, we can ignore this complex case as per instructions.
    printf("munmap: hole punching not supported\n");
  }

  return 0;
}