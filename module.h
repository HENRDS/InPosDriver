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

static int dev_open(struct inode *inode, struct file *fle);
static int dev_release(struct inode *inode, struct file *fle);
static ssize_t dev_read(struct file *fle, char __user * buffer, size_t length, loff_t *offset);
static ssize_t dev_write (struct file *fle, const char __user *buffer, size_t length, loff_t *offset);
static int dev_mmap (struct file *fle, struct vm_area_struct *vmarea);


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

