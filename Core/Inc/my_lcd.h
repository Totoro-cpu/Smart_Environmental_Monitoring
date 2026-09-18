#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include <stdint.h>


/******************************************************************************************/
/* 显示屏控制引脚 定义 */
#define LCD_CS_PIN      GPIO_PIN_7              /* 片选引脚 定义 */
#define LCD_CS_PORT     GPIOD

#define LCD_WR_PIN      GPIO_PIN_5              /* 数据写入引脚 定义 */
#define LCD_WR_PORT     GPIOD

#define LCD_RS_PIN      GPIO_PIN_13             /* （0命令，1数据）标志引脚 定义 */
#define LCD_RS_PORT     GPIOD

#define LCD_RD_PIN      GPIO_PIN_4              /* 数据读取引脚 定义 */
#define LCD_RD_PORT     GPIOD

#define LCD_BL_PIN      GPIO_PIN_5              /* 背光引脚 定义 */
#define LCD_BL_PORT     GPIOB

/* 触摸屏控制引脚 定义 */
#define TOUCH_CS_PIN    GPIO_PIN_8              /* 片选引脚 定义 */
#define TOUCH_CS_PORT   GPIOI

#define TOUCH_PEN_PIN   GPIO_PIN_7              /* 触摸中断信号平时为高电平，按下时变为低电平，表示有触摸事件 */
#define TOUCH_PEN_PORT  GPIOH

#define TOUCH_MISO_PIN  GPIO_PIN_3              /* 数据线，触摸芯片通过它把坐标数据发送给 MCU */
#define TOUCH_MISO_PORT GPIOG

#define TOUCH_MOSI_PIN  GPIO_PIN_3              /* 数据线，MCU 通过它发送命令给触摸芯片 */
#define TOUCH_MOSI_PORT GPIOI

#define TOUCH_SCK_PIN   GPIO_PIN_6              /* SPI 时钟，同步数据传输 */
#define TOUCH_SCK_PORT  GPIOH

/*------------------ LCD 数据引脚定义 ------------------*/
/* GPIOD 上的数据引脚 */
#define LCD_D0_PIN      GPIO_PIN_14
#define LCD_D1_PIN      GPIO_PIN_15
#define LCD_D2_PIN      GPIO_PIN_0
#define LCD_D3_PIN      GPIO_PIN_1
#define LCD_D13_PIN     GPIO_PIN_8
#define LCD_D14_PIN     GPIO_PIN_9
#define LCD_D15_PIN     GPIO_PIN_10

/* GPIOE 上的数据引脚 */
#define LCD_D4_PIN      GPIO_PIN_7
#define LCD_D5_PIN      GPIO_PIN_8
#define LCD_D6_PIN      GPIO_PIN_9
#define LCD_D7_PIN      GPIO_PIN_10
#define LCD_D8_PIN      GPIO_PIN_11
#define LCD_D9_PIN      GPIO_PIN_12
#define LCD_D10_PIN     GPIO_PIN_13
#define LCD_D11_PIN     GPIO_PIN_14
#define LCD_D12_PIN     GPIO_PIN_15

/* 触摸屏有效范围定义 */
#define ADC_X_MIN  400
#define ADC_X_MAX  3650
#define ADC_Y_MIN  400
#define ADC_Y_MAX  3620

/* 为了方便操作，定义每个端口上所有数据引脚的组合掩码 */
#define LCD_D_PORTD_MASK   (LCD_D0_PIN | LCD_D1_PIN | LCD_D2_PIN | LCD_D3_PIN | \
                            LCD_D13_PIN | LCD_D14_PIN | LCD_D15_PIN)

#define LCD_D_PORTE_MASK   (LCD_D4_PIN | LCD_D5_PIN | LCD_D6_PIN | LCD_D7_PIN | \
                            LCD_D8_PIN | LCD_D9_PIN | LCD_D10_PIN | LCD_D11_PIN | LCD_D12_PIN)

/* FMC参数定义 */
#define LCD_REG     (*(volatile uint16_t *)0x60000000)
#define LCD_RAM     (*(volatile uint16_t *)0x60080000)

/* 端口时钟使能 */
#define LCD_PORTB_CLK_ENABLE()          do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)
#define LCD_PORTD_CLK_ENABLE()          do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define LCD_PORTE_CLK_ENABLE()          do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define LCD_FMC_CLK_ENABLE()            do{ __HAL_RCC_FMC_CLK_ENABLE(); }while(0)

/* ========== GPIO 模拟 SPI 宏定义 ========== */
#define TOUCH_CS_LOW()   HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_RESET)
#define TOUCH_CS_HIGH()  HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_SET)

#define TOUCH_SCK_LOW()  HAL_GPIO_WritePin(TOUCH_SCK_PORT, TOUCH_SCK_PIN, GPIO_PIN_RESET)
#define TOUCH_SCK_HIGH() HAL_GPIO_WritePin(TOUCH_SCK_PORT, TOUCH_SCK_PIN, GPIO_PIN_SET)

#define TOUCH_MOSI_LOW()  HAL_GPIO_WritePin(TOUCH_MOSI_PORT, TOUCH_MOSI_PIN, GPIO_PIN_RESET)
#define TOUCH_MOSI_HIGH() HAL_GPIO_WritePin(TOUCH_MOSI_PORT, TOUCH_MOSI_PIN, GPIO_PIN_SET)

#define TOUCH_MISO_READ() HAL_GPIO_ReadPin(TOUCH_MISO_PORT, TOUCH_MISO_PIN)

/* 外部函数 */
void lcd_init();
void write_lcd_cmd(uint16_t data);
void write_lcd_dat(uint16_t data);
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_fill_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_fill_arry(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color);
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint16_t color, uint16_t bg_color);
void lcd_show_string(uint16_t x, uint16_t y, char *str, uint16_t color, uint16_t bg_color);
void lcd_show_number(uint16_t x, uint16_t y, uint16_t num, uint16_t color, uint16_t bg_color);
void lcd_dis_chip_id(uint16_t x, uint16_t y, uint16_t color, uint16_t bg_color);
void lcd_show_chinese(uint16_t x, uint16_t y, const unsigned char *pfont, uint16_t color, uint16_t bg_color);
void touch_init();
uint16_t touch_read(uint8_t cmd);
uint8_t touch_read_calibrated(uint16_t *screen_x, uint16_t *screen_y);
void lcd_ex_st7789_reginit(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif