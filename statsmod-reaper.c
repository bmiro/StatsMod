#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "statsmod-reaper.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Esteban <alberto84.eo@gmail.com>, \
               Bartomeu Miró <bartomeumiro@gmail.com>");
MODULE_DESCRIPTION("ProSO stats grower");

static int __init statsmodreaper_init(void) {
  /* Codi d’inicialització */
  printk(KERN_DEBUG "Now is the time, reapers, \n");
  printk(KERN_DEBUG "Now is the time to be alert., \n");
  printk(KERN_DEBUG "For when another June comes, \n");
  printk(KERN_DEBUG "Let us sharpen our tools well. \n");
  printk(KERN_DEBUG "A good blow with the sickle!!! \n");
  return 0;
}


static void __exit statsmodreaper_exit(void) {
;
}

module_init(statsmodreaper_init);
module_exit(statsmodreaper_exit);

