# Solar Panel Performance Monitoring and Fault Detection Using ESP32

## Project Overview
This project is an ESP32-based solar panel monitoring system designed to observe solar panel performance and identify abnormal operating conditions.

It displays electrical and environmental parameters through an OLED display and a Wi-Fi-based web dashboard.

## Features
- Solar panel voltage monitoring
- Solar current and power display
- Panel temperature monitoring using DS18B20
- Light intensity monitoring using LDR
- Fault and performance status indication
- OLED display with touch-based page navigation
- Wi-Fi access point dashboard
- LED and buzzer alerts

## Hardware Components
- ESP32 DevKit V1
- 6 V, 3 W solar panel
- ACS712 30 A current sensor
- Voltage sensor module
- DS18B20 temperature sensor
- LDR and 10 kΩ resistor
- 1.3-inch SH1106 OLED display
- TTP223 touch sensor
- CN3065 solar charging module
- Buzzer
- Red and green LEDs
- 20 Ω, 5 W load resistor

## Software and Libraries
- Arduino IDE
- Adafruit GFX
- Adafruit SH110X
- OneWire
- DallasTemperature
- ESP32 Wi-Fi
- WebServer

## Pin Configuration

| Component | ESP32 GPIO |
|---|---|
| DS18B20 | GPIO4 |
| OLED SDA | GPIO21 |
| OLED SCL | GPIO22 |
| Buzzer | GPIO25 |
| Green LED | GPIO26 |
| Red LED | GPIO27 |
| LDR | GPIO32 |
| Touch sensor | GPIO33 |
| Voltage sensor | GPIO34 |
| ACS712 | GPIO35 |

## Wi-Fi Dashboard
1. Power on the ESP32.
2. Connect a phone or laptop to `Solar_Monitor`.
3. Password: `12345678`.
4. Open `http://192.168.4.1`.

## Fault Detection
The system checks for conditions such as:
- Solar panel disconnection
- High panel temperature
- Low sunlight
- Low performance under sufficient light

## Project Code
The Arduino source code is available in the `Code` folder.

## Important Measurement Note
The current display in the demonstration version uses a fixed 0.48 A value when the voltage exceeds the configured threshold. Consequently, displayed power is calculated from voltage × 0.48 A. This is a demonstration setting and not a live measurement of panel current.

## Project Hardware

![Complete Project](images/project_setup.jpg)

![Circuit Connections](images/circuit_connection.jpg)

![OLED Display](images/oled_display.jpg)

## Experimental Graphs

![Solar Voltage Graph](images/voltage_graph.png)

![Solar Power Graph](images/power_graph.png)

![Temperature Graph](images/temperature_graph.png)

## Applications
- Solar panel performance demonstrations
- Educational renewable energy monitoring
- Small-scale solar energy experiments
- Embedded systems and IoT learning

## Developed Using
ESP32 | Arduino IDE | Embedded Systems | Solar Energy Monitoring
