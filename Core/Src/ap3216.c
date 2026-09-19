#include "ap3216.h"
#include "myiic.h"

/* 写一个字节到指定寄存器 */
static uint8_t ap3216_write_reg(uint8_t reg, uint8_t data)
{
    return iic_write_reg(AP3216_ADDR, reg, data);
}

/* 读一个字节从指定寄存器（每次只读 1 字节） */
static uint8_t ap3216_read_reg(uint8_t reg, uint8_t *pdata)
{
    return iic_read_reg(AP3216_ADDR, reg, pdata, 1);
}

/* 初始化 */
uint8_t ap3216_init(void)
{
    uint8_t temp;

    if (ap3216_write_reg(0x00, 0x04)) return 1;   /* 软复位 */
    HAL_Delay(50);
    if (ap3216_write_reg(0x00, 0x03)) return 1;   /* 开启 ALS + PS + IR */
    HAL_Delay(10);

    ap3216_read_reg(0x00, &temp);
    return (temp == 0x03) ? 0 : 1;
}

/* 读数据 */
uint8_t ap3216_read_data(uint16_t *ir, uint16_t *als, uint16_t *ps)
{
    uint8_t buf[6];

    /* 逐个寄存器读，每次一个完整的 I2C 事务 */
    for (uint8_t i = 0; i < 6; i++)
    {
        if (ap3216_read_reg(0x0A + i, &buf[i])) return 1;
    }

    /* IR：10 位，bit7 为无效标志 */
    if (buf[0] & 0x80)
        *ir = 0;
    else
        *ir = (uint16_t)(buf[1] << 2) | (buf[0] & 0x03);

    /* ALS：16 位直接拼接 */
    *als = (uint16_t)(buf[3] << 8) | buf[2];

    /* PS：10 位，bit6 为无效标志 */
    if (buf[4] & 0x40)
        *ps = 0;
    else
        *ps = (uint16_t)(buf[5] << 2) | (buf[4] & 0x0F);

    return 0;
}