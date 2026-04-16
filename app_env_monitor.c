#include "app_env_monitor.h"
#include "bmp280.h"
#include "lcd_i2c.h"
#include <stdio.h>
#include <string.h>

static I2C_HandleTypeDef *app_hi2c;
static UART_HandleTypeDef *app_huart;
static BMP280_HandleTypeDef bmp280;

static uint32_t last_update = 0;
static uint8_t bmp_ready = 0;

static void UART_SendString(const char *text)
{
    HAL_UART_Transmit(app_huart, (uint8_t *)text, strlen(text), HAL_MAX_DELAY);
}

void APP_EnvMonitor_Init(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    char msg[64];
    uint8_t chip_id = 0;

    app_hi2c = hi2c;
    app_huart = huart;

    LCD_Init(app_hi2c);
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_SendString("Env Monitor Start");

    if (BMP280_Init(&bmp280, app_hi2c, BMP280_I2C_ADDR_76) == HAL_OK)
    {
        bmp_ready = 1;
    }
    else if (BMP280_Init(&bmp280, app_hi2c, BMP280_I2C_ADDR_77) == HAL_OK)
    {
        bmp_ready = 1;
    }
    else
    {
        bmp_ready = 0;
    }

    if (bmp_ready)
    {
        BMP280_ReadChipID(&bmp280, &chip_id);

        snprintf(msg, sizeof(msg), "BMP280 Ready, ID=0x%02X\r\n", chip_id);
        UART_SendString(msg);

        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_SendString("BMP280 Ready");
        HAL_Delay(1000);
        LCD_Clear();
    }
    else
    {
        UART_SendString("BMP280 Init Failed\r\n");
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_SendString("BMP280 Failed");
    }
}

void APP_EnvMonitor_Task(void)
{
    if ((HAL_GetTick() - last_update) >= 2000)
    {
        float temperature;
        float pressure_pa;
        float pressure_hpa;
        char lcd_line1[21];
        char lcd_line2[21];
        char uart_msg[100];

        last_update = HAL_GetTick();

        if (!bmp_ready)
        {
            UART_SendString("Sensor not ready\r\n");
            return;
        }

        if (BMP280_ReadTemperaturePressure(&bmp280, &temperature, &pressure_pa) == HAL_OK)
        {
            pressure_hpa = pressure_pa / 100.0f;

            snprintf(lcd_line1, sizeof(lcd_line1), "Temp: %.2f C", temperature);
            snprintf(lcd_line2, sizeof(lcd_line2), "Pres: %.2f hPa", pressure_hpa);

            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_SendString("Environmental Mon.");
            LCD_SetCursor(1, 0);
            LCD_SendString(lcd_line1);
            LCD_SetCursor(2, 0);
            LCD_SendString(lcd_line2);
            LCD_SetCursor(3, 0);
            LCD_SendString("UART update every 2s");

            snprintf(uart_msg, sizeof(uart_msg),
                     "%lu,%.2f,%.2f\r\n",
                     HAL_GetTick(),
                     temperature,
                     pressure_hpa);

            UART_SendString(uart_msg);
        }
        else
        {
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_SendString("Sensor Read Error");
            UART_SendString("BMP280 read error\r\n");
        }
    }
}
