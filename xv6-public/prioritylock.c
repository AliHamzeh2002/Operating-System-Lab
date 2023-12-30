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
  lk->queue_size = -1;
}

void
add_process_to_priority_lock(struct prioritylock *lk, int pid){
    lk->queue_size++;
    int last_idx = (lk->queue_size + lk->queue_front) % MAX_PRIORITY_QUEUE_SIZE;
    lk->queue[last_idx] = pid;
    int cur_idx = last_idx;
    for (int i = 0; i < lk->queue_size; i++){
        int prev_idx = (cur_idx - 1) % MAX_PRIORITY_QUEUE_SIZE;
        if (lk->queue[prev_idx] > lk->queue[cur_idx]){
            break;
        }
        int temp = lk->queue[cur_idx];
        lk->queue[cur_idx] = lk->queue[prev_idx];
        lk->queue[prev_idx] = temp;
        cur_idx = prev_idx;
    }
    //cprintf("queue_size : %d queue_front: %d\n", lk->queue_size, lk->queue[lk->queue_front]);
}

void
pop_priority_queue(struct prioritylock *lk){
    lk->queue_front = (lk->queue_front + 1) % MAX_PRIORITY_QUEUE_SIZE;
    lk->queue_size--;
}

int
acquirepriority(struct prioritylock *lk){
  acquire(&lk->lk);
  if (lk->pid == myproc()->pid){
    release(&lk->lk);
    return -1;
  }
  add_process_to_priority_lock(lk, myproc()->pid);
  while (lk->locked || lk->queue[lk->queue_front] != myproc()->pid) {
    sleep(lk, &lk->lk);
  }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  pop_priority_queue(lk);
  release(&lk->lk);
  return 0;
}

int
releasepriority(struct prioritylock *lk){
  acquire(&lk->lk);
  if (lk->pid != myproc()->pid) {
    release(&lk->lk);
    return -1;
  }
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk);
  release(&lk->lk);
  return 0;
}

void
print_priority_queue(struct prioritylock *lk){
  acquire(&lk->lk);
  cprintf("Queue: [");
  for (int i = 0;i < lk->queue_size + 1; i++){
    int idx = (i + lk->queue_front) % MAX_PRIORITY_QUEUE_SIZE;
   cprintf("%d, ", lk->queue[idx]);
  }
  cprintf("]\n");
  release(&lk->lk);

}
