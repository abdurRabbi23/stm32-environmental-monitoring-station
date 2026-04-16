#include "bmp280.h"

static HAL_StatusTypeDef BMP280_ReadRegs(BMP280_HandleTypeDef *bmp, uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(bmp->hi2c, bmp->address, reg, I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef BMP280_WriteReg(BMP280_HandleTypeDef *bmp, uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(bmp->hi2c, bmp->address, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BMP280_ReadChipID(BMP280_HandleTypeDef *bmp, uint8_t *chip_id)
{
    return BMP280_ReadRegs(bmp, BMP280_REG_ID, chip_id, 1);
}

static HAL_StatusTypeDef BMP280_ReadCalibration(BMP280_HandleTypeDef *bmp)
{
    uint8_t calib[24];

    if (BMP280_ReadRegs(bmp, BMP280_REG_CALIB, calib, 24) != HAL_OK)
        return HAL_ERROR;

    bmp->calib.dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    bmp->calib.dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    bmp->calib.dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);

    bmp->calib.dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
    bmp->calib.dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
    bmp->calib.dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
    bmp->calib.dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
    bmp->calib.dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
    bmp->calib.dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
    bmp->calib.dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
    bmp->calib.dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
    bmp->calib.dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);

    return HAL_OK;
}

static int32_t BMP280_CompensateTemp(BMP280_HandleTypeDef *bmp, int32_t adc_T)
{
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)bmp->calib.dig_T1 << 1))) * ((int32_t)bmp->calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)bmp->calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)bmp->calib.dig_T1))) >> 12) *
            ((int32_t)bmp->calib.dig_T3)) >> 14;

    bmp->calib.t_fine = var1 + var2;
    T = (bmp->calib.t_fine * 5 + 128) >> 8;

    return T; // temperature in 0.01 degree C
}

static uint32_t BMP280_CompensatePressure(BMP280_HandleTypeDef *bmp, int32_t adc_P)
{
    int64_t var1, var2, p;

    var1 = ((int64_t)bmp->calib.t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)bmp->calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)bmp->calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)bmp->calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)bmp->calib.dig_P3) >> 8) + ((var1 * (int64_t)bmp->calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bmp->calib.dig_P1) >> 33;

    if (var1 == 0)
        return 0;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)bmp->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)bmp->calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)bmp->calib.dig_P7) << 4);

    return (uint32_t)p; // pressure in Q24.8 format
}

HAL_StatusTypeDef BMP280_Init(BMP280_HandleTypeDef *bmp, I2C_HandleTypeDef *hi2c, uint16_t address)
{
    uint8_t chip_id;

    bmp->hi2c = hi2c;
    bmp->address = address;

    if (BMP280_ReadChipID(bmp, &chip_id) != HAL_OK)
        return HAL_ERROR;

    if (chip_id != BMP280_CHIP_ID)
        return HAL_ERROR;

    if (BMP280_ReadCalibration(bmp) != HAL_OK)
        return HAL_ERROR;

    // Standby 1000ms, filter off
    if (BMP280_WriteReg(bmp, BMP280_REG_CONFIG, 0xA0) != HAL_OK)
        return HAL_ERROR;

    // Temp oversampling x1, Pressure oversampling x1, normal mode
    if (BMP280_WriteReg(bmp, BMP280_REG_CTRL_MEAS, 0x27) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(100);
    return HAL_OK;
}

HAL_StatusTypeDef BMP280_ReadTemperaturePressure(BMP280_HandleTypeDef *bmp, float *temperature, float *pressure_pa)
{
    uint8_t data[6];
    int32_t adc_P, adc_T;
    int32_t temp_x100;
    uint32_t press_q24_8;

    if (BMP280_ReadRegs(bmp, BMP280_REG_PRESS_MSB, data, 6) != HAL_OK)
        return HAL_ERROR;

    adc_P = (int32_t)(((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4));
    adc_T = (int32_t)(((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4));

    temp_x100 = BMP280_CompensateTemp(bmp, adc_T);
    press_q24_8 = BMP280_CompensatePressure(bmp, adc_P);

    *temperature = temp_x100 / 100.0f;
    *pressure_pa = press_q24_8 / 256.0f;

    return HAL_OK;
}
