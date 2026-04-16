#Main Modules

##bmp280.c / bmp280.h
#Handles:
BMP280 initialization
register read/write
calibration data reading
temperature and pressure compensation

##lcd_i2c.c / lcd_i2c.h
#Handles:
LCD initialization
cursor positioning
text display on 20x4 LCD

##app_env_monitor.c / app_env_monitor.h
#Handles:
periodic task execution
reading sensor data
formatting LCD display
UART transmission every 2 seconds

##UART Output Format
#Example serial output:
2000,27.53,1008.41
4000,27.55,1008.38
6000,27.54,1008.43

#Format:
time_ms,temperature_c,pressure_hpa

This format can be used directly for:

1. CSV logging
2. Python live plotting
3. Excel graphing

#How to Run the Project
Open the project in STM32CubeIDE
Check the peripheral configuration:
-I2C1
-USART2
-Build the project
-Flash the code to the STM32 Nucleo-F446RE board
-Connect the BMP280 and LCD as described above

Open a serial terminal with:
Baud rate: 115200
Data bits: 8
Parity: None
Stop bits: 1
Observe:
temperature and pressure on the LCD
data output on PC serial terminal
PC Logging and Live Graphing

The UART output can be read on the PC and stored automatically in a CSV file. It can also be plotted in real time using Python.

#Suggested Python libraries:
1. pyserial
2. matplotlib

#Future Improvements:
-Buzzer alert for threshold values
-Fan or heater control
-Data logging to SD card
-Wi-Fi / IoT monitoring
-Humidity sensing using BME280
-Improved GUI for live plotting

#Learning Outcomes
This project helped demonstrate:

STM32 peripheral initialization
I2C communication
UART communication
sensor interfacing
modular embedded software design
real-time data display and logging

##Author
MD ABDUR RABBI
Department of Mechatronics Engineering
Khulna University of Engineering & Technology, Khulna
Embedded Systems Lab Project (4-2 semester)
STM32 Nucleo-F446RE Environmental Monitoring Station
