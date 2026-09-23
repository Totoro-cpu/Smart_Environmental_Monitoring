#include "aht20.h"
#include "myiic.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

#define AHT20_ADDRESS 0x38

/* 上电后给充足稳定时间 */
#define AHT20_STARTUP_DELAY   200
#define AHT20_CALIBRATION_DELAY 200
#define AHT20_MEASURE_DELAY   100

static float last_temp = 0.0f;
static float last_humi = 0.0f;

/* 发送 0x71 状态命令 */
static void aht20_send_status_cmd(void)
{
    IIC_Start();
    IIC_SendByte(AHT20_ADDRESS << 1);   /* 写地址 0x70 */
    IIC_WaitAck();
    IIC_SendByte(0x71);                 /* 命令 0x71 */
    IIC_WaitAck();
    IIC_Stop();
}

/* 读 1 字节状态 */
static uint8_t aht20_read_status(void)
{
    uint8_t status;

    IIC_Start();
    IIC_SendByte((AHT20_ADDRESS << 1) | 1);   /* 读地址 0x71 */
    IIC_WaitAck();
    status = iic_read_byte(0);                /* NACK 结束 */
    IIC_Stop();

    return status;
}

/* 初始化 */
uint8_t aht20_init(void)
{
    HAL_Delay(AHT20_STARTUP_DELAY);

    /* 方式：先发 0x71 命令，再读状态 */
    aht20_send_status_cmd();
    HAL_Delay(20);

    uint8_t status = aht20_read_status();
    printf("AHT20 status = 0x%02X\r\n", status);

    /* 如果 bit3 = 0（未校准），发初始化命令 */
    if ((status & 0x08) == 0)
    {
        IIC_Start();
        IIC_SendByte(AHT20_ADDRESS << 1);
        IIC_WaitAck();
        IIC_SendByte(0xBE); IIC_WaitAck();
        IIC_SendByte(0x08); IIC_WaitAck();
        IIC_SendByte(0x00); IIC_WaitAck();
        IIC_Stop();

        HAL_Delay(AHT20_CALIBRATION_DELAY);

        aht20_send_status_cmd();
        HAL_Delay(20);
        status = aht20_read_status();
        printf("AHT20 init status = 0x%02X\r\n", status);
    }

    return (status & 0x08) ? 0 : 1;
}

/* 读温湿度 */
uint8_t aht20_read_data(float *temp, float *humi)
{
    /* 1. 触发测量 */
    IIC_Start();
    IIC_SendByte(AHT20_ADDRESS << 1);
    IIC_WaitAck();
    IIC_SendByte(0xAC); IIC_WaitAck();
    IIC_SendByte(0x33); IIC_WaitAck();
    IIC_SendByte(0x00); IIC_WaitAck();
    IIC_Stop();

    HAL_Delay(AHT20_MEASURE_DELAY);

    /* 2. 直接读 7 字节（状态 + 6 数据 + CRC） */
    uint8_t buf[7];

    IIC_Start();
    IIC_SendByte((AHT20_ADDRESS << 1) | 1);
    IIC_WaitAck();

    buf[0] = iic_read_byte(1);
    buf[1] = iic_read_byte(1);
    buf[2] = iic_read_byte(1);
    buf[3] = iic_read_byte(1);
    buf[4] = iic_read_byte(1);
    buf[5] = iic_read_byte(1);
    buf[6] = iic_read_byte(0);   /* 最后一字节 NACK */

    IIC_Stop();

    printf("AHT20 raw: %02X %02X %02X %02X %02X %02X %02X\r\n",
           buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);

    /* 3. 如果忙，用上次值 */
    if (buf[0] & 0x80)
    {
        *temp = last_temp;
        *humi = last_humi;
        return 1;
    }

    /* 4. 解析 */
    uint32_t raw_humi = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | (buf[3] >> 4);
    uint32_t raw_temp = ((uint32_t)(buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | buf[5];

    last_humi = (float)raw_humi * 100.0f / 1048576.0f;
    last_temp = (float)raw_temp * 200.0f / 1048576.0f - 50.0f;

    *humi = last_humi;
    *temp = last_temp;

    return 0;
}