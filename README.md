# STM32 Environmental Monitoring Station

An embedded systems project built with the **STM32 Nucleo-F446RE** board to measure **temperature** and **pressure** using the **BMP280 sensor**, display the readings on a **20x4 LCD**, and send the data to a PC through **UART** for logging and live plotting.

## Project Overview

This project was developed for an embedded systems lab using **STM32CubeIDE** and **HAL libraries**. The system reads environmental data from the BMP280 sensor over **I2C**, shows the measured values on an LCD, and transmits the readings through UART every 2 seconds.

The project follows a **modular design**, where the code is divided into:
- sensor driver
- LCD driver
- application logic

This makes the system easier to understand, test, debug, and extend.

## Features

- Reads **temperature** and **pressure** from BMP280
- Uses **I2C communication**
- Displays data on a **20x4 LCD**
- Sends data to PC using **UART**
- Updates readings every **2 seconds**
- Supports **CSV serial output** for PC logging
- Can be extended for **live plotting**, **alerts**, and **threshold-based control**

## Hardware Used

- **STM32 Nucleo-F446RE**
- **BMP280 temperature and pressure sensor**
- **20x4 LCD with I2C backpack**
- Jumper wires
- USB cable for STM32 connection

## Software Used

- **STM32CubeIDE**
- **STM32 HAL libraries**
- Optional PC-side tools:
  - Serial Terminal / Tera Term / PuTTY
  - Python
  - `pyserial`
  - `matplotlib`

## System Working Principle

1. STM32 initializes GPIO, I2C, and UART peripherals.
2. BMP280 sensor is initialized over I2C.
3. The microcontroller reads temperature and pressure values every 2 seconds.
4. Measured values are displayed on the 20x4 LCD.
5. The same data is sent to the PC through UART.
6. The PC can save the received data and display it as live graphs.

## Pin Connections

### BMP280 to STM32
- **VCC** → 3.3V
- **GND** → GND
- **SDA** → PB9
- **SCL** → PB8
- **CSB** → 3.3V
- **SDO** → GND (for I2C address 0x76)

### LCD 20x4 with I2C backpack to STM32
- **VCC** → 5V
- **GND** → GND
- **SDA** → PB9
- **SCL** → PB8

## Project Structure

```text
Core/
├── Inc/
│   ├── app_env_monitor.h
│   ├── bmp280.h
│   └── lcd_i2c.h
│
├── Src/
│   ├── app_env_monitor.c
│   ├── bmp280.c
│   ├── lcd_i2c.c
│   └── main.c
