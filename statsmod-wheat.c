#include "statsmod-wheat.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Esteban <alberto84.eo@gmail.com>, \
               Bartomeu Miró <bartomeumiro@gmail.com>");
MODULE_DESCRIPTION("ProSO stats grower");

extern void * sys_call_table[];

int (*sys_open_old)(const char __user *, unsigned int, unsigned int);
int (*sys_close_old)(unsigned int);
int (*sys_write_old)(unsigned int, const char __user *, size_t);
int (*sys_clone_old)(struct pt_regs regs);
int (*sys_lseek_old)(unsigned int, off_t, unsigned int);

/* Our custom syscalls to intercept the original ones */
long sys_open_local(const char __user * filename, int flags, int mode) {
  return sys_open_old(filename, flags, mode);
}

long sys_close_local(unsigned int fd) {
  return sys_close_old(fd);
}

ssize_t sys_write_local(unsigned int fd, const char __user * buf, size_t count) {
  return sys_write_old(fd, buf, count);
}

int sys_clone_local(struct pt_regs regs) {
  return sys_clone_old(regs);
}

off_t sys_lseek_local(unsigned int fd, off_t offset, unsigned int origin) {
  return sys_lseek_old(fd, offset, origin);
}

/* Module inicialization */
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

