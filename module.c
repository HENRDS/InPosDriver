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
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "module.h"

MODULE_AUTHOR("Henry");
MODULE_DESCRIPTION("Nothing");

static void * inport;
static int dev_major = 0;
#ifdef THREAD_SAFE_
static volatile int isOpen = 0;
#else
static int isOpen = 0;
#endif
static const char * name = "InPos_Module";
static const char * msg = "Hello world!!";

/*----------CHAR DRIVER----------*/

static struct file_operations fops = {
    .unlocked_ioctl = NULL, /* ioctl */
    .mmap = dev_mmap,
    .open = dev_open, /* open */
    .flush = NULL,
    .release = dev_release,
    .read = dev_read
            
};

/*----------PCI DRIVER----------*/

static struct pci_device_id pci_ids[] = {
    { PCI_DEVICE(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_CYCLONE_IV), },
    {0, } 
};
MODULE_DEVICE_TABLE(pci, pci_ids);

static struct pci_driver pcidev_driver = {
   .name = name,
   .id_table= pci_ids,
   .probe = pci_probe,
   .remove = pci_remove
};

/*----------INITIALIZATION & CLEANUP----------*/

int __init init_module() { 
    
    int r = pci_register_driver(pcidev_driver);
    if (r < 0)
        printk(KERN_ALERT "ERROR - PCI Device register failed with code: %d\n", r);
    
    dev_major = register_chrdev(0,name, &fops);
    if (dev_major < 0)
        printk(KERN_ALERT "ERROR - Device register failed with code: %d\n", dev_major);            
    
    return dev_major | r;
}
void  __exit cleanup_module () { 
    unregister_chrdev(dev_major, name);
    pci_unregister_driver(pcidev_driver);
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

static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea) {
    /*NOT IMPLEMENTED*/
    return -EINVAL;
}





static int pci_probe (struct pci_dev *dev, const struct pci_device_id *id) {
    unsigned long resource;
    pci_enable_device(dev);
    
    if (dev->revision != 0x01) {
        printk(KERN_ALERT "cannot find pci device!\n");
        return -ENODEV;
    }
    resource = pci_resource_start(dev, 0);
#ifdef VERBOSE_
    int vendor;
    pci_read_config_dword(dev, 0, &vendor);
    printk(KERN_ALERT "%s - vendor id: %d\n", name, vendor);
    printk(KERN_ALERT "%s - Resource base address 0 starts at: %lx\n", name, resource);
#endif
    inport = ioremap_nocache(resource + 0xC020, 0x20);
}

static void pci_remove (struct pci_dev *dev) {
    
}