#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "prioritylock.h"

void
initprioritylock(struct prioritylock *lk, char *name){
  initlock(&lk->lk, "priority lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
  lk->queue_front = 0;
  lk->queue_size = 0;
}

void
add_process_to_priority_lock(struct prioritylock *lk, int pid){
    lk->queue_size++;
    uint last_idx = (lk->queue_size + lk->queue_front) % MAX_PRIORITY_QUEUE_SIZE;
    lk->queue[last_idx] = pid;
    if (lk->queue[last_idx] > lk->queue[lk->queue_front]){
        lk->queue[last_idx] = lk->queue[lk->queue_front];
        lk->queue[lk->queue_front] = pid;
    }
}

void
pop_priority_queue(struct prioritylock *lk){
    lk->queue_front = (lk->queue_front + 1) % MAX_PRIORITY_QUEUE_SIZE;
    lk->queue_size--;
    for (int i = lk->queue_front; i < lk->queue_size; i++){
        if(lk->queue[i] > lk->queue[lk->queue_front]){
            int temp = lk->queue[i];
            lk->queue[i] = lk->queue[lk->queue_front];
            lk->queue[lk->queue_front] = temp;
        }
    }
}

void
acquirepriority(struct prioritylock *lk){
  acquire(&lk->lk);
  add_process_to_priority_lock(lk, myproc()->pid);
  while (lk->locked || lk->queue[lk->queue_front] != myproc()->pid) {
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  pop_priority_queue(lk);
  release(&lk->lk);
}

void
releasepriority(struct prioritylock *lk){
  acquire(&lk->lk);
  if (lk->pid != myproc()->pid) {
    release(&lk->lk);
    panic("Process is not the owner of the lock!");
  }
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
}
