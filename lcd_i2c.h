#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define LCD_I2C_ADDR   (0x3F << 1)   // change to (0x3F << 1) if needed

void LCD_Init(I2C_HandleTypeDef *hi2c);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_SendChar(char ch);
void LCD_SendString(char *str);

#endif
