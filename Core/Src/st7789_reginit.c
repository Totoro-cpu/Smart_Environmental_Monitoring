#include "my_lcd.h"

void lcd_ex_st7789_reginit(void)
{
    write_lcd_cmd(0x11);

    delay_ms(120); 

    write_lcd_cmd(0x36);
    write_lcd_dat(0x08);

    write_lcd_cmd(0x3A);
    write_lcd_dat(0x05);

    write_lcd_cmd(0xB2);
    write_lcd_dat(0x0C);
    write_lcd_dat(0x0C);
    write_lcd_dat(0x00);
    write_lcd_dat(0x33);
    write_lcd_dat(0x33);

    write_lcd_cmd(0xB7);
    write_lcd_dat(0x35);

    write_lcd_cmd(0xBB); /* vcom */
    write_lcd_dat(0x32);  /* 30 */

    write_lcd_cmd(0xC0);
    write_lcd_dat(0x0C);

    write_lcd_cmd(0xC2);
    write_lcd_dat(0x01);

    write_lcd_cmd(0xC3); /* vrh */
    write_lcd_dat(0x10);  /* 17 0D */

    write_lcd_cmd(0xC4); /* vdv */
    write_lcd_dat(0x20);  /* 20 */

    write_lcd_cmd(0xC6);
    write_lcd_dat(0x0f);

    write_lcd_cmd(0xD0);
    write_lcd_dat(0xA4); 
    write_lcd_dat(0xA1); 

    write_lcd_cmd(0xE0); /* Set Gamma  */
    write_lcd_dat(0xd0);
    write_lcd_dat(0x00);
    write_lcd_dat(0x02);
    write_lcd_dat(0x07);
    write_lcd_dat(0x0a);
    write_lcd_dat(0x28);
    write_lcd_dat(0x32);
    write_lcd_dat(0x44);
    write_lcd_dat(0x42);
    write_lcd_dat(0x06);
    write_lcd_dat(0x0e);
    write_lcd_dat(0x12);
    write_lcd_dat(0x14);
    write_lcd_dat(0x17);


    write_lcd_cmd(0xE1);  /* Set Gamma */
    write_lcd_dat(0xd0);
    write_lcd_dat(0x00);
    write_lcd_dat(0x02);
    write_lcd_dat(0x07);
    write_lcd_dat(0x0a);
    write_lcd_dat(0x28);
    write_lcd_dat(0x31);
    write_lcd_dat(0x54);
    write_lcd_dat(0x47);
    write_lcd_dat(0x0e);
    write_lcd_dat(0x1c);
    write_lcd_dat(0x17);
    write_lcd_dat(0x1b); 
    write_lcd_dat(0x1e);


    write_lcd_cmd(0x2A);
    write_lcd_dat(0x00);
    write_lcd_dat(0x00);
    write_lcd_dat(0x00);
    write_lcd_dat(0xef);

    write_lcd_cmd(0x2B);
    write_lcd_dat(0x00);
    write_lcd_dat(0x00);
    write_lcd_dat(0x01);
    write_lcd_dat(0x3f);

    write_lcd_cmd(0x29); /* display on */
}
