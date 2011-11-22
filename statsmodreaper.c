#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

//#include "statsmodreaper.h"

MODULE_LICENSE("GPL");

static int __init statsmodreaper_init(void) {
  /* Codi d’inicialització */
  printk(KERN_DEBUG "Mymodule carregat amb exit\n");
  return 0;
}


static void __exit statsmodreaper_exit(void) {
;
}

module_init(statsmodreaper_init);
module_exit(statsmodreaper_exit);

