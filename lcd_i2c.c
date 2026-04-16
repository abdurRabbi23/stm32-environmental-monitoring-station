#include "lcd_i2c.h"

static I2C_HandleTypeDef *lcd_hi2c;

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE    0x04
#define LCD_RS        0x01

static void LCD_WriteNibble(uint8_t nibble)
{
    uint8_t data = nibble | LCD_BACKLIGHT;
    HAL_I2C_Master_Transmit(lcd_hi2c, LCD_I2C_ADDR, &data, 1, HAL_MAX_DELAY);

    data |= LCD_ENABLE;
    HAL_I2C_Master_Transmit(lcd_hi2c, LCD_I2C_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);

    data &= ~LCD_ENABLE;
    HAL_I2C_Master_Transmit(lcd_hi2c, LCD_I2C_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
}

static void LCD_SendCmd(uint8_t cmd)
{
    uint8_t high = cmd & 0xF0;
    uint8_t low  = (cmd << 4) & 0xF0;

    LCD_WriteNibble(high);
    LCD_WriteNibble(low);
}

void LCD_SendChar(char ch)
{
    uint8_t high = (ch & 0xF0) | LCD_RS;
    uint8_t low  = ((ch << 4) & 0xF0) | LCD_RS;

    LCD_WriteNibble(high);
    LCD_WriteNibble(low);
}

void LCD_SendString(char *str)
{
    while (*str)
    {
        LCD_SendChar(*str++);
    }
}

void LCD_Clear(void)
{
    LCD_SendCmd(0x01);
    HAL_Delay(5);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t address;

    switch (row)
    {
        case 0: address = 0x80 + col; break;
        case 1: address = 0xC0 + col; break;
        case 2: address = 0x94 + col; break;
        case 3: address = 0xD4 + col; break;
        default: address = 0x80 + col; break;
    }

    LCD_SendCmd(address);
}

void LCD_Init(I2C_HandleTypeDef *hi2c)
{
    lcd_hi2c = hi2c;

    HAL_Delay(50);

    LCD_WriteNibble(0x30);
    HAL_Delay(5);
    LCD_WriteNibble(0x30);
    HAL_Delay(1);
    LCD_WriteNibble(0x30);
    HAL_Delay(10);
    LCD_WriteNibble(0x20); // 4-bit mode

    LCD_SendCmd(0x28); // 4-bit, 2-line, 5x8 font
    LCD_SendCmd(0x0C); // display on, cursor off
    LCD_SendCmd(0x06); // entry mode
    LCD_SendCmd(0x01); // clear display

    HAL_Delay(5);
}
