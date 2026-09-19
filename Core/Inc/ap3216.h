#ifndef __AP3216_H
#define __AP3216_H

#include "main.h"
#include <stdint.h>

#define AP3216_ADDR 0x1E // AP3216C I2C address

uint8_t ap3216_init(void);
uint8_t ap3216_read_data(uint16_t *ir, uint16_t *als, uint16_t *ps);

#endif /* __AP3216_H */