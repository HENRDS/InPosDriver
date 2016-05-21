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
#define MIN_REG_SIZE sizeof(struct ImgStruct) * RING_BUFFER_LENGTH

MODULE_AUTHOR("Henry");
MODULE_DESCRIPTION("Indoor positioning system device driver");


static int dev_major = 0;
#ifdef THREAD_SAFE_
static volatile int isOpen = 0;
#else
static int isOpen = 0;
#endif
static struct pci_dev *inPos_device = 0;
static struct ImgStruct * pImage;
static struct PCI_Region region = {
    .resource_num = -1
};


static struct file_operations fops = {
    .mmap = dev_mmap,
    .open = dev_open,
    .flush = dev_flush
};


/*----------INITIALIZATION & CLEANUP----------*/

int __init init_module() {             
    int cmd, i, flags;
    inPos_device = pci_get_device(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_CYCLONE_IV, inPos_device);
    if (!inPos_device) {
        printk(KERN_ALERT "ERROR - Device register failed, no such device\n");            
        return -ENODEV;
    }
    
    pci_read_config_word(inPos_device, PCI_COMMAND, &cmd);
    
    /* Set flags */
    set_command_flag(&cmd, PCI_COMMAND_MEMORY); /* Enable response in Memory space */
    set_command_flag(&cmd, PCI_COMMAND_MASTER); /* Enable bus mastering */
    set_command_flag(&cmd, PCI_COMMAND_INVALIDATE); /* Use memory write and invalidate */
    
    /* Find desired region */
    
    for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
        
        region.size = pci_resource_len(inPos_device, i);
        region.phys_addr = pci_resource_start(inPos_device, i);
        flags = pci_resource_flags(inPos_device, i);
        if (!(flags & IORESOURCE_IO || region.size)) // is not IO and has size > 0
            continue;
        if ((region.size >= MIN_REG_SIZE) && !(flags & IORESOURCE_READONLY)) {
            region.resource_num = i;
            break;
        }
    }
    if (region.resource_num < 0) {
        printk(KERN_ALERT "ERROR - Device memory region with size >= %d not found!\n", MIN_REG_SIZE);
        return -EINVAL;
    }
        
    region.phys_addr &= PCI_BASE_ADDRESS_MEM_MASK;
    region.size = ~(region.size & PCI_BASE_ADDRESS_MEM_MASK) + 1;
    
    dev_major = register_chrdev(IN_POS_MAJOR, name, &fops);
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

inline void set_command_flag(int *cmd, int flag) {
    if (!(*cmd & flag)) 
        *cmd |= flag;
}

static int dev_open(struct inode *inode, struct file *fle) {
    
    if (isOpen)
        return -EBUSY;
    
    isOpen++;
    try_module_get(THIS_MODULE);
    pImage = (struct ImgStruct*) region.phys_addr;
    return 0;
    
}
static int dev_flush(struct file *fle, fl_owner_t id) {
    isOpen--;
    
    module_put(THIS_MODULE);
    return 0;
}

static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea) {
    unsigned long vma_size;
    while(!pImage->canRead) {
        yield();
    }
    vma_size = vmarea->vm_start - vmarea->vm_end;
    
    if ((vma_size + vmarea->vm_pgoff * PAGE_SIZE) > SIZE_OF_IMG)
        return -EINVAL;
    
    if (!remap_pfn_range(vmarea, vmarea->vm_start, ((unsigned int) &(pImage->img)) << PAGE_SHIFT, 
            SIZE_OF_IMG, vmarea->vm_page_prot))
        return -EAGAIN;

    pImage = (struct ImgStruct*) region.phys_addr + PLUS_1_MOD_RB((unsigned int)pImage);
    
    pImage->canWrite = 1;
    
    return 0;
}
