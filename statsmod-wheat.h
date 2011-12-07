#ifndef __statsmod_whead_h__
#define __statsmod_whead_h__

#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#define OPEN  0
#define LSEEK 1
#define CLONE 2
#define CLOSE 3
#define WRITE 4

#define SYSCALL_NAME_LEN 6

#define NUM_INTERCEPTED_CALLS 5

#define proso_rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#define current_thread_stats  ((my_thread_info*)current_thread_info())->land_where_wheat_grows
//#define pid_thread_stats(pid) ((my_thread_info*)find_task_by_pid(pid)->thread_info)->land_where_wheat_grows
#define task_to_thread_stats(tsk) ((my_thread_info*)((tsk)->thread_info))->land_where_wheat_grows
#define task_to_thread_pid(tsk) ((my_thread_info*)((tsk)->thread_info))->pid

/* These symbol must be exported by the kernel */
extern void *sys_call_table[];

int pid = 1;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid,"Procces wich stats will be printed and the module exit.");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Esteban <alberto84.eo@gmail.com>, Bartomeu Miró <bartomeumiro@gmail.com> {[JJ06]}");
MODULE_DESCRIPTION("ProSO stats grower");


typedef struct {
  unsigned long total;
  unsigned long success;
  unsigned long fail;
  unsigned long long time;
} groove;

typedef struct {
  struct thread_info thread_info_old;
  groove land_where_wheat_grows[NUM_INTERCEPTED_CALLS];
  int pid;
} my_thread_info;

typedef struct {
  long pos;
  int (*call)();
} t_old_syscall;

/** Given a process pid and the function to monitor it returns the stats
 * in the given pointer.
 * @return:
 *         0 if success
 *         -ESRCH if pid doesn't exist
 *         -EINVAL if syscall is invalid
 *         -EFAULT if the buffer is not correct 
 *	   Positive number means that there are remaining bytes to copy
 */
int get_stats(my_thread_info *t_info, pid_t pid, int syscall);

/** Stops recording the statistics. 
 * @return:
 *        0 if success
 *        -1 if error (already stoped)
 */
int freeze_stats(void);

/** Continues recording the statistics. 
 * @return:
 *        0 if success
 *        -1 if error (already started)
 */
int microwave_stats(void);

/** Resets the stats of the given pid.
 * @return:
 *        0 if success
 *        -ESRCH if pid doesn't exist
 */
int reset_stats(pid_t pid, int syscall);

#endif
