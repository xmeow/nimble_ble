# ESP-IDF NimBLE Nordic UART Service (NUS) Peripheral Demo

This project demonstrates how to implement a BLE peripheral using the ESP-IDF framework and the NimBLE library, providing the Nordic UART Service (NUS).

## Features

- Acts as a BLE peripheral with a custom NUS service
- Provides NUS RX and TX characteristics
- Handles read and write operations on the NUS characteristics
- Sends a response message when the NUS TX characteristic is read
- Logs the received data when the NUS RX characteristic is written

## Requirements

ESP32 development board
ESP-IDF v4.x

## Setup

- Recommends vscode esp-idf integration, just open the cloned folder in vscode.
- Select the target mcu (tested on esp32s3)
- Then click the build/flash/monitor button.


## Usage

After flashing the firmware, the ESP32 will start advertising the Nordic UART Service (NUS).

Use a BLE scanner app, such as "nRF Connect" or "LightBlue", to discover the ESP32's BLE device.

Connect to the ESP32's BLE device and discover its services and characteristics.

Write data to the NUS RX characteristic (6E400002-B5A3-F393-E0A9-E50E24DCCA9E). The ESP32 will log the received data.

Read data from the NUS TX characteristic (6E400003-B5A3-F393-E0A9-E50E24DCCA9E). The ESP32 will respond with a message: "Hello from ESP32!"