#ifndef __statsmod_whead_h__
#define __statsmod_whead_h__

#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#define OPEN  0
#define WRITE 1
#define LSEEK 2
#define CLOSE 3
#define CLONE 4

#define NUM_INTERCEPTED_CALLS 5

#define SYSCALL_NAME_LEN 8

#define proso_rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#define current_thread_stats  ((struct my_thread_info*)current_thread_info())->land_where_wheat_grows
//#define pid_thread_stats(pid) ((my_thread_info*)find_task_by_pid(pid)->thread_info)->land_where_wheat_grows
#define task_to_thread_stats(tsk) ((struct my_thread_info*)((tsk)->thread_info))->land_where_wheat_grows
#define task_to_my_thread_pid(tsk) ((struct my_thread_info*)(task_thread_info(tsk)))->pid

static int __init statsmodwheat_init(void);
static void __exit statsmodwheat_exit(void);

/* These symbol must be exported by the kernel */
extern void *sys_call_table[];

int pid = 1;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid,"Procces wich stats will be printed and the module exit.");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Esteban <alberto84.eo@gmail.com>, Bartomeu Miró <bartomeumiro@gmail.com> {[JJ06]}");
MODULE_DESCRIPTION("ProSO stats grower");


struct t_info {
  unsigned long total;
  unsigned long success;
  unsigned long fail;
  unsigned long long time;
};

struct my_thread_info {
  struct thread_info thread_info_old;
  struct t_info land_where_wheat_grows[NUM_INTERCEPTED_CALLS];
  int pid;
};

struct t_old_syscall{
  long pos;
  int (*call)(void);
};

#endif
