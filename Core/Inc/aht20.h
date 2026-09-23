#ifndef __AHT20_H
#define __AHT20_H

#include "main.h"
#include <stdint.h>

#define AHT20_ADDR 0x38                                             // AHT20 I2C address

uint8_t aht20_init(void);                                           // AHT20 初始化
uint8_t aht20_read_data(float *temp, float *humi);                  // AHT20 读取温湿度

#endif /* __AHT20 */