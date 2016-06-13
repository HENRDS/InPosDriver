/* 
 * File:   module.c
 * Author: henry
 *
 * Created on 16 de Maio de 2016, 19:02
 */

#include "InPosMod.h"


MODULE_AUTHOR("Henry");
MODULE_DESCRIPTION("Indoor positioning system device driver");

static const char * name = "InPos_Module";
static int dev_major = 0;
#ifdef THREAD_SAFE_
static volatile int isOpen = 0;
#else
static int isOpen = 0;
#endif
static struct pci_dev *inPos_device = 0;
static bool still_polling;
static struct ImgStruct * pImage;
static struct PhysImg *pHardImg;
static struct PCI_Region region = {
    .resource_num = -1
};

static struct file_operations fops = {
    .mmap = dev_mmap,
    .open = dev_open,
    .flush = dev_flush
};

/*----------INITIALIZATION & CLEANUP----------*/

int __init init_module(void) {             
    int cmd, i, flags;
    struct resource * r;
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
    
    if (!dma_set_mask(&inPos_device->dev, 0xffffffff)) {
         printk (KERN_ALERT "DMA 32-bit not supported\n");
         return -ENOTSUPP;
    }
       
    /* Find desired region */
    
    for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
        
        region.size = pci_resource_len(inPos_device, i);
        region.phys_addr = pci_resource_start(inPos_device, i);
        flags = pci_resource_flags(inPos_device, i);
        if (!(flags & IORESOURCE_IO || region.size)) // is not IO and has size > 0
            continue;
        if ((region.size >= sizeof(struct PhysImg)) && !(flags & IORESOURCE_READONLY)) {
            region.resource_num = i;
            break;
        }
    }
    if (region.resource_num < 0) {
        printk(KERN_ALERT "ERROR - Device memory region with size >= %d not found!\n", sizeof(struct PhysImg));
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

void  __exit cleanup_module(void) { 
    unregister_chrdev(dev_major, name);
    kfree(inPos_device);
}

inline void set_command_flag(int *cmd, int flag) {
    if (!(*cmd & flag)) 
        *cmd |= flag;
}

static int dev_open(struct inode *inode, struct file *fle) {
    
    if (!try_module_get(THIS_MODULE))
        return -EAGAIN;
    
    if (isOpen)
        return -EBUSY;
    isOpen++;
    pImage = kmalloc(sizeof(struct ImgStruct) * RING_BUFFER_LENGTH, GFP_DMA);
    return 0;
}
static int dev_flush(struct file *fle, fl_owner_t id) {
    isOpen--;
    kfree(pImage);
    module_put(THIS_MODULE);
    return 0;
}

static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea) {
    unsigned int vma_size;

    vma_size = vmarea->vm_start - vmarea->vm_end;
    
    if ((vma_size + vmarea->vm_pgoff * PAGE_SIZE) > sizeof(struct ImgStruct))
        return -EINVAL;
    
    if (!remap_pfn_range(vmarea, vmarea->vm_start, (unsigned int) pImage << PAGE_SHIFT, 
            sizeof(struct ImgStruct), vmarea->vm_page_prot))
        return -EAGAIN;   
    
    
    return 0;
}
