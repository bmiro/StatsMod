#include "statsmod-reaper.h"

/****************************************************************************/
/********************** Needed anticipated definitions **********************/
/****************************************************************************/




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

  smr_handler = MKDEV(MAJOR, MINOR);

  error = register_chrdev_region(smr_handler, 1, "smr");
  if (error) {
    printk(KERN_DEBUG "[smr] \tRegister region falied. \n");
    return error;
  }

  smr_cdev = cdev_alloc(); //TODO torna error?
  smr_cdev->owner THIS_MODULE;
  smr_cdev->ops = &smr_ops;

  error = cdev_add(smr_cdev, smr_handler, 1);
  if (error) {
    printk(KERN_DEBUG "[smr] \tFailed adding handler to system.\n");
  }

  return 0;
}


static void __exit statsmodreaper_exit(void) {
  printk(KERN_DEBUG "[smr] \tJust as we cut golden ears of wheat,\n");
  printk(KERN_DEBUG "[smr] \twhen the time calls we cut off chains.\n");

  unregister_chrdev_region(smr_handler, 1);
  cdev_del(smr_handler);
}


/****************************************************************************/
/**************************** Auxiliar functions ****************************/
/****************************************************************************/



/****************************************************************************/
/**************************** Our file operations ***************************/
/****************************************************************************/
ssize_t smr_read (struct file *f, char __user *buffer, size_t size, loff_t *offet) {
  get_stats()
}

int smr_ioctl (struct inode *i, struct file *f, unsigned int arg1, unsigned long arg2) {

}

int smr_open (struct inode *i, struct file *f) {
  check root
  check only one

}

int smr_release (struct inode *i, struct file *f) {
  count the release
}


module_init(statsmodreaper_init);
module_exit(statsmodreaper_exit);

