#include "statsmod-wheat.h"

extern void * sys_call_table[];

int (*sys_open_old)(const char __user *, unsigned int, unsigned int);
int (*sys_close_old)(unsigned int);
int (*sys_write_old)(unsigned int, const char __user *, size_t);
int (*sys_clone_old)(struct pt_regs regs);
int (*sys_lseek_old)(unsigned int, off_t, unsigned int);

/* Module status 0 for disabled, 1 for enabled */
char enabled;

/* Globals to use less stack */
long error;
unsigned long long time;

/****************************************************************************/
/**************************** Auxiliar functions ****************************/
/****************************************************************************/
static inline unsigned long long proso_get_cycles() {
  unsigned long eax, edx;
  
  proso_rdtsc(eax, edx);
  return ((unsinged long long edx <<32) + eax;
}

void save_stats(int syscall) {
  current_thread_info()->land_where_wheat_grows[syscall].total++;
  if (error < 0) {
    current_thread_info()->land_where_wheat_grows[syscall].fail++;
  } else {
    current_thread_info()->land_where_wheat_grows[syscall].sucess++;
  }
  current_thread_info()->land_where_wheat_grows[syscall].time += time;
}

void intercept_sys_calls() {
  sys_open_old = sys_call_table[__NR_OPEN];
  sys_write_old = sys_call_table[__NR_WRITE];
  sys_clone_old = sys_call_table[__NR_CLONE];
  sys_close_old = sys_call_table[__NR_CLOSE];
  sys_lseek_old = sys_call_table[__NR_LSEEK];
  
  sys_call_table[__NR_OPEN] = sys_open_local;
  sys_call_table[__NR_WRITE] = sys_write_local;
  sys_call_table[__NR_CLONE] = sys_clone_local;
  sys_call_table[__NR_CLOSE] = sys_close_local;
  sys_call_table[__NR_LSEEK] = sys_lseek_local;
}

void restore_sys_calls() {
  sys_call_table[__NR_OPEN] = sys_open_old;
  sys_call_table[__NR_WRITE] = sys_write_old;
  sys_call_table[__NR_CLONE] = sys_clone_old;
  sys_call_table[__NR_CLOSE] = sys_close_old;
  sys_call_table[__NR_LSEEK] = sys_lseek_old;
}

/****************************************************************************/
/************ Our custom syscalls to intercept the original ones ************/
/****************************************************************************/
long sys_open_local(const char __user * filename, int flags, int mode) {
  time = proso_get_cycles();
  error = sys_open_old(filename, flags, mode);
  time = proso_get_cycles() - time;
  
  save_stats(OPEN);
  
  return error;
}

long sys_close_local(unsigned int fd) {
  time = proso_get_cycles();
  error = sys_close_old(fd);
  time = proso_get_cycles() - time;
  
  save_stats(CLOSE);
  
  return error;
}

ssize_t sys_write_local(unsigned int fd, const char __user * buf, size_t count) {
  time = proso_get_cycles();
  error = sys_write_old(fd, buf, count);
  time = proso_get_cycles() - time;

  save_stats(WRITE);
  
  return error;
}

int sys_clone_local(struct pt_regs regs) {
  time = proso_get_cycles();
  error = sys_clone_old(regs);
  time = proso_get_cycles() - time;

  save_stats(CLONE);
  
  return error;
}

off_t sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin) {
  time = proso_get_cycles();
  error = sys_lseek_old(fd, offset, origin);
  time = proso_get_cycles() - time;

  save_stats(LSEEK);
  
  return error;
}

/****************************************************************************/
/***************************** Public interface *****************************/
/****************************************************************************/
int get_stats(my_thread_info t_info*, int pid, int syscall);

int freeze_stats() {
  if (!enabled) {
    return -1;
  }
  restore_sys_calls();
  enabled = 0;
  return 0;
}

int microwave_stats() {
  if (enabled) {
    return -1;
  }
  intercept_sys_calls();
  enabled = 1;
  return 0;
}

int reset_stats(pid_t pid) {
//   something->land_where_wheat_grows[syscall].total = 0;
//   something->land_where_wheat_grows[syscall].fail = 0;
//   something->land_where_wheat_grows[syscall].success = 0;
//   something->land_where_wheat_grows[syscall].time = 0;
}

/****************************************************************************/
/************************* Module control (ins/rm) **************************/
/****************************************************************************/
static int __init statsmodwheat_init(void) {
  printk(KERN_DEBUG "Starting the wheat planting...");
  
  /* Adds statitical fields to task struct */
  ;
  ;
  printk(KERN_DEBUG "\t Task struct fields added.");

  /* Initializes task strcut statistics */
  ;
  ;
  printk(KERN_DEBUG "\t Task struct fields initialized.");
  
  /* Intercepting sys_call. Don't be evil, unlike Google */
  intercept_sys_calls();
  enabled = 1;
  printk(KERN_DEBUG "\t Syscalls intercepted.");
  
  printk(KERN_DEBUG "Wheat planted, waiting until it grows...");
  return 0;
}

static void __exit statsmodwheat_exit(void) {
  restore_sys_calls();
  enabled = 0;
}

module_init(statsmodwheat_init);
module_exit(statsmodwheat_exit);

