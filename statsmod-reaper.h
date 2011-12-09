#ifndef __statsmod_reaper_h__
#define __statsmod_reaper_h__

#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/fs.h>

#include "statsmod-wheat.h"

/* ioctl operations */
#define CHANGE_PROCESS    0
#define CHANGE_SYSCALL    1
#define RESET_CUR_PROCESS 2
#define RESET_ALL_PROCESS 3

//TODO dynamic!
#define MJR 169
#define MNR 0

static int __init statsmodreaper_init(void);
static void __exit statsmodreaper_exit(void);

/* Functions exported from wheat module (the #include is only for syscall defines) */
// extern int get_stats(struct my_thread_info *t_info, pid_t pid, int syscall);
// extern int reset_stats(pid_t pid, int syscall);
// extern int ignore_syscall(int syscall);
// extern int lookat_syscall(int syscall);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Esteban <alberto84.eo@gmail.com>, Bartomeu Miró <bartomeumiro@gmail.com> {[JJ06]}");
MODULE_DESCRIPTION("ProSO stats reaper");

struct cdev *smr_cdev;
dev_t smr_handler;

ssize_t smr_read (struct file *f, char __user *buffer, size_t size, loff_t *offet);
int smr_ioctl (struct inode *i, struct file *f, unsigned int arg1, unsigned long arg2);
int smr_open (struct inode *i, struct file *f);
int smr_release (struct inode *i, struct file *f);

struct file_operations smr_ops = {
  owner: THIS_MODULE,
  read: smr_read,
  ioctl: smr_ioctl,
  open: smr_open,
  release: smr_release,
};

#endif
