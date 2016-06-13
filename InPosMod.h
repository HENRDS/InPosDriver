/* 
 * File:   module.h
 * Author: henry
 *
 * Created on 16 de Maio de 2016, 19:02
 */

#ifndef MODULE_H
#define MODULE_H
#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include<linux/kthread.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include<linux/sched.h>
#include "InPosStructures.h"
/*DEFNITIONS*/
#define THREAD_SAFE_
#define PCI_VENDOR_ID_ALTERA 0x1172
#define PCI_DEVICE_ID_CYCLONE_IV 0x0004
#define RING_BUFFER_LENGTH 3
#define IN_POS_MAJOR 121
#define SIZE_OF_IMG 3600 // (45*80)



static int dev_open(struct inode *inode, struct file *fle);
static int dev_flush(struct file *fle, fl_owner_t id);
static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea);
static int dev_poll(void* data);
static inline void set_command_flag(int *cmd, int flag);

#endif /* MODULE_H */

