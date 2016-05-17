/* 
 * File:   module.c
 * Author: henry
 *
 * Created on 16 de Maio de 2016, 19:02
 */
/*INCLUDES*/
#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/errno.h>

#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "module.h"

MODULE_AUTHOR("Henry");
MODULE_DESCRIPTION("Nothing");

static int dev_major = 0;
static volatile int isOpen = 0;
static char * name = "modTest";
//static char msgBuff[80];
static char * msg = "Hello world!!";
module_param(name, charp, 0);

//module_param(irq, int, 0);
//MODULE_PARM_DESC(irq,"aaaa");


/* DMA BUFFER FILE OPERATIONS */
static struct file_operations fops = {
    .unlocked_ioctl = NULL, /* ioctl */
    .mmap = dev_mmap,
    .open = dev_open, /* open */
    .flush = NULL,
    .release = dev_release,
    .read = dev_read
            
};

int init_module() { 
    dev_major = register_chrdev(0,name,&fops);
    if (dev_major < 0) {
            printk(KERN_ALERT "ERROR - Device register failed with code: %d\n", dev_major);
            return dev_major;            
    }
    
    return 0;
	
}
void cleanup_module () { 
    unregister_chrdev(dev_major, name);
}

static int dev_open(struct inode *inode, struct file *fle) {
    
    if (isOpen)
        return -EBUSY;
    
    Inc(&isOpen);
    try_module_get(THIS_MODULE);
    return 0;
    
}
static int dev_release(struct inode *inode, struct file *fle) {
    Dec(&isOpen);
    
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t dev_read(struct file *fle, char __user * buffer, size_t length, loff_t *offset) {
    int bytes_read = 0;
    if (*msg == 0)
        return 0;
    
    while (length && *msg) {
        put_user(*(msg++), buffer++);
        
        length--;
        bytes_read++;
    }
    return bytes_read;
}
static ssize_t dev_write (struct file *fle, const char __user *buffer, size_t length, loff_t *offset) {
    printk(KERN_ALERT "Operation not supported!\n");
    return -EINVAL;
}
static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea) {
    /*NOT IMPLEMENTED*/
    return -EINVAL;
}

