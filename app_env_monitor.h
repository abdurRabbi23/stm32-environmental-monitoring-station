#ifndef APP_ENV_MONITOR_H
#define APP_ENV_MONITOR_H

#include "stm32f4xx_hal.h"

void APP_EnvMonitor_Init(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);
void APP_EnvMonitor_Task(void);

#endif
