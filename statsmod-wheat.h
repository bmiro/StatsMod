#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/unistd.h>


#define NUM_INTERCEPTED_CALLS 5

#define OPEN  0
#define LSEEK 1
#define CLONE 2
#define CLOSE 3
#define WRITE 4

typedef struct {
  unsigned long total;
  unsigned long success;
  unsigned long fail;
  unsigned long time;
} groove;

typedef struct {
  struct thread_info thread_info_old;
  struct groove land_where_wheat_grows[NUM_INTERCEPTED_CALLS];
  int pid;
} my_thread_info;