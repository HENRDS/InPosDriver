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
#ifdef NOTEBOOK_
#define PCI_VENDOR_ID_ALTERA 0x1172
#define PCI_DEVICE_ID_CYCLONE_IV 0x0004
#else
#define PCI_VENDOR_ID_ALTERA 0x1172
#define PCI_DEVICE_ID_CYCLONE_IV 0x0004
#endif
#define RING_BUFFER_LENGTH 10

static const char * name = "InPos_Module";

static int dev_open(struct inode *inode, struct file *fle);
static int dev_flush(struct inode *inode, struct file *fle);
static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea);
typedef unsigned char uchar; 

struct ImgStruct {
    bool img[45][80];
    bool canWrite;
    bool canRead;
};
/*----------UTIL----------*/

/*I don't know if I need the increment to be atomic, but just in case...*/
#ifdef THREAD_SAFE_ 
inline int Inc (volatile int* var) {
    register int val = 1;
    asm volatile ("lock xaddl %0, %2;" :"=a" (val) :"a" (val), "m" (*var) :"memory");
    return val;
}
inline int Dec(volatile int* var) {
    register int val = -1;
    asm volatile ("lock xaddl %0, %2;" :"=a" (val) :"a" (val), "m" (*var) :"memory");
    return val;
}
#else 
inline int Inc (int* var) { (*var) += 1; return (*var); }
inline int Dec (int* var) { (*var) -= 1; return (*var); }
#endif

#endif /* MODULE_H */

