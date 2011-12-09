#ifndef __statsmod_common_h__
#define __statsmod_common_h__

#define OPEN  0
#define WRITE 1
#define LSEEK 2
#define CLOSE 3
#define CLONE 4

#define NUM_INTERCEPTED_CALLS 5

/* ioctl operations */
#define CHANGE_PROCESS    0
#define CHANGE_SYSCALL    1
#define RESET_CUR_PROCESS 2
#define RESET_ALL_PROCESS 3

struct t_info {
  unsigned long total;
  unsigned long success;
  unsigned long fail;
  unsigned long long time;
};

#endif
