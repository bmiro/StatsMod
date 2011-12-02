#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/unistd.h>

#define OPEN  0
#define LSEEK 1
#define CLONE 2
#define CLOSE 3
#define WRITE 4

#define NUM_INTERCEPTED_CALLS 5

#define proso_rdtsc(low,high) \
__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Esteban <alberto84.eo@gmail.com>, Bartomeu Miró <bartomeumiro@gmail.com>");
MODULE_DESCRIPTION("ProSO stats grower");

typedef struct {
  unsigned long total;
  unsigned long success;
  unsigned long fail;
  unsigned long long time;
} groove;

typedef struct {
  struct thread_info thread_info_old;
  struct groove land_where_wheat_grows[NUM_INTERCEPTED_CALLS];
  int pid;
} my_thread_info;

/** Given a process pid and the function to monitor it returns the stats
 * in the given pointer.
 * @return:
 *         0 if success
 *         -ESRCH if pid doesn't exist
 *         -EINVAL if syscall is invalid
 *         -EFAULT if the buffer is not correct 
 */
int get_stats(my_thread_info t_info*, int pid, int syscall);

int freeze_stats();
int microwave_stats();

/** Resets the stats of the given pid.
 * @return:
 *        0 if success
 *        -ESRCH if pid doesn't exist
 */
int reset_stats();