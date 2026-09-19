#ifndef __MYIIC_H
#define __MYIIC_H

#include "main.h"

void IIC_Start(void);
void IIC_Stop(void);
void IIC_Ack(void);
void IIC_NAck(void);
uint8_t IIC_WaitAck(void);
void IIC_SendByte(uint8_t dat);
uint8_t IIC_ReceiveByte(uint8_t ack);
uint8_t iic_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);
uint8_t iic_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len);
void iic_scan(void);

#endif /* __MYIIC_H */