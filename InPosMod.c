/* 
 * File:   module.c
 * Author: henry
 *
 * Created on 16 de Maio de 2016, 19:02
 */
#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/errno.h>

#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/segment.h>

#include "InPosMod.h"

MODULE_AUTHOR("Henry");
MODULE_DESCRIPTION("Nothing");


static int dev_major = 0;
#ifdef THREAD_SAFE_
static volatile int isOpen = 0;
#else
static int isOpen = 0;
#endif
static struct pci_dev *inPos_device = 0;
static struct ImgStruct * pImage;
static int rBufferN = 0;

static struct file_operations fops = {
    .mmap = dev_mmap,
    .open = dev_open,
    .flush = dev_flush
};


/*----------INITIALIZATION & CLEANUP----------*/

int __init init_module() {             
    inPos_device = pci_get_device(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_CYCLONE_IV, inPos_device);
    if (!inPos_device) {
        printk(KERN_ALERT "ERROR - Device register failed, no such device\n");            
        return -ENODEV;
    }
    
    dev_major = register_chrdev(0, name, &fops);
    if (dev_major < 0) {
        printk(KERN_ALERT "ERROR - Device register failed with code: %d\n", dev_major);            
        return dev_major;
    }
    
    return 0;
}

void  __exit cleanup_module() { 
    unregister_chrdev(dev_major, name);
    kfree(inPos_device);
}

static int dev_open(struct inode *inode, struct file *fle) {
    
    if (isOpen)
        return -EBUSY;
    
    Inc(&isOpen);
    try_module_get(THIS_MODULE);
    return 0;
    
}
static int dev_flush(struct inode *inode, struct file *fle) {
    Dec(&isOpen);
    
    module_put(THIS_MODULE);
    return 0;
}

static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea) {
    unsigned long size, resSize;
    while(!pImage->canRead) {
        yield();
    }
    size = vmarea->vm_start - vmarea->vm_end;
    resSize = inPos_device->resource[0].end - inPos_device->resource[0].start;
    
    if ((size + vmarea->vm_pgoff * PAGE_SIZE) > resSize)
        return -EINVAL;
    remap_pfn_range(vmarea, vmarea->vm_start,);
    rBufferN++;
    if (rBufferN == RING_BUFFER_LENGTH) {
        pImage -= (RING_BUFFER_LENGTH - 1);
    } else {
        pImage++;
    }
    return -EINVAL;
}
