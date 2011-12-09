#include "statsmod-reaper.h"

/* Current monitorized pid */
int pid;

/* Pid of the process who opened the device */
int open_pid;

/* Current monitorized syscall */
int sysc;

char opened;

/****************************************************************************/
/************************* Module control (ins/rm) **************************/
/****************************************************************************/
static int __init statsmodreaper_init(void) {
  int error;

  printk(KERN_DEBUG "[smr] \tNow is the time, reapers, \n");
  printk(KERN_DEBUG "[smr] \tNow is the time to be alert., \n");
  printk(KERN_DEBUG "[smr] \tFor when another June comes, \n");
  printk(KERN_DEBUG "[smr] \tLet us sharpen our tools well. \n");
  printk(KERN_DEBUG "[smr] \tA good blow with the sickle!!! \n");

  smr_handler = MKDEV(MJR, MNR);

  error = register_chrdev_region(smr_handler, 1, "smr");
  if (error) {
    printk(KERN_DEBUG "[smr] \tRegister region falied. \n");
    return error;
  }

  smr_cdev = cdev_alloc(); //TODO comprovar return

  smr_cdev->owner = THIS_MODULE;
  smr_cdev->ops = &smr_ops;

  error = cdev_add(smr_cdev, smr_handler, 1);
  if (error) {
    printk(KERN_DEBUG "[smr] \tFailed adding handler to system.\n");
  }

  opened = 0;

  return 0;
}


static void __exit statsmodreaper_exit(void) {
  printk(KERN_DEBUG "[smr] \tJust as we cut golden ears of wheat,\n");
  printk(KERN_DEBUG "[smr] \twhen the time calls we cut off chains.\n");

  unregister_chrdev_region(smr_handler, 1);
  cdev_del(smr_cdev);
}


/****************************************************************************/
/**************************** Our file operations ***************************/
/****************************************************************************/
ssize_t smr_read(struct file *f, char __user *buffer, size_t size, loff_t *offet) {
  struct t_info stats;
  int error;
  int to_read;

  to_read = (size < sizeof(struct t_info)) ? size : sizeof(struct t_info);

  printk(KERN_DEBUG "[smr] Trying to read!\n");

  /* Params check */
  if (size < 0) return -EINVAL; /* Check before the access_ok */
  if (!access_ok(VERIFY_WRITE, buffer, to_read)) return -EFAULT;

  //TODO check *f, offset just ignore?

  printk(KERN_DEBUG "[smr] size %d, to_read %d, pid %d, sysc %d\n", size, to_read, pid, sysc);

  error = get_stats(buffer, pid, sysc);

//   printk(KERN_DEBUG "[smr] error %d\n", error);
// 
//   if (error < 0) return error;
// 
//   error = copy_to_user(buffer, &stats, to_read);
// 
//   printk(KERN_DEBUG "[smr] \tCopied to user\n");
// 
//   //return to_read - error;
  return error;
}

int smr_ioctl (struct inode *i, struct file *f, unsigned int arg1, unsigned long arg2) {
  struct task_struct *t;
  int error;
  int p;
  int c;

  switch (arg1) {
    case CHANGE_PROCESS:
      if ((void *)arg2 == NULL) {
        pid = open_pid;
      } else {
        error = copy_from_user(&p, (void *)arg2, sizeof(unsigned long));
        if (error < 0) return error;

        error = (long)find_task_by_pid(p);
        if ((int)error == NULL) return -ESRCH;

        pid = p;
      }
      break;

    case CHANGE_SYSCALL:
      if (!valid_intercepted_syscall(arg2)) return -EINVAL;
      sysc = arg2;
      break;

    case RESET_CUR_PROCESS:
      for (c = 0; c < NUM_INTERCEPTED_CALLS; c++) {
        error = reset_stats(pid, c);
        printk(KERN_DEBUG "pid: %d!", pid);
        if (error < 0) return error;
      }
      break;

    case RESET_ALL_PROCESS:
      for_each_process(t) {
        for (c = 0; c < NUM_INTERCEPTED_CALLS; c++) {
          error = reset_stats(p = t->pid, c);
          if (error < 0) return error;
        }
      }
      break;

    default:
      return -EBADRQC;
  }
  return 0;
}

int smr_open(struct inode *i, struct file *f) {

  if (current->uid != 0) return -EACCES;
  if (opened) return -EBUSY;

  pid = current->pid;
  open_pid = current->pid;
  sysc = OPEN;

  opened = 1;
  return 0;

}

int smr_release (struct inode *i, struct file *f) {
  if (!opened) return -ENODEV;

  opened = 0;
  return 0;
}


module_init(statsmodreaper_init);
module_exit(statsmodreaper_exit);

