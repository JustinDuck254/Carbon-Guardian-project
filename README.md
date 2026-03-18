# Carbon Guardian – Smoke & CO Detector

A simple Arduino-based smoke and carbon-monoxide (CO) alarm built with an **ESP32**, an **MQ2** gas/smoke sensor, an **MQ7** CO sensor, and an active **buzzer**.

---

## Features

| Feature | Detail |
|---|---|
| Smoke / combustible-gas detection | MQ2 sensor |
| Carbon Monoxide (CO) detection | MQ7 sensor |
| Audible alarm | Active buzzer |
| Serial monitor output | 115200 baud – real-time sensor values and alarm state |
| Easy threshold tuning | `MQ2_THRESHOLD` and `MQ7_THRESHOLD` constants in the sketch |

---

## Hardware

| Component | Quantity |
|---|---|
| ESP32 dev board | 1 |
| MQ2 gas/smoke sensor module | 1 |
| MQ7 carbon-monoxide sensor module | 1 |
| Active buzzer | 1 |
| Jumper wires + breadboard | as needed |

---

## Wiring

```
MQ2  AOUT  ──►  GPIO 34  (ADC1_CH6 on ESP32)
MQ7  AOUT  ──►  GPIO 35  (ADC1_CH7 on ESP32)
Buzzer (+) ──►  GPIO 26
Buzzer (-) ──►  GND

MQ2  VCC   ──►  5 V
MQ2  GND   ──►  GND

MQ7  VCC   ──►  5 V
MQ7  GND   ──►  GND
```

> **Note:** MQ sensor heater elements require 5 V.  
> GPIO 34 and 35 on ESP32 are input-only ADC pins that are 3.3 V tolerant —  
> connect only the **AOUT** signal line, not VCC, to these pins.

---

## Software Setup

1. Install the **Arduino IDE** (≥ 2.x recommended).
2. Add ESP32 board support:  
   *File → Preferences → Additional boards manager URLs*  
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Install the `esp32` board package via *Tools → Board → Boards Manager*.
4. Open `arduino/CarbonGuardian/CarbonGuardian.ino`.
5. Select your board (*Tools → Board → ESP32 Dev Module*) and the correct **Port**.
6. Click **Upload**.

---

## Calibration

After uploading, open the **Serial Monitor** at **115200 baud** and let the sensors warm up for ~20 seconds (handled automatically in `setup()`).

In clean air, note the idle ADC values printed for `MQ2` and `MQ7`.  
Adjust the thresholds in the sketch if needed:

```cpp
#define MQ2_THRESHOLD   1500   // raise/lower to tune smoke sensitivity
#define MQ7_THRESHOLD   1800   // raise/lower to tune CO sensitivity
```

---

## Serial Monitor Output

```
=== Carbon Guardian booting ===
Sensor warm-up: 20 seconds...
20... 19... 18... ...
Ready. Monitoring started.
MQ2=312 (OK)  |  MQ7=420 (OK)
MQ2=315 (OK)  |  MQ7=418 (OK)
!!! ALARM TRIGGERED !!!
MQ2=1823 (SMOKE!)  |  MQ7=2104 (CO!)
```

---

## File Structure

```
arduino/
└── CarbonGuardian/
    └── CarbonGuardian.ino   ← main Arduino sketch
```

---

## License

This project is open-source. Feel free to use and modify it for your own needs.