#ifndef BMP280_H
#define BMP280_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define BMP280_I2C_ADDR_76   (0x76 << 1)
#define BMP280_I2C_ADDR_77   (0x77 << 1)

#define BMP280_REG_CALIB     0x88
#define BMP280_REG_ID        0xD0
#define BMP280_REG_RESET     0xE0
#define BMP280_REG_STATUS    0xF3
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG    0xF5
#define BMP280_REG_PRESS_MSB 0xF7

#define BMP280_CHIP_ID       0x58

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    int32_t t_fine;
} BMP280_CalibData;

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    uint16_t address;
    BMP280_CalibData calib;
} BMP280_HandleTypeDef;

HAL_StatusTypeDef BMP280_Init(BMP280_HandleTypeDef *bmp, I2C_HandleTypeDef *hi2c, uint16_t address);
HAL_StatusTypeDef BMP280_ReadChipID(BMP280_HandleTypeDef *bmp, uint8_t *chip_id);
HAL_StatusTypeDef BMP280_ReadTemperaturePressure(BMP280_HandleTypeDef *bmp, float *temperature, float *pressure_pa);

#endif
