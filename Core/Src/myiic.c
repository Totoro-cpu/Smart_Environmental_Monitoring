#include "main.h"
#include <stdint.h>
#include "myiic.h"
#include <stdio.h>

/* IIC delay function */
static void IIC_Delay(void)
{
    // Implementation for IIC delay
    for (volatile uint32_t i = 0; i < 180; i++);
}

/* IIC_SCL_SET */
static void IIC_SCL(uint8_t state)
{
    HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* IIC_SDA_SET */
static void IIC_SDA(uint8_t state)
{
    HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* IIC_SDA_READ */
static uint8_t IIC_SDA_Read(void)
{
    return HAL_GPIO_ReadPin(IIC_SDA_GPIO_Port, IIC_SDA_Pin);
}

/* IIC_Start */
void IIC_Start(void)
{
    IIC_SDA(1);
    IIC_SCL(1);
    IIC_Delay();
    IIC_SDA(0);
    IIC_Delay();
    IIC_SCL(0);
}

/* IIC_Stop */
void IIC_Stop(void)
{
    IIC_SCL(0);
    IIC_SDA(0);
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_SDA(1);
    IIC_Delay();
}

/* IIC_Ack */
void IIC_Ack(void)
{
    IIC_SDA(0);
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_SCL(0);
    IIC_SDA(1);
    IIC_Delay();
}

/* IIC_NAck */
void IIC_NAck(void)
{
    IIC_SDA(1);
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_SCL(0);
    IIC_SDA(0);
    IIC_Delay();
}

/* IIC_WaitAck */
uint8_t IIC_WaitAck(void)
{
    uint8_t ack;
    IIC_SDA(1);
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    ack = IIC_SDA_Read();
    IIC_SCL(0);
    return ack;
}

/* IIC_SendByte */
void IIC_SendByte(uint8_t dat)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (dat & 0x80)
            IIC_SDA(1);
        else
            IIC_SDA(0);
        dat <<= 1;
        IIC_Delay();
        IIC_SCL(1);
        IIC_Delay();
        IIC_SCL(0);
        IIC_Delay();
    }
}

/* IIC_ReceiveByte */
uint8_t iic_read_byte(uint8_t ack)
{
    uint8_t receive = 0;

    IIC_SDA(1);         /* 释放 SDA，准备输入 */
    IIC_Delay();
    IIC_Delay();

    for (uint8_t t = 0; t < 8; t++)
    {
        receive <<= 1;

        IIC_SCL(0);     /* SCL 拉低 */
        IIC_Delay();
        IIC_Delay();

        IIC_SCL(1);     /* SCL 拉高，数据有效 */
        IIC_Delay();
        IIC_Delay();    /* 等 SDA 稳定 */

        if (IIC_SDA_Read()) receive++;   /* 采样 SDA */

        IIC_Delay();
    }

    /* ★ 关键：主机发 1 位 ACK，不是 8 位 ★ */
    IIC_SDA(ack ? 0 : 1);     /* ACK 拉低 SDA，NACK 释放 SDA */
    IIC_Delay();
    IIC_SCL(1);
    IIC_Delay();
    IIC_Delay();
    IIC_SCL(0);
    IIC_Delay();
    IIC_SDA(1);               /* 释放 SDA，准备下一次传输 */
    IIC_Delay();

    return receive;
}

/* 向指定设备写入一个字节 */
uint8_t iic_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data)
{
    IIC_Start();
    IIC_SendByte(dev_addr << 1);   /* 写地址 */
    if (IIC_WaitAck()) { IIC_Stop(); return 1; }
    IIC_SendByte(reg);
    if (IIC_WaitAck()) { IIC_Stop(); return 1; }
    IIC_SendByte(data);
    if (IIC_WaitAck()) { IIC_Stop(); return 1; }
    IIC_Stop();
    return 0;
}

/* 从指定设备读取一个字节 */
uint8_t iic_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    IIC_Start();
    IIC_SendByte(dev_addr << 1);   /* 写地址 */
    if (IIC_WaitAck()) { IIC_Stop(); return 1; }
    IIC_SendByte(reg);
    if (IIC_WaitAck()) { IIC_Stop(); return 1; }

    IIC_Start();                    /* 重新起始 */
    IIC_SendByte((dev_addr << 1) | 1);  /* 读地址 */
    if (IIC_WaitAck()) { IIC_Stop(); return 1; }

    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = iic_read_byte(i < (len - 1) ? 1 : 0);
    }
    IIC_Stop();
    return 0;
}

/* 扫描 I2C 总线上的所有设备 */
void iic_scan(void)
{
    printf("\r\n===== I2C Scan Start =====\r\n");
    for (uint8_t addr = 1; addr < 128; addr++)
    {
        if (addr == 0x20) continue;
        IIC_Start();
        IIC_SendByte(addr << 1);
        if (IIC_WaitAck() == 0)
        {
            printf("Found device at 0x%02X\r\n", addr);
        }
        IIC_Stop();
    }
    printf("===== I2C Scan Done =====\r\n");
}