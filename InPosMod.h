/* 
 * File:   module.h
 * Author: henry
 *
 * Created on 16 de Maio de 2016, 19:02
 */

#ifndef MODULE_H
#define MODULE_H

/*DEFNITIONS*/
#define THREAD_SAFE_
#define PCI_VENDOR_ID_ALTERA 0x1172
#define PCI_DEVICE_ID_CYCLONE_IV 0x0004
//#define RING_BUFFER_LENGTH 2
#define IN_POS_MAJOR 121
//#define PLUS_1_MOD_RB(a) ((a + 1) % RING_BUFFER_LENGTH)
#define SIZE_OF_IMG 3600 // (45*80)



static int dev_open(struct inode *inode, struct file *fle);
static int dev_flush(struct file *fle, fl_owner_t id);
static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea);
static int dev_poll(void* data);
static inline void set_command_flag(int *cmd, int flag);

struct ImgStruct {
    unsigned char img[45][80];
    volatile bool canRead;
    volatile bool canWrite;
};

struct PhysImg {
    unsigned char img[45][80];
    volatile bool canRead;
};

struct PCI_Region {
   int resource_num; 
   unsigned int phys_addr;
   unsigned int size; 
   
};
#endif /* MODULE_H */

