#ifndef I2C_LCD_H
#define I2C_LCD_H

#include "ch32fun.h"

#define LCD_SDA_PIN  18
#define LCD_SCL_PIN  19
#define LCD_ADDR     0x27

#define LCD_CLEAR        0x01
#define LCD_HOME         0x02
#define LCD_ENTRY_MODE   0x06
#define LCD_DISPLAY_ON   0x0C
#define LCD_FUNCTION     0x28
#define LCD_SET_DDRAM    0x80

void lcd_init(void);
void lcd_cmd(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_str(const char *str);
void lcd_set_cursor(uint8_t col, uint8_t row);

#include "i2c_lcd.c"
#endif
