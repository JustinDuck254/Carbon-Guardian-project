# 🌿 Carbon Guardian Project

<p align="center">
  Air quality monitoring system using ESP32, MQ-7, MQ-2 and OLED SSD1306
</p>

<p align="center">
  <img alt="Platform" src="https://img.shields.io/badge/Platform-ESP32-00979D?logo=espressif&logoColor=white">
  <img alt="Language" src="https://img.shields.io/badge/Language-Arduino%20C%2B%2B-00979D?logo=arduino&logoColor=white">
  <img alt="Display" src="https://img.shields.io/badge/OLED-SSD1306-blue">
  <img alt="Sensors" src="https://img.shields.io/badge/Sensors-MQ--7%20%7C%20MQ--2-success">
  <img alt="Version" src="https://img.shields.io/badge/Version-v1.4-orange">
  <img alt="Status" src="https://img.shields.io/badge/Status-Active-brightgreen">
</p>

---

## 📖 Table of Contents

- [Introduction](#-introduction)
- [Features](#-features)
- [Hardware](#-hardware)
- [Pin Mapping](#-pin-mapping)
- [Required Libraries](#-required-libraries)
- [How It Works](#-how-it-works)
- [Alert Thresholds](#-alert-thresholds)
- [Setup & Upload](#-setup--upload)
- [Serial Monitor](#-serial-monitor)
- [Quick Customization](#-quick-customization)
- [Technical Notes](#-technical-notes)
- [Future Development](#-future-development)
- [License](#-license)

---

## 🧠 Introduction

**Carbon Guardian** is a project for measuring and alerting on air gas levels using:
- **MQ-7** to monitor **CO**
- **MQ-2** to monitor **Gas/LPG**
- **OLED SSD1306** to display real-time data
- **LED + Buzzer** to alert when danger is detected

This project is suitable for IoT/Embedded learning, lab demonstrations, or basic indoor environmental monitoring.

---

## ✨ Features

- Reads data from 2 gas sensors (MQ-7 and MQ-2)
- Displays on OLED:
  - CO (ppm), Gas (ppm)
  - Per-channel level: `SAFE / WARN / DANGER`
  - Combined `OVERALL` level
- Auto-calibration on startup (INIT)
- Noise filtering via **EMA**
- Level debouncing via **hysteresis**
- Alert only when `OVERALL = DANGER`:
  - LED ON
  - Buzzer beeps **100ms ON / 100ms OFF**

---

## 🧰 Hardware

- ESP32
- MQ-7 Gas Sensor Module
- MQ-2 Gas Sensor Module
- OLED SSD1306 I2C 128x64 (address `0x3C`)
- LED + current-limiting resistor
- Active buzzer
- Jumper wires, breadboard, stable power supply

---

## 🔌 Pin Mapping

| Component | ESP32 Pin |
|---|---|
| MQ-7 (AO) | **GPIO 35** |
| MQ-2 (AO) | **GPIO 34** |
| LED | **GPIO 25** |
| Buzzer (active) | **GPIO 26** |
| OLED SDA | **GPIO 21** |
| OLED SCL | **GPIO 22** |

> Note: GPIO 34/35 are input-only pins (suitable for ADC reading).

---

## 📚 Required Libraries

Install via Library Manager in Arduino IDE:

- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `Wire` (included with core)

---

## ⚙️ How It Works

### 1) INIT Calibration (startup)
- Collects `INIT_SAMPLES = 80` samples (~16 seconds, 200ms interval)
- Computes averages to derive:
  - `R0_MQ7`
  - `R0_MQ2`
- Once enough samples are collected → switches to real-time measurement mode

### 2) Measurement & Processing
- Reads ADC values from MQ-7 and MQ-2
- Calculates sensor resistance `Rs`
- Converts to ppm using the curve function
- Smooths values using EMA (`ALPHA = 0.15`)

### 3) Level Classification
- Applies `WARN/DANGER` thresholds
- Uses hysteresis to reduce state oscillation when values are near a threshold

### 4) Alerting
- `OVERALL = max(levelCO, levelGas)`
- Alert is triggered only when `OVERALL == DANGER`

---

## 🚨 Alert Thresholds

| Measurement | WARN | DANGER |
|---|---:|---:|
| CO | 20 ppm | 80 ppm |
| GAS | 800 ppm | 2500 ppm |

---

## 🚀 Setup & Upload

1. Open the `.ino` file in Arduino IDE
2. Select the appropriate ESP32 board
3. Select the correct COM port
4. Install all required libraries listed above
5. Upload the code
6. Open Serial Monitor at baudrate **115200**

---

## 🖥️ Serial Monitor

Example log output:
```text
[DATA] CO: 12.3 (SAFE) | GAS: 950.1 (WARN) | OVERALL: WARN | raw7=1234 raw2=1456
```

Log information includes:
- CO/GAS values (ppm)
- Per-channel level
- Overall OVERALL level
- Raw ADC values

---

## 🛠️ Quick Customization

You can adjust the following variables in the code:

- Calibration:
  - `INIT_SAMPLES`
- Filtering:
  - `ALPHA`
- Thresholds:
  - `CO_WARN`, `CO_DANGER`
  - `GAS_WARN`, `GAS_DANGER`
- Hysteresis:
  - `HYS_CO`, `HYS_GAS`
- Curve coefficients:
  - `MQ7_CO_A`, `MQ7_CO_B`
  - `MQ2_LPG_A`, `MQ2_LPG_B`

---

## ⚠️ Technical Notes

- MQ-series sensors are suitable for **relative monitoring / trend alerting** and are not a replacement for industrially calibrated gas measurement devices.
- Accuracy depends on power supply, module quality, temperature/humidity, circuit layout, and environment.
- The current version is optimized for display stability and practical alerting.

---

## 🔮 Future Development

- Send data to MQTT/Blynk/ThingsBoard
- Log measurement history (SD card/cloud)
- Multi-level alerts (WARN = slow beep, DANGER = fast beep)
- Add temperature/humidity sensor for error compensation
- On-device "Recalibrate" button

---

## 📄 License

Recommended to use **MIT License** for easy sharing and reuse.

---

## 🤝 Contributing

All contributions are welcome:
- Optimize filtering algorithms
- Add support for additional sensors/components
- Improve measurement accuracy
- Enhance display interface / dashboard
- Add IoT features
