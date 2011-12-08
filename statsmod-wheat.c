#include "statsmod-wheat.h"

/** Backuped original syscall function array */
t_old_syscall syscall_old[NUM_INTERCEPTED_CALLS];

/** Itercepted syscall function array */
int (*syscall_local[NUM_INTERCEPTED_CALLS])(void);

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

/** Check if the specified syscall is intercepted or not accorting
 *  @return 1 if valid 0 if not */
int valid_syscall(int syscall) {
  return (0 < syscall) && (syscall < NUM_INTERCEPTED_CALLS);
}

/** Saves stats of the current syscall of the current thread.
 * @time: time consumed defined as global variable in order to use less stack
 * @error: result of the syscall defined as global variable in order to use less stack
 */
int save_current_stats(int syscall) {
  if (!valid_syscall(syscall)) return -1;

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

  //TODO ensure that tsk is valid?

  if (tsk->pid == task_to_thread_pid(tsk)) return 0;

  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
    task_to_thread_stats(tsk)[i].total = 0;
    task_to_thread_stats(tsk)[i].fail = 0;
    task_to_thread_stats(tsk)[i].success = 0;
    task_to_thread_stats(tsk)[i].time = 0;
  }

  ((my_thread_info*)((tsk)->thread_info))->pid = tsk->pid;

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
  long (*opn)(const char __user *, int, int);

  try_module_get(THIS_MODULE);

  opn = syscall_old[OPEN].call;

  time = proso_get_cycles();
  error = opn(filename, flags, mode);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(OPEN);

  module_put(THIS_MODULE);
  return error;
}

long sys_close_local(unsigned int fd) {
  long (*cls)(unsigned int);

  try_module_get(THIS_MODULE);

  cls = syscall_old[CLOSE].call;

  time = proso_get_cycles();
  error = cls(fd);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(CLOSE);

  module_put(THIS_MODULE);
  return error;
}

ssize_t sys_write_local(unsigned int fd, const char __user * buf, size_t count) {
  ssize_t (*wrt)(unsigned int, const char __user *, size_t);

  try_module_get(THIS_MODULE);

  wrt = syscall_old[WRITE].call;

  time = proso_get_cycles();
  error = wrt(fd, buf, count);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(WRITE);

  module_put(THIS_MODULE);
  return error;
}

int sys_clone_local(struct pt_regs regs) {
  int (*cln) (struct pt_regs);

  try_module_get(THIS_MODULE);

  cln = syscall_old[CLONE].call;

  time = proso_get_cycles();
  error = cln(regs);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(CLONE);

  module_put(THIS_MODULE);
  return error;
}

off_t sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin) {
  off_t (*lsk)(unsigned int, off_t, unsigned int);

  try_module_get(THIS_MODULE);

  lsk = syscall_old[LSEEK].call;

  time = proso_get_cycles();
  error = lsk(fd, offset, origin);
  time = proso_get_cycles() - time;

  stats_check_and_set(current);
  save_current_stats(LSEEK);

  module_put(THIS_MODULE);
  return error;
}

/****************************************************************************/
/***************************** Public interface *****************************/
/****************************************************************************/
int get_stats(my_thread_info *t_info, pid_t desitred_pid, int syscall) {
  struct task_struct *tsk;
  int thi_size;

  thi_size = sizeof(my_thread_info);

  if (!valid_syscall(syscall))return -EINVAL;
  if (!access_ok(VERIFY_WRITE, t_info, thi_size)) return -EFAULT;

  tsk = find_task_by_pid(desitred_pid);
  if (tsk == NULL) return -ESRCH;

  try_module_get(THIS_MODULE);

  /* If not inizcialized will the copy_to_user will copy 0 not rubish */
  stats_check_and_set(tsk);

  error = copy_to_user(t_info, &(task_to_thread_stats(tsk)[syscall]), thi_size);

  module_put(THIS_MODULE);
  return error;
}

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

int microwave_stats(void) {
  try_module_get(THIS_MODULE);
  if (enabled) {
    module_put(THIS_MODULE);
    return -1;
  }
  intercept_sys_calls();
  enabled = 1;

  module_put(THIS_MODULE);
  return 0;
}

int reset_stats(pid_t desitred_pid, int syscall) {
  struct task_struct *tsk;

  if (!valid_syscall(syscall)) return -EINVAL;

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
  //char sc_name[SYSCALL_NAME_LEN];
  struct task_struct *tsk;

  printk(KERN_DEBUG "[smw] Bye!\n");

  tsk = find_task_by_pid(pid);

  for (i = 0; i < NUM_INTERCEPTED_CALLS; i++) {
//     switch (i) {
//       case WRITE:
//         strcpy(sc_name, "WRITE");
//         break;
//       case CLONE:
//         strcpy(sc_name, "CLONE");
//         break;
//       case CLOSE:
//         strcpy(sc_name, "CLOSE");
//         break;
//       case LSEEK:
//         strcpy(sc_name, "LSEEK");
//         break;
//       case OPEN:
//         strcpy(sc_name, "OPEN");
//         break;
//       default:
//         strcpy(sc_name, "UNKWN");
//     }

    printk(KERN_DEBUG "[smw] Pid: %d %d\n", task_to_thread_pid(tsk), pid);

    if (tsk->pid != task_to_thread_pid(tsk)) {
      //printk(KERN_DEBUG "[smw] The %s syscall isn't initialized\n", sc_name);
    } else {
      //printk(KERN_DEBUG "[smw] %s syscall:\n", sc_name);
      printk(KERN_DEBUG "[smw] Total: %lu\n", task_to_thread_stats(tsk)[i].total);
      printk(KERN_DEBUG "[smw] Success: %lu\n", task_to_thread_stats(tsk)[i].success);
      printk(KERN_DEBUG "[smw] Fail: %lu\n", task_to_thread_stats(tsk)[i].fail);
      printk(KERN_DEBUG "[smw] Mean time: %llu\n", task_to_thread_stats(tsk)[i].time);
    }
  }

  restore_sys_calls();
  enabled = 0;
}

module_init(statsmodwheat_init);
module_exit(statsmodwheat_exit);

