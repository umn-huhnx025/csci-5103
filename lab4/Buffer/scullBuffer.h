#ifndef _SCULLBUFFER_H_
#define _SCULLBUFFER_H_

#include <linux/cdev.h>
#include <linux/configfs.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/wait.h>

#include <asm/uaccess.h>

#define SCULL_MAJOR 0   /* dynamic major by default */
#define SCULL_NR_DEVS 1 /* scullBuffer0 */
#define SCULL_SIZE 4096 /* default size of buffer */


// item struct
struct item {
  int size;
  char buffer[512];
};


struct scull_buffer {
  struct item *bufferPtr;      /* pointer to the data buffer */
  struct semaphore sem; /* mutual exclusion semaphore     */
  struct semaphore semNumItems;
  struct semaphore semNumItemsFree;
  int readerCnt;        /* count of no of readers accessing the device */
  int writerCnt;        /* count of no of writers accessing the device */
  int size;             /* amount of data held in the buffer currently */
  int readIndex;
  int writeIndex;
  struct cdev cdev;     /* Char device structure		*/
};


/*
 * The different configurable parameters
 */
extern int scull_major;
extern int scull_minor;
extern int scull_size;

/* Function prototypes */
void scull_cleanup_module(void);
static void scull_setup_cdev(struct scull_buffer *dev);
int scull_init_module(void);
int scullBuffer_open(struct inode *inode, struct file *filp);
int scullBuffer_release(struct inode *inode, struct file *filp);
ssize_t scullBuffer_read(struct file *filp, char __user *buf, size_t count,
                         loff_t *f_pos);
ssize_t scullBuffer_write(struct file *filp, const char __user *buf,
                          size_t count, loff_t *f_pos);

#endif /* _SCULLBUFFER_H_ */
