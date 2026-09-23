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
uint8_t iic_read_byte(uint8_t ack);
uint8_t iic_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data);                 /* 向指定设备写入1个字节 */
uint8_t iic_write_nreg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t num); /* 向指定设备写入n个字节 */
uint8_t iic_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len);
uint8_t iic_read_bytes(uint8_t dev_addr, uint8_t *buf, uint16_t len);
void iic_scan(void);

#endif /* __MYIIC_H */