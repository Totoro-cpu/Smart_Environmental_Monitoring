#include "my_lcd.h"
#include "lcdfont.h"
#include "main.h"
#include <stdio.h>


/* 微秒级延时（180MHz 主频忙等待，约 1us） */
void delay_us(uint32_t us)
{
    while (us--)
    {
        for (volatile uint32_t i = 0; i < 45; i++);
    }
}

/* 毫秒级延时，直接用 HAL 库 */
void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}


/* lcd触摸屏初始化函数 */
void touch_init()
{
    GPIO_InitTypeDef gpio_init = {0};
    __HAL_RCC_GPIOG_CLK_ENABLE();                       /* MISO */
    __HAL_RCC_GPIOH_CLK_ENABLE();                       /* PEN、SCK */
    __HAL_RCC_GPIOI_CLK_ENABLE();                       /* CS、MOSI */
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init.Pin = TOUCH_CS_PIN;
    HAL_GPIO_Init(TOUCH_CS_PORT, &gpio_init);           /* CS输出模式，默认高电平 */
    TOUCH_CS_HIGH();
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Pin = TOUCH_SCK_PIN;
    HAL_GPIO_Init(TOUCH_SCK_PORT, &gpio_init);          /* CLK输出模式，默认低电平 */
    TOUCH_SCK_LOW();
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Pin = TOUCH_PEN_PIN;
    HAL_GPIO_Init(TOUCH_PEN_PORT, &gpio_init);          /* PEN输入模式，默认未按下所以上拉，按下输出低电平 */
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Pin = TOUCH_MOSI_PIN;
    HAL_GPIO_Init(TOUCH_MOSI_PORT, &gpio_init);         /* MOSI输出模式，MCU->XPT2046，默认低电平 */
    TOUCH_MOSI_LOW();
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Pin = TOUCH_MISO_PIN;
    HAL_GPIO_Init(TOUCH_MISO_PORT, &gpio_init);         /* MISO输入模式，XPT2046->MCU */
}

/* 触屏读取函数 */
/* ========== 软件 SPI发送一字节 函数 ========== */
static void tp_write_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)                     /* 逐位处理一个字节的数据 */
    {
        if (data & 0x80)                                /* XPT2046芯片按照最高位优先 */
        {
            TOUCH_MOSI_HIGH();                          /* 如果该高位置是1，就输出到MOSI线上，表现为引脚电平拉高输出生效 */
        }
        else
        {
            TOUCH_MOSI_LOW();                           /* 如果该高位置是0，就不输出到MOSI线上，表现为引脚电平拉低输出失效 */
        }
        data <<= 1;                                     /* 处理完一位依次处理下一位所以直接左移一位即可 */
        /* XPT2046 在上升沿锁存 MOSI 上的数据，通过SCK先拉低再拉高来产生上升沿 */
        TOUCH_SCK_LOW();                                /* 片内时钟拉低 */
        delay_us(1);                                    /* 片内时钟拉低后保持tCL时间，芯片手册有，芯片数据手册200ns，保守设置1us */
        TOUCH_SCK_HIGH();                               /* 片内时钟拉高 */
        delay_us(1);                                    /* 片内时钟拉低后保持tCH时间，芯片手册有，芯片数据手册200ns，保守设置1us */
    }
}
/* ========== 软件 SPI 读取数据 函数 ========== */
static uint16_t tp_read_ad(uint8_t cmd)
{
    uint16_t data = 0;                                  /* 接收读取的数据 */
    TOUCH_CS_LOW();                                     /* 拉低片选，选中芯片 */
    tp_write_byte(cmd);                                 /* 调用发送字节函数给XPT2046发送命令 */
    delay_us(6);                                        /* 等待XPT2046处理 */
    /* XPT2046处理完毕后会出现一个忙等待标志，需要发送一个额外的 SCK 脉冲来清除忙等待标志 */
    TOUCH_SCK_LOW();                                    /* 片内时钟拉低开始创造脉冲 */
    delay_us(1);                                        /* 片内时钟拉低后保持tCL时间，芯片手册有，芯片数据手册200ns，保守设置1us */
    TOUCH_SCK_HIGH();                                   /* 片内时钟拉高 */
    delay_us(1);                                        /* 片内时钟拉低后保持tCH时间，芯片手册有，芯片数据手册200ns，保守设置1us */
    for (uint8_t i = 0; i < 16; i++)                    /* 接收数据按一位一位处理 */
    {
        data <<= 1;                                     /* 循环开始前，先把最低位挪出来一个空位置 */
        TOUCH_SCK_LOW();                                /* 片内时钟拉低开始创造脉冲 */
        delay_us(1);                                    /* 片内时钟拉低后保持tCL时间，芯片手册有，芯片数据手册200ns，保守设置1us */
        if (TOUCH_MISO_READ())                          /* 判断MCU输入引脚电平，看是否有数据通过XPT2046输入过来 */
        {
            data |= 0x01;                               /* 如果有数据输入进来，就把这个最低位置1，通过外部循环让数据不断左移 */
        }
        TOUCH_SCK_HIGH();                               /* 片内时钟拉低开始创造脉冲 */
        delay_us(1);                                    /* 片内时钟拉低后保持tCH时间，芯片手册有，芯片数据手册200ns，保守设置1us */
    }
    return data >> 4;                                   /* 现在data的高12位就是需要的数据，低4位都是0，所以把高12位右移4位便是最后的数据，可直接返回 */
}

/* 触屏读取封装函数 */
uint16_t touch_read(uint8_t cmd)
{
    return tp_read_ad(cmd);
}

/* 触屏校验函数 */
uint8_t touch_read_calibrated(uint16_t *screen_x, uint16_t *screen_y)
{
    uint16_t adc_x = 0;
    uint16_t adc_y = 0;
    // 1. 读取原始 ADC 值
    adc_x = touch_read(0xD0);
    adc_y = touch_read(0x90);
    // 2. 判断是否有效
    if (adc_x > 100 && adc_x < 4000 && adc_y > 100 && adc_y < 4000)
    {
        // 3. 转换成屏幕坐标
        *screen_x = (adc_x - ADC_X_MIN) * 240 / (ADC_X_MAX - ADC_X_MIN);
        *screen_y = (adc_y - ADC_Y_MIN) * 320 / (ADC_Y_MAX - ADC_Y_MIN);
        // 4. 返回结果
        return 1;
    }
    else
    {
        return 0;
    }
}

/* lcd初始化函数（使用FMC加速） */
void lcd_init()
{
    HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_RESET); /* 先关背光 */
    lcd_ex_st7789_reginit();                 /* 发送 ST7789 初始化序列 */
    HAL_GPIO_WritePin(LCD_BL_PORT, LCD_BL_PIN, GPIO_PIN_SET);   /* 再开背光 */
}

/* lcd命令发送函数（使用FMC加速） */
void write_lcd_cmd(uint16_t data)
{
    LCD_REG = data;
}

/* lcd数据发送函数（使用FMC加速） */
void write_lcd_dat(uint16_t data)
{
    LCD_RAM = data;
}

/* 画点函数 */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    // 1. 发送命令 0x2A（列地址）
    write_lcd_cmd(0x2A);
    //    发送起始列 = x
    write_lcd_dat(x >> 8);
    write_lcd_dat(x & 0xFF);
    //    发送结束列 = x（同一个点，所以起始和结束一样）
    write_lcd_dat(x >> 8);
    write_lcd_dat(x & 0xFF);

    // 2. 发送命令 0x2B（行地址）
    write_lcd_cmd(0x2B);
    //    发送起始行 = y
    write_lcd_dat(y >> 8);
    write_lcd_dat(y & 0xFF);
    //    发送结束行 = y
    write_lcd_dat(y >> 8);
    write_lcd_dat(y & 0xFF);
    // 3. 发送命令 0x2C（写显存）
    write_lcd_cmd(0x2C);
    //    发送颜色值 color
    write_lcd_dat(color);
}

/* 指定区域填充颜色函数,左上角坐标(x1,y1)，右下角坐标(x2,y2) */
void lcd_fill_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t length = 0;                            /* 定义区域长 */
    uint16_t width = 0;                             /* 定义区域宽 */

    length = y2 - y1 + 1;                           /* 区域长赋值 */
    width = x2 - x1 + 1;                            /* 区域宽赋值 */

    write_lcd_cmd(0x2A);                            /* 命令开始传左上角横坐标 */
    write_lcd_dat(x1 >> 8);                         /* 传左上角横坐标高8位 */
    write_lcd_dat(x1 & 0xFF);                       /* 传左上角横坐标低8位 */
    write_lcd_dat(x2 >> 8);                         /* 传右下角横坐标高8位 */
    write_lcd_dat(x2 & 0xFF);                       /* 传右下角横坐标低8位 */
    write_lcd_cmd(0x2B);                            /* 命令开始传左上角纵坐标 */
    write_lcd_dat(y1 >> 8);                         /* 传左上角纵坐标低8位 */
    write_lcd_dat(y1 & 0xFF);                       /* 传左上角纵坐标低8位 */
    write_lcd_dat(y2 >> 8);                         /* 传右下角纵坐标低8位 */
    write_lcd_dat(y2 & 0xFF);                       /* 传右下角纵坐标低8位 */
    write_lcd_cmd(0x2C);                            /* 命令开始传颜色数据 */
    for (uint32_t i = 0; i < (length * width); i++)
    {
        write_lcd_dat(color);
    }
}

/* 指定区域填充颜色数组函数,左上角坐标(x1,y1)，右下角坐标(x2,y2) */
void lcd_fill_arry(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color)
{
    uint16_t length = 0;                            /* 定义区域长 */
    uint16_t width = 0;                             /* 定义区域宽 */

    length = y2 - y1 + 1;                           /* 区域长赋值 */
    width = x2 - x1 + 1;                            /* 区域宽赋值 */

    write_lcd_cmd(0x2A);                            /* 命令开始传左上角横坐标 */
    write_lcd_dat(x1 >> 8);                         /* 传左上角横坐标高8位 */
    write_lcd_dat(x1 & 0xFF);                       /* 传左上角横坐标低8位 */
    write_lcd_dat(x2 >> 8);                         /* 传右下角横坐标高8位 */
    write_lcd_dat(x2 & 0xFF);                       /* 传右下角横坐标低8位 */
    write_lcd_cmd(0x2B);                            /* 命令开始传左上角纵坐标 */
    write_lcd_dat(y1 >> 8);                         /* 传左上角纵坐标低8位 */
    write_lcd_dat(y1 & 0xFF);                       /* 传左上角纵坐标低8位 */
    write_lcd_dat(y2 >> 8);                         /* 传右下角纵坐标低8位 */
    write_lcd_dat(y2 & 0xFF);                       /* 传右下角纵坐标低8位 */
    write_lcd_cmd(0x2C);                            /* 命令开始传颜色数据 */
    for (uint32_t i = 0; i < (length * width); i++)
    {
        write_lcd_dat(color[i]);
    }
}

/* 指定位置显示一个字符 */
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint16_t color, uint16_t bg_color)
{
    uint16_t temp = 0;                                  /* 提取某一列的2个字节数据 */
    uint8_t row, col;                                   /* 定义行号和列号 */
    const unsigned char *pfont = NULL;                  /* 字库获取指针，char只有一个字节 */
    /* 1. 根据 chr 计算出在字库数组中的偏移位置（提示：chr - ' '） */
    uint8_t index = chr - ' ';                          /* 获取要显示字符在字库的位置，空格是第一个可屏显字符，拿目标字符减去空格字符即可得到 */
    /* 2. 用指针指向 asc2_1608[偏移] 这个数组 */
    pfont = asc2_1608[index];                           /* 找到字符点阵 */
    /* 3. 外层循环：遍历 8 列 (t = 0; t < 8; t++)，因为有16行8列 */
    for (col = 0; col < 8; col++)
    {
        /*    4. 取这一列的2个字节数据 */
        temp = ((uint16_t)pfont[col * 2] << 8) | (uint16_t)pfont[col * 2 + 1];/* 按列把两字节拼接成一个16位 */
        for (row = 0; row < 16; row++) /*    5. 然后按照拼出来的每一列逐位处理，只看最高位 */
        {
            if (temp & 0x8000) /*    6. 相当于在最高位放了个窗口，依次判断最高位、次高位、次次高位.....为0还是1 */
            {
                lcd_draw_point(x + col, y + row, color);
            }
            else
            {
                lcd_draw_point(x + col, y + row, bg_color);
            }
            temp <<= 1;
        }
    }
}

/* 指定位置显示两个中文字符‘你好’ */
void lcd_show_chinese(uint16_t x, uint16_t y, const unsigned char *pfont, uint16_t color, uint16_t bg_color)
{
    uint16_t temp = 0;                                  /* 提取某一列的2个字节数据 */
    uint8_t row, col;                                   /* 定义行号和列号 */
    /* 3. 外层循环：遍历 8 列 (t = 0; t < 16; t++)，因为有16行16列 */
    for (col = 0; col < 16; col++)
    {
        /*    4. 取这一列的2个字节数据 */
        temp = ((uint16_t)pfont[col * 2] << 8) | (uint16_t)pfont[col * 2 + 1];/* 按列把两字节拼接成一个16位 */
        for (row = 0; row < 16; row++) /*    5. 然后按照拼出来的每一列逐位处理，只看最高位 */
        {
            if (temp & 0x8000) /*    6. 相当于在最高位放了个窗口，依次判断最高位、次高位、次次高位.....为0还是1 */
            {
                lcd_draw_point(x + col, y + row, color);
            }
            else
            {
                lcd_draw_point(x + col, y + row, bg_color);
            }
            temp <<= 1;
        }
    }
}

/* 指定位置显示一个字符串 */
void lcd_show_string(uint16_t x, uint16_t y, char *str, uint16_t color, uint16_t bg_color)
{
    /* 1. 只要字符串还没结束（即 *str != '\0'） */
    /* 2. 就在当前 (x, y) 位置调用 lcd_show_char 显示当前字符 */
    /* 3. 将 x 增加 8（因为字符宽度是8） */
    /* 4. str++（指针移到下一个字符） */
    //uint16_t index = 0;
    while (*str != '\0')
    {
        if (x > 240 - 8) // 如果剩余宽度不够放一个字符
        {
            x = 0;// 换行到最左边
            y += 16;// 移到下一行（高度16）
            if (y > 320 - 16) // 超出屏幕底部则截断
            {
                break;
            }
        }
        lcd_show_char(x, y, *str, color, bg_color);
        x += 8;
        str++;
    }
}

/* 指定位置显示一个数字 */
void lcd_show_number(uint16_t x, uint16_t y, uint16_t num, uint16_t color, uint16_t bg_color)
{
    char buf[6];
    uint8_t i = 0, j = 0;
    char temp;

    if (num == 0) //处理0这个特殊值
    {
        lcd_show_char(x, y, '0', color, bg_color);
        return;
    }
    /* 1.提取出数字的每一位 对于10取模得到从低位开始的每一个位，处理完一位后原值再除以10，处理次低位 */
    while (num > 0)
    {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    buf[i] = '\0';//最后添加停止符
    /* 2.此时buf数组存放的是每一位数的字符形式，不过是反序的，我们输出需要从高位往低位输出，所以要做反序处理 */
    /* 上面对数组赋值的时候操作了i变量自增，i也就是数组的数字字符个数 */
    for (j = 0; j < (i / 2); j++)
    {
        temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }
    lcd_show_string(x, y, buf, color, bg_color);
}

/* 从 LCD 读取一个 16 位数据（RS=1，RD 产生下降沿） *  */
uint16_t lcd_read_data(void)
{
    return LCD_RAM;
}

/* 在指定位置显示给定颜色的lcd的控制器id */
void lcd_dis_chip_id(uint16_t x, uint16_t y, uint16_t color, uint16_t bg_color)
{
    uint8_t d1 = 0, d2 = 0, d3 = 0;
    char id_str[20];
    write_lcd_cmd(0x04);
    lcd_read_data();
    d1 = lcd_read_data();
    d2 = lcd_read_data();
    d3 = lcd_read_data();
    if (d2 == 0x85 && d3 == 0x52)
    {
        d2 = 0x77;
        d3 = 0x89;
        sprintf(id_str, "LCD ID: ST%02X%02X", d2, d3);
    }
    lcd_show_string(x, y, id_str, color, bg_color);
}
