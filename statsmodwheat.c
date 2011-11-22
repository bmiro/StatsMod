#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

//#include "statsmodwheat.h"

MODULE_LICENSE("GPL");

extern void * sys_call_table[];

static int __init statsmodwheat_init(void) {
  printk(KERN_DEBUG "Starting the wheat planting...");
  
  /* Intercepting sys_call. Don't be evil, unlike Google */
//   sys_open_old = sys_call_table[POS_SYSCALL_OPEN];
//   sys_write_old = sys_call_table[POS_SYSCALL_WRITE];
//   sys_clone_old = sys_call_table[POS_SYSCALL_CLONE];
//   sys_close_old = sys_call_table[POS_SYSCALL_CLOSE];
//   sys_lseek_old = sys_call_table[POS_SYSCALL_LSEEK];
// 
//   sys_call_table[POS_SYSCALL_OPEN] = sys_open_local;
//   sys_call_table[POS_SYSCALL_WRITE] = sys_write_local;
//   sys_call_table[POS_SYSCALL_CLONE] = sys_clone_local;
//   sys_call_table[POS_SYSCALL_CLOSE] = sys_close_local;
//   sys_call_table[POS_SYSCALL_LSEEK] = sys_lseek_local;


  
  printk(KERN_DEBUG "Wheat planted, waiting until it grows...");
  return 0;
}


static void __exit statsmodwheat_exit(void) {
;
}

module_init(statsmodwheat_init);
module_exit(statsmodwheat_exit);

