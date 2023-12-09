#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

static int
find_digital_root(int n){
  if (n <= 0)
    return -1;
  while (n > 9){
    int new_n = 0;
    while (n != 0){
      new_n += n % 10;
      n /= 10;
    }
    n = new_n;
  }
  return n;
}

int
sys_find_digital_root(void){
  return find_digital_root(myproc()->tf->ebx);
}

int
sys_get_uncle_count(void){
  int pid;
  if (argint(0, &pid) < 0)
    return -1;
  return uncle_count(pid);
}

int
sys_get_process_lifetime(void){
    int pid;
  if (argint(0, &pid) < 0)
    return -1;
  return find_process_lifetime(pid);

}
int
sys_change_process_queue(void){
    int pid;
    int queue_num;
  if (argint(0, &pid) < 0)
    return -1;
  if (argint(1, &queue_num) < 0)
    return -1;
  return change_process_queue(pid,queue_num);

}

