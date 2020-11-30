#ifndef __SPI_SLAVE_H
#define __SPI_SLAVE_H

#include "stdint.h"

typedef struct{
    uint8_t reg_addr;
    uint32_t * val_ptr;
    uint8_t val_size;
}SPI_COM_VAL;

#endif