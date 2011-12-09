#include "statsmod-wheat.h"

/** Backuped original syscall function array */
struct t_old_syscall syscall_old[NUM_INTERCEPTED_CALLS];

/** Itercepted syscall function array */
int (*syscall_local[NUM_INTERCEPTED_CALLS])(void);

/* Module status 0 for disabled, 1 for enabled */
char enabled;

/****************************************************************************/
/********************** Needed anticipated definitions **********************/
/****************************************************************************/
/**************************** Auxiliar functions ****************************/
void init_syscall_arrays(void);
void intercept_sys_calls(void);
void restore_sys_calls(void);
static inline unsigned long long proso_get_cycles(void);
int save_current_stats(int syscall, unsigned long long time, int error);
int stats_check_and_set(struct task_struct *tsk);
/************ Our custom syscalls to intercept the original ones ************/
long sys_open_local(const char __user * filename, int flags, int mode);
long sys_close_local(unsigned int fd);
ssize_t sys_write_local(unsigned int fd, const char __user * buf, size_t count);
int sys_clone_local(struct pt_regs regs);
off_t sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin);

/****************************************************************************/
/************************* Module control (ins/rm) **************************/
/****************************************************************************/
static int __init statsmodwheat_init(void) {
  printk(KERN_DEBUG "[smw] Aboning land...\n");
  init_syscall_arrays();

  printk(KERN_DEBUG "[smw] Starting the wheat planting...\n");

  /* Intercepting sys_call. Don't be evil, unlike Google */
  intercept_sys_calls();
  enabled = 1;
  printk(KERN_DEBUG "[smw] \t Syscalls intercepted.\n");

  printk(KERN_DEBUG "[smw] Wheat planted, waiting until it grows...\n");
  return 0;
}

static void __exit statsmodwheat_exit(void) {
  int i;

  char sc_name[SYSCALL_NAME_LEN];
  struct task_struct *tsk;

  printk(KERN_DEBUG "[smw] Bye!\n");

  tsk = find_task_by_pid(pid);

  if (tsk == NULL) {
    printk(KERN_DEBUG "[smw] The %d pid is in heaven.\n", pid);
  } else {
    for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
      switch (i) {
        case WRITE:
          strcpy(sc_name, "WRITE");
          break;
        case CLONE:
          strcpy(sc_name, "CLONE");
          break;
        case CLOSE:
          strcpy(sc_name, "CLOSE");
          break;
        case LSEEK:
          strcpy(sc_name, "LSEEK");
          break;
        case OPEN:
          strcpy(sc_name, "OPEN");
          break;
        default:
          strcpy(sc_name, "UNKWN");
      }

      printk(KERN_DEBUG "[smw] \n");

      if (tsk->pid != task_to_my_thread_pid(tsk)) {
        printk(KERN_DEBUG "[smw] The %s syscall isn't initialized\n", sc_name);
      } else {
        printk(KERN_DEBUG "[smw] %s syscall:\n", sc_name);
        printk(KERN_DEBUG "[smw] \tTotal: %lu\n", task_to_thread_stats(tsk)[i].total);
        printk(KERN_DEBUG "[smw] \tSuccess: %lu\n", task_to_thread_stats(tsk)[i].success);
        printk(KERN_DEBUG "[smw] \tFail: %lu\n", task_to_thread_stats(tsk)[i].fail);
        printk(KERN_DEBUG "[smw] \tMean time: %llu\n", task_to_thread_stats(tsk)[i].time);
      }
    }
  }

  restore_sys_calls();
  enabled = 0;
}

/****************************************************************************/
/***************************** Public interface *****************************/
/****************************************************************************/
/** Check if the syscall is valid or not.
 * @return :
 *         0 if invalid (fail)
 *         1 if valid (success)
 */
int valid_intercepted_syscall(int syscall) {
  return (-1 < syscall) && (syscall < NUM_INTERCEPTED_CALLS);
}

/** Given a process pid and the function to monitor it returns the stats
 * in the given pointer.
 * @return:
 *         0 if success
 *         -ESRCH if pid doesn't exist
 *         -EINVAL if syscall is invalid
 *         -EFAULT if the buffer is not correct 
 *	   Positive number means that there are remaining bytes to copy
 */
int get_stats(struct t_info *stats, pid_t desired_pid, int syscall) {
  struct task_struct *tsk;
  int thi_size;
  int error;

  thi_size = sizeof(struct t_info);

  if (!valid_intercepted_syscall(syscall)) return -EINVAL;
  if (!access_ok(VERIFY_WRITE, stats, thi_size)) return -EFAULT;

  tsk = find_task_by_pid(desired_pid);
  if (tsk == NULL) return -ESRCH;

  try_module_get(THIS_MODULE);

  /* If not inizcialized will the copy_to_user will copy 0 not rubish */
  stats_check_and_set(tsk);

  error = copy_to_user(stats, &(task_to_thread_stats(tsk)[syscall]), thi_size);

  module_put(THIS_MODULE);
  return error;
}

/** Stops recording the statistics. 
 * @return:
 *        0 if success
 *        -1 if error (already stoped)
 */
int freeze_stats(void) {
  try_module_get(THIS_MODULE);

  if (!enabled) {
    module_put(THIS_MODULE);
    return -1;
  }
  restore_sys_calls();
  enabled = 0;

  module_put(THIS_MODULE);
  return 0;
}

/** Continues recording the statistics. 
 * @return:
 *        0 if success
 *        -1 if error (already started)
 */
int microwave_stats(void) {
  if (enabled) {
    return -1;
  }

  try_module_get(THIS_MODULE);

  intercept_sys_calls();
  enabled = 1;

  module_put(THIS_MODULE);
  return 0;
}

/** Stops recording the statistics of the given syscall. 
 * @return:
 *        0 if success
 *        -1 if error (already stoped)
 */
int ignore_syscall(int syscall) {
  if (!valid_intercepted_syscall(syscall)) return -EINVAL;

  sys_call_table[syscall_old[syscall].pos] = syscall_old[syscall].call;
  return 0;
}

/** Continues recording the statistics of the given syscall. 
 * @return:
 *        0 if success
 *        -1 if error (already started)
 */
int lookat_syscall(int syscall) {
  if (!valid_intercepted_syscall(syscall)) return -EINVAL;

  if (sys_call_table[syscall_old[syscall].pos] != syscall_local[syscall]) {
    syscall_old[syscall].call = sys_call_table[syscall_old[syscall].pos];
    sys_call_table[syscall_old[syscall].pos] = syscall_local[syscall];
  }
  return 0;
} 

/** Resets the stats of the given pid.
 * @return:
 *        0 if success
 *        -ESRCH if pid doesn't exist
 */
int reset_stats(pid_t desitred_pid, int syscall) {
  struct task_struct *tsk;

  if (!valid_intercepted_syscall(syscall)) return -EINVAL;

  tsk = find_task_by_pid(desitred_pid);
  if (tsk == NULL) return -ESRCH;

  try_module_get(THIS_MODULE);

  task_to_thread_stats(tsk)[syscall].total = 0;
  task_to_thread_stats(tsk)[syscall].fail = 0;
  task_to_thread_stats(tsk)[syscall].success = 0;
  task_to_thread_stats(tsk)[syscall].time = 0;

  module_put(THIS_MODULE);
  return 0;
}


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
int save_current_stats(int syscall, unsigned long long time, int error) {
  if (!valid_intercepted_syscall(syscall)) return -1;

  current_thread_stats[syscall].total++;

  if (error < 0) {
    current_thread_stats[syscall].fail++;
  } else {
    current_thread_stats[syscall].success++;
  }
  current_thread_stats[syscall].time += time;

  return 0;
}

int stats_check_and_set(struct task_struct *tsk) {
  int i;
  //TODO ensure that tsk is valid?

  if (tsk->pid == task_to_my_thread_pid(tsk)) return 0;

  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
    task_to_thread_stats(tsk)[i].total = 0;
    task_to_thread_stats(tsk)[i].fail = 0;
    task_to_thread_stats(tsk)[i].success = 0;
    task_to_thread_stats(tsk)[i].time = 0;
  }

  task_to_my_thread_pid(tsk) = tsk->pid;

  return 0;
}

void init_syscall_arrays(void) {
  syscall_old[OPEN].pos = __NR_open;
  syscall_old[LSEEK].pos = __NR_lseek;
  syscall_old[CLONE].pos = __NR_clone;
  syscall_old[CLOSE].pos = __NR_close;
  syscall_old[WRITE].pos = __NR_write;

  syscall_local[OPEN] = (int (*)(void))sys_open_local;
  syscall_local[LSEEK] = (int (*)(void))sys_lseek_local;
  syscall_local[CLONE] = (int (*)(void))sys_clone_local;
  syscall_local[CLOSE] = (int (*)(void))sys_close_local;
  syscall_local[WRITE] = (int (*)(void))sys_write_local;
}

void intercept_sys_calls(void) {
  int i;
  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
    syscall_old[i].call = sys_call_table[syscall_old[i].pos];
    sys_call_table[syscall_old[i].pos] = syscall_local[i];
  }
}

void restore_sys_calls(void) {
  int i;
  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
    sys_call_table[syscall_old[i].pos] = syscall_old[i].call;
  }
}

/****************************************************************************/
/************ Our custom syscalls to intercept the original ones ************/
/****************************************************************************/
long sys_open_local(const char __user * filename, int flags, int mode) {
  unsigned long long time;
  int error;

  long (*opn)(const char __user *, int, int);

  try_module_get(THIS_MODULE);

  opn = (long (*)(const char __user *, int, int))syscall_old[OPEN].call;

  time = proso_get_cycles();
  error = opn(filename, flags, mode);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(OPEN, time, error);

  module_put(THIS_MODULE);
  return error;
}

long sys_close_local(unsigned int fd) {
  unsigned long long time;
  int error;

  long (*cls)(unsigned int);

  try_module_get(THIS_MODULE);

  cls = (long (*)(unsigned int))syscall_old[CLOSE].call;

  time = proso_get_cycles();
  error = cls(fd);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(CLOSE, time, error);

  module_put(THIS_MODULE);
  return error;
}

ssize_t sys_write_local(unsigned int fd, const char __user * buf, size_t count) {
  unsigned long long time;
  int error;

  ssize_t (*wrt)(unsigned int, const char __user *, size_t);

  try_module_get(THIS_MODULE);

  wrt = (ssize_t (*)(unsigned int, const char __user *, size_t))syscall_old[WRITE].call;

  time = proso_get_cycles();
  error = wrt(fd, buf, count);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(WRITE, time, error);

  module_put(THIS_MODULE);
  return error;
}

int sys_clone_local(struct pt_regs regs) {
  unsigned long long time;
  int error;

  int (*cln)(struct pt_regs);

  try_module_get(THIS_MODULE);

  cln = (int (*)(struct pt_regs))syscall_old[CLONE].call;

  time = proso_get_cycles();
  error = cln(regs);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(CLONE, time, error);

  module_put(THIS_MODULE);
  return error;
}

off_t sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin) {
  unsigned long long time;
  int error;

  off_t (*lsk)(unsigned int, off_t, unsigned int);

  try_module_get(THIS_MODULE);

  lsk = (off_t (*)(unsigned int, off_t, unsigned int))syscall_old[LSEEK].call;

  time = proso_get_cycles();
  error = lsk(fd, offset, origin);
  time = proso_get_cycles() - time;


  stats_check_and_set(current);
  save_current_stats(LSEEK, time, error);

  module_put(THIS_MODULE);
  return error;
}

EXPORT_SYMBOL(get_stats);
EXPORT_SYMBOL(freeze_stats);
EXPORT_SYMBOL(microwave_stats);
EXPORT_SYMBOL(reset_stats);
EXPORT_SYMBOL(valid_intercepted_syscall);

module_init(statsmodwheat_init);
module_exit(statsmodwheat_exit);

