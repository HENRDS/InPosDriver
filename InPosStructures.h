/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   InPosStructures.h
 * Author: henry
 *
 * Created on 8 de Junho de 2016, 10:48
 */

#ifndef INPOSSTRUCTURES_H
#define INPOSSTRUCTURES_H

#define INPOS_MAT_HEIGHT 60 // 480/8
#define INPOS_MAT_WIDTH 80 // 640/8
#define INPOS_FRAME_RATE 0.5

#ifdef __cplusplus
extern "C" {
#endif
        
struct InPosImage {
    unsigned char img[INPOS_MAT_HEIGHT][INPOS_MAT_WIDTH];
    volatile bool canConsume;
    volatile bool frameProduced;
    volatile bool canTransfer;
    
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


#ifdef __cplusplus
}
#endif

#endif /* INPOSSTRUCTURES_H */

