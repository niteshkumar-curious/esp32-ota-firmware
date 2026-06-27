# ESP32 OTA Firmware Management, Rollback and IoT Device Dashboard

## Overview

This project demonstrates a secure, modular Over-The-Air (OTA) firmware update system for the ESP32 using **ESP-IDF**. It enables remote firmware updates without requiring physical access to the device while providing firmware version management, rollback capability, remote monitoring, and an IoT dashboard.

The project uses **MQTT** for bidirectional communication, **HTTPS** for secure firmware download, **FreeRTOS** for multitasking, and a **Node.js + React** web dashboard for device management.

---

# Features

* Secure OTA firmware updates using ESP-IDF
* HTTPS firmware download with TLS
* MQTT-based remote device communication
* Modular firmware architecture
* Firmware version management
* Rollback support
* Automatic firmware validation after OTA
* Real-time OTA progress reporting
* Device online/offline monitoring
* LED remote control through MQTT
* Web dashboard for device management
* NTC Thermistor temperature monitoring (Firmware v1.0.4)
* GitHub-hosted firmware binaries

---

# Firmware Versions

| Version    | Features                                                                   |
| ---------- | --------------------------------------------------------------------------- |
| **v1.0.1** | Initial OTA implementation with MQTT communication                          |
| **v1.0.2** | Improved OTA workflow and led blink and manual push button using interrupt  |
| **v1.0.3** | Firmware validation and rollback enhancements                               |
| **v1.0.4** | Added NTC Thermistor temperature monitoring, ADC averaging and PWM control  |

---

# System Architecture

```text
                GitHub Firmware Repository
                         │
                     HTTPS Download
                         │
               ┌──────── ESP32 ────────┐
               │                       │
         FreeRTOS Tasks           OTA Manager
               │                       │
          ADC / PWM             Firmware Validation
               │                       │
               └──── MQTT over TLS ────┘
                         │
                  HiveMQ Cloud Broker
                         │
          ┌──────────────┴──────────────┐
          │                             │
      Node.js Backend             React Dashboard
```

---

# Dashboard Features

* Device online/offline indication
* Current firmware version
* OTA firmware selection
* OTA update progress bar
* Rollback button
* Firmware result notification
* Temperature display (available only in Firmware v1.0.4)

---

# Project Structure

```text
main/
│
├── main.c
├── task_handling.c
├── task_handling.h
├── mqtt_ota_connect.c
├── mqtt_ota_connect.h
├── wifi.c
├── wifi.h
├── adc_cal_us.c
├── adc_cal_us.h
├── gpio_user_config.c
├── gpio_user_config.h
├── common_header.h
├── mydata.h
└── CMakeLists.txt

firmware_ota/
├── fw_v101.bin
├── fw_v102.bin
├── fw_v103.bin
└── fw_v104.bin

front-end/

README.md
```

---

# FreeRTOS Tasks

| Task         | Function                                                  |
| ------------ | --------------------------------------------------------- |
| sensor_task  | Reads ADC, calculates temperature, updates PWM            |
| status_task  | Publishes device status, firmware version and temperature |
| ota_led_task | Indicates OTA progress using LED                          |

---

# MQTT Topics

| Topic                      | Description               |
| -------------------------- | ------------------------- |
| esp32/device1/status       | Device online status      |
| esp32/device1/led          | LED control               |
| esp32/device1/ota          | OTA update request        |
| esp32/device1/ota_progress | OTA progress              |
| esp32/device1/ota_result   | OTA result                |
| esp32/device1/version      | Running firmware version  |
| esp32/device1/temperature  | Temperature data (v1.0.4) |

---

# Technologies Used

## Hardware

* ESP32 Development Board
* 10k NTC Thermistor
* Fixed resistor (Voltage Divider)

## Software

* ESP-IDF
* Embedded C
* FreeRTOS
* MQTT
* HTTPS
* Node.js
* Express.js
* React.js
* Axios
* HiveMQ Cloud
* GitHub
* Netlify
* Render

---

# OTA Workflow

1. ESP32 connects to Wi-Fi.
2. MQTT connection is established.
3. Device publishes its firmware version.
4. Dashboard displays current firmware.
5. User selects a firmware version.
6. Backend publishes OTA request.
7. ESP32 downloads firmware via HTTPS.
8. OTA progress is reported to the dashboard.
9. Device reboots.
10. Firmware performs self-validation.
11. If validation succeeds, firmware is marked valid.
12. Otherwise, automatic rollback is performed.

---

# Temperature Monitoring (Firmware v1.0.4)

Firmware v1.0.4 introduces temperature monitoring using a 10k NTC thermistor.

Features include:

* ADC calibration
* 5-sample averaging
* Beta parameter temperature calculation
* PWM duty cycle proportional to ADC value
* Temperature published through MQTT
* Dashboard displays temperature only when firmware v1.0.4 is running

---

# Key Concepts Demonstrated

* ESP-IDF OTA Updates
* FreeRTOS Multitasking
* MQTT Publish/Subscribe
* HTTPS Secure Communication
* Firmware Rollback
* Firmware Validation
* ADC Calibration
* Thermistor Temperature Measurement
* PWM Control
* Modular Embedded Software Design
* REST API Development
* React Dashboard Development

---

# Learning Outcomes

This project provided practical experience in:

* ESP32 firmware development
* Embedded C programming
* ESP-IDF component-based project structure
* FreeRTOS task scheduling
* MQTT communication
* HTTPS OTA firmware updates
* Firmware version management
* Rollback strategies
* ADC calibration techniques
* Thermistor temperature sensing
* REST API development using Express.js
* React dashboard development
* Git and GitHub version control
* Deploying backend on Render
* Deploying frontend on Netlify

---

# Future Improvements

* Secure Boot
* Flash Encryption
* Digital Signature Verification
* Delta OTA Updates
* Multiple Device Management
* Device Authentication
* Sensor Data Logging
* Historical Temperature Graphs
* OTA Scheduling
* Mobile Dashboard
* ESP32 CAN Bus Integration

---

# Author

**H Nitesh Kumar**

B.E. Electronics and Communication Engineering

GitHub:
https://github.com/niteshkumar-curious
