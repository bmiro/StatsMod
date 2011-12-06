#include "statsmod-wheat.h"

/** Backuped original syscall function array */
t_old_syscall syscall_old[NUM_INTERCEPTED_CALLS];

/** Itercepted syscall function array */
int (*syscall_local[NUM_INTERCEPTED_CALLS])();

/* Module status 0 for disabled, 1 for enabled */
char enabled;

/* Globals to use less stack */
unsigned long long time;
long error;
int i;

/****************************************************************************/
/********************** Needed anticipated definitions **********************/
/****************************************************************************/
long sys_open_local(const char __user * filename, int flags, int mode);
long sys_close_local(unsigned int fd);
ssize_t sys_write_local(unsigned int fd, const char __user * buf, size_t count);
int sys_clone_local(struct pt_regs regs);
off_t sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin);

/****************************************************************************/
/**************************** Auxiliar functions ****************************/
/****************************************************************************/
/** @return: long long with precise epoc from hardware */
static inline unsigned long long proso_get_cycles(void) {
  unsigned long eax, edx;
  
  proso_rdtsc(eax,edx);
  return ((unsigned long long) edx <<32) + eax;
}

/** Saves stats of the current syscall of the current thread.
 * @time: time consumed defined as global variable in order to use less stack
 * @error: result of the syscall defined as global variable in order to use less stack
 */
void save_current_stats(int syscall) {
  current_thread_stats[syscall].total++;
  
  if (error < 0) {
    current_thread_stats[syscall].fail++;
  } else {
    current_thread_stats[syscall].success++;
  }
  current_thread_stats[syscall].time += time;
}

int stats_check_and_set(struct task_struct *tsk, int syscall) {
  if (tsk->pid == task_to_thread_pid(tsk)) return 0;
  
  task_to_thread_stats(tsk)[syscall].total = 0;
  task_to_thread_stats(tsk)[syscall].fail = 0;
  task_to_thread_stats(tsk)[syscall].success = 0;
  task_to_thread_stats(tsk)[syscall].time = 0;
  
  task_to_thread_pid(tsk) = tsk->pid;
 
  return 0;
}

void init_syscall_arrays(void) {
  syscall_old[OPEN].pos = __NR_open;
  syscall_old[LSEEK].pos = __NR_lseek;
  syscall_old[CLONE].pos = __NR_clone;
  syscall_old[CLOSE].pos = __NR_close;
  syscall_old[WRITE].pos = __NR_write;
  
  syscall_local[OPEN] = sys_open_local;
  syscall_local[LSEEK] = sys_lseek_local;
  syscall_local[CLONE] = sys_clone_local;
  syscall_local[CLOSE] = sys_close_local;
  syscall_local[WRITE] = sys_write_local;
}

void intercept_sys_calls(void) {  
  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
    syscall_old[i].call = sys_call_table[syscall_old[i].pos];
  }

  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
   sys_call_table[syscall_old[i].pos] = syscall_local[i];
  }
}

void restore_sys_calls(void) {
  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
    sys_call_table[syscall_old[i].pos] = syscall_old[i].call;
  }
}

/****************************************************************************/
/************ Our custom syscalls to intercept the original ones ************/
/****************************************************************************/
long sys_open_local(const char __user * filename, int flags, int mode) {
  time = proso_get_cycles();
  error = syscall_old[OPEN].call(filename, flags, mode);
  time = proso_get_cycles() - time;
  
  stats_check_and_set(current, OPEN);
  save_current_stats(OPEN);
  
  return error;
}

long sys_close_local(unsigned int fd) {
  time = proso_get_cycles();
  error = syscall_old[CLOSE].call(fd);
  time = proso_get_cycles() - time;
  
  stats_check_and_set(current, CLOSE);
  save_current_stats(CLOSE);
  
  return error;
}

ssize_t sys_write_local(unsigned int fd, const char __user * buf, size_t count) {
  time = proso_get_cycles();
  error = syscall_old[WRITE].call(fd, buf, count);
  time = proso_get_cycles() - time;

  stats_check_and_set(current, WRITE);
  save_current_stats(WRITE);
  
  return error;
}

int sys_clone_local(struct pt_regs regs) {
  time = proso_get_cycles();
  error = syscall_old[CLONE].call(regs);
  time = proso_get_cycles() - time;

  stats_check_and_set(current, CLONE);
  save_current_stats(CLONE);
  
  return error;
}

off_t sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin) {
  time = proso_get_cycles();
  error = syscall_old[LSEEK].call(fd, offset, origin);
  time = proso_get_cycles() - time;

  stats_check_and_set(current, LSEEK);
  save_current_stats(LSEEK);
  
  return error;
}

/****************************************************************************/
/***************************** Public interface *****************************/
/****************************************************************************/
int get_stats(my_thread_info *t_info, pid_t pid, int syscall) {
  struct task_struct *tsk;
  int thi_size;
  
  thi_size = sizeof(my_thread_info);
  
  if (syscall < 0 || NUM_INTERCEPTED_CALLS < syscall) return -EINVAL;
  if (!access_ok(VERIFY_WRITE, t_info, thi_size)) return -EFAULT;
  
  tsk = find_task_by_pid(pid);
  if (tsk == NULL) return -ESRCH;

  /* If not inizcialized will the copy_to_user will copy 0 not rubish */
  stats_check_and_set(tsk, syscall);
  
  error = copy_to_user(t_info, &(task_to_thread_stats(tsk)[syscall]), thi_size);
  
  return error;
}

int freeze_stats(void) {
  if (!enabled) {
    return -1;
  }
  restore_sys_calls();
  enabled = 0;
  return 0;
}

int microwave_stats(void) {
  if (enabled) {
    return -1;
  }
  intercept_sys_calls();
  enabled = 1;
  return 0;
}

int reset_stats(pid_t pid, int syscall) {
  struct task_struct *tsk;

  tsk = find_task_by_pid(pid);
  if (tsk == NULL) return -ESRCH;
    
  task_to_thread_stats(tsk)[syscall].total = 0;
  task_to_thread_stats(tsk)[syscall].fail = 0;
  task_to_thread_stats(tsk)[syscall].success = 0;
  task_to_thread_stats(tsk)[syscall].time = 0;
  
  return 0;
}

/****************************************************************************/
/************************* Module control (ins/rm) **************************/
/****************************************************************************/
static int __init statsmodwheat_init(void) {
  printk(KERN_DEBUG "Aboning land...\n");
  init_syscall_arrays();

  printk(KERN_DEBUG "Starting the wheat planting...\n");

  /* Intercepting sys_call. Don't be evil, unlike Google */
  intercept_sys_calls();
  enabled = 1;
  printk(KERN_DEBUG "\t Syscalls intercepted.\n");
  
  printk(KERN_DEBUG "Wheat planted, waiting until it grows...\n");
  return 0;
}

static void __exit statsmodwheat_exit(void) {
  restore_sys_calls();
  enabled = 0;
}

module_init(statsmodwheat_init);
module_exit(statsmodwheat_exit);

