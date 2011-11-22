#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "statsmod-wheat.h"

MODULE_LICENSE("GPL");

extern void * sys_call_table[];

int (*sys_open_old)(void*, void*, int);
int (*sys_write_old)(void*, void*, int);
int (*sys_close_old)(void*, void*, int);
int (*sys_clone_old)(void*, void*, int);
int (*sys_lseek_old)(void*, void*, int);

int (*sys_open_local)(void*, void*, int);
int (*sys_write_local)(void*, void*, int);
int (*sys_close_local)(void*, void*, int);
int (*sys_clone_local)(void*, void*, int);
int (*sys_lseek_local)(void*, void*, int);

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
  sys_open_old = sys_call_table[POS_SYSCALL_OPEN];
  sys_write_old = sys_call_table[POS_SYSCALL_WRITE];
  sys_clone_old = sys_call_table[POS_SYSCALL_CLONE];
  sys_close_old = sys_call_table[POS_SYSCALL_CLOSE];
  sys_lseek_old = sys_call_table[POS_SYSCALL_LSEEK];
  
  sys_call_table[POS_SYSCALL_OPEN] = sys_open_local;
  sys_call_table[POS_SYSCALL_WRITE] = sys_write_local;
  sys_call_table[POS_SYSCALL_CLONE] = sys_clone_local;
  sys_call_table[POS_SYSCALL_CLOSE] = sys_close_local;
  sys_call_table[POS_SYSCALL_LSEEK] = sys_lseek_local;

  printk(KERN_DEBUG "\t Syscalls intercepted.");
  
  
  printk(KERN_DEBUG "Wheat planted, waiting until it grows...");
  return 0;
}


static void __exit statsmodwheat_exit(void) {
;
}

module_init(statsmodwheat_init);
module_exit(statsmodwheat_exit);

