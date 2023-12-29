#define MAX_PRIORITY_QUEUE_SIZE NPROC

// Long-term locks for processes
struct prioritylock {
  uint locked;       // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock
  int queue[MAX_PRIORITY_QUEUE_SIZE];
  uint queue_front;
  uint queue_size;
  
  // For debugging:
  char *name;        // Name of lock.
  int pid;           // Process holding lock
};

