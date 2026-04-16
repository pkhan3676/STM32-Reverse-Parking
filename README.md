# 🚗 STM32 Reverse Parking Sensor System with CAN Bus

![STM32](https://img.shields.io/badge/STM32-F446RE-blue?style=for-the-badge&logo=stmicroelectronics)
![CAN Bus](https://img.shields.io/badge/CAN_Bus-500kbps-green?style=for-the-badge)
![OLED](https://img.shields.io/badge/OLED-SSD1306-orange?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Working-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

> A real-time automotive reverse parking sensor system built on STM32F446RE with CAN bus communication, SSD1306 OLED display, tri-color LED alerts, and 3-sample averaging filter.

---

## 📋 Table of Contents

- [About](#-about)
- [Features](#-features)
- [Hardware](#-hardware)
- [Pin Configuration](#-pin-configuration)
- [CAN Bus Setup](#-can-bus-setup)
- [Software](#-software)
- [How It Works](#-how-it-works)
- [OLED Display](#-oled-display)
- [LED Alert System](#-led-alert-system)
- [Results](#-results)
- [Challenges](#-challenges-and-solutions)
- [Future Work](#-future-work)
- [Getting Started](#-getting-started)
- [Author](#-author)

---

## 🔍 About

This project implements a complete automotive-grade reverse parking sensor system using the STM32F446RE Nucleo development board. Three HC-SR04 ultrasonic sensors measure distances simultaneously, transmit data over a CAN bus network at 500 kbps, display real-time readings on an OLED screen, and alert the driver through color-coded LEDs.

The project demonstrates real-world embedded systems concepts including:
- Timer Input Capture for precise pulse measurement
- CAN bus protocol for automotive communication
- I2C communication with OLED display
- Interrupt-driven sensor reading
- Signal filtering and noise reduction

---

## ✨ Features

| Feature | Details |
|---------|---------|
| 🎯 Sensors | 3x HC-SR04 Ultrasonic Sensors |
| 📡 Communication | CAN Bus @ 500 kbps |
| 🖥️ Display | SSD1306 OLED 128x64 with progress bars |
| 🚦 Alerts | Tri-Color LED (Red/Yellow/Green) |
| 📊 Filter | 3-Sample Moving Average |
| 🔌 Debug | UART @ 115200 baud |
| 🔧 Verification | PCAN-View CAN monitoring |

---

## 🔧 Hardware

### Components List

| Component | Model | Quantity |
|-----------|-------|----------|
| Microcontroller | STM32F446RE Nucleo | 1 |
| Ultrasonic Sensor | HC-SR04 | 3 |
| CAN Transceiver | Waveshare CAN Module | 1 |
| USB-CAN Adapter | PCAN-USB | 1 |
| OLED Display | SSD1306 128x64 | 1 |
| LEDs | Red, Yellow, Green | 3 |
| Resistors | 220Ω (LED), 4.7kΩ (I2C Pull-up), 120Ω (CAN Termination) | - |
| Breadboard | Standard | 1 |

### System Block Diagram

```
┌─────────────┐     TRIG/ECHO      ┌──────────────┐
│  HC-SR04 x3 │◄──────────────────►│              │
└─────────────┘                    │              │
                                   │  STM32F446RE │
┌─────────────┐     I2C            │              │
│ OLED SSD1306│◄──────────────────►│   Nucleo     │
└─────────────┘                    │              │
                                   │              │
┌─────────────┐     GPIO           │              │
│  RGB LEDs   │◄──────────────────►│              │
└─────────────┘                    └──────┬───────┘
                                          │ CAN Bus
                                   ┌──────▼───────┐
                                   │  Waveshare   │
                                   │  Transceiver │
                                   └──────┬───────┘
                                          │ CAN H/L
                                   ┌──────▼───────┐
                                   │  USB-CAN     │
                                   │  PCAN-View   │
                                   └──────────────┘
```

---

## 📌 Pin Configuration

### Sensor Pins

| Sensor | TRIG Pin | ECHO Pin | Timer |
|--------|----------|----------|-------|
| Sensor 1 | PB0 | PA0 | TIM2_CH1 |
| Sensor 2 | PB1 | PA6 | TIM3_CH1 |
| Sensor 3 | PB2 | PB6 | TIM4_CH1 |

### Communication Pins

| Interface | Pin | Function |
|-----------|-----|----------|
| CAN TX | PB9 | CAN1_TX |
| CAN RX | PB8 | CAN1_RX |
| OLED SCL | PB8 | I2C1_SCL |
| OLED SDA | PB9 | I2C1_SDA |
| UART TX | PA2 | USART2_TX |
| UART RX | PA3 | USART2_RX |

### LED Pins

| LED Color | Pin | Status |
|-----------|-----|--------|
| 🔴 Red | PC6 | DANGER |
| 🟡 Yellow | PC7 | WARNING |
| 🟢 Green | PC5 | SAFE |

---

## 📡 CAN Bus Setup

### Configuration

```
Parameter              Value
─────────────────────────────
Baud Rate              500 kbps
Prescaler              6
Time Segment 1         11 TQ
Time Segment 2         2 TQ
Sync Jump Width        1 TQ
Auto Retransmission    Enabled
Auto Bus-Off           Disabled
Termination Resistor   120Ω each end
Message ID             0x123
```

### CAN Data Frame Format

```
Byte 0  │ Sensor 1 Distance HIGH byte
Byte 1  │ Sensor 1 Distance LOW  byte
Byte 2  │ Sensor 2 Distance HIGH byte
Byte 3  │ Sensor 2 Distance LOW  byte
Byte 4  │ Sensor 3 Distance HIGH byte
Byte 5  │ Sensor 3 Distance LOW  byte
```

### Decode Example

```c
// Receiver side decoding
uint16_t d1 = (data[0] << 8) | data[1];  // Sensor 1 in cm
uint16_t d2 = (data[2] << 8) | data[3];  // Sensor 2 in cm
uint16_t d3 = (data[4] << 8) | data[5];  // Sensor 3 in cm
```

---

## 💻 Software

### Development Tools

| Tool | Purpose |
|------|---------|
| STM32CubeIDE | Code development & debugging |
| STM32CubeMX | Peripheral configuration |
| PCAN-View | CAN bus monitoring |
| PuTTY | UART serial monitor |
| afiskon/stm32-ssd1306 | OLED display library |

### Timer Configuration

```
Timer    Bit Width    Prescaler    Resolution    Sensor
──────────────────────────────────────────────────────
TIM2     32-bit       83           1 μs          S1
TIM3     16-bit       83           1 μs          S2
TIM4     16-bit       83           1 μs          S3

APB1 Clock = 84 MHz
Timer Clock = 84 MHz / (83+1) = 1 MHz = 1μs per tick
```

### Averaging Filter

```c
#define AVG_SIZE 3

// Circular buffer
dist_buf[avg_idx] = new_reading;
avg_idx = (avg_idx + 1) % AVG_SIZE;

// Average calculation
float sum = 0;
for(int i = 0; i < AVG_SIZE; i++)
    sum += dist_buf[i];
dist_avg = sum / AVG_SIZE;
```

---

## 🔄 How It Works

```
1. STM32 sends 10μs TRIG pulse to sensor
          │
          ▼
2. HC-SR04 emits 8x 40kHz ultrasonic bursts
          │
          ▼
3. Echo bounces off obstacle
          │
          ▼
4. ECHO pin goes HIGH (TIM Input Capture starts)
          │
          ▼
5. Echo received → ECHO pin goes LOW (capture ends)
          │
          ▼
6. Pulse width measured in microseconds
          │
          ▼
7. Distance = PulseWidth × 0.034 / 2
          │
          ▼
8. 3-sample average calculated
          │
          ▼
9. Data packed into CAN frame and transmitted
          │
          ▼
10. OLED updated + LED changed based on distance
```

### Distance Formula

```
Speed of Sound = 0.034 cm/μs at room temperature

Distance (cm) = Echo Pulse Width (μs) × 0.034 / 2

Division by 2 because sound travels TO object AND BACK
```

---

## 🖥️ OLED Display

### Layout

```
┌────────────────────────────┐
│     PARKING SENSOR         │  ← Title
├────────────────────────────┤
│ S1:  15cm [████████░░░░░] │  ← Distance + Bar
│ S2:  08cm [████░░░░░░░░░] │  ← Distance + Bar
│ S3:  22cm [███████████░░] │  ← Distance + Bar
├────────────────────────────┤
│      ** WARNING **         │  ← Status Text
└────────────────────────────┘
```

### Status Messages

| Zone | Distance | Message |
|------|----------|---------|
| DANGER | ≤ 5cm | Custom alert text |
| WARNING | ≤ 10cm | Custom alert text |
| SAFE | > 10cm | Custom safe text |

### OLED Library Setup (ssd1306_conf.h)

```c
#define SSD1306_USE_I2C
#include "stm32f4xx_hal.h"
#define SSD1306_I2C_PORT    hi2c1
#define SSD1306_I2C_ADDR    (0x3C << 1)
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64
#define SSD1306_INCLUDE_FONT_7x10
```

---

## 🚦 LED Alert System

```
Distance Range    LED        OLED Status     Meaning
──────────────────────────────────────────────────────
0 - 5 cm         🔴 Red      !! DANGER !!    Stop immediately
5 - 10 cm        🟡 Yellow   ** WARNING **   Slow down
> 10 cm          🟢 Green    SAFE            Clear to go
```

### LED Wiring

```
STM32 Pin → 220Ω Resistor → LED Anode (+) → LED Cathode (-) → GND

PC6 → 220Ω → Red LED    → GND
PC7 → 220Ω → Yellow LED → GND
PC5 → 220Ω → Green LED  → GND
```

---

## 📊 Results

### Measurement Accuracy

| Actual Distance | Measured Range | Error |
|----------------|----------------|-------|
| 5 cm | 4 - 6 cm | ±1 cm |
| 10 cm | 9 - 11 cm | ±1 cm |
| 20 cm | 19 - 21 cm | ±1 cm |
| 50 cm | 48 - 52 cm | ±2 cm |
| 100 cm | 97 - 103 cm | ±3 cm |

### System Performance

| Parameter | Result |
|-----------|--------|
| Update Rate | ~200ms |
| CAN Baud Rate | 500 kbps ✅ |
| OLED Refresh | Real-time ✅ |
| Filter Type | 3-sample average |
| Max Range | 200 cm |
| Min Range | 2 cm |

### PCAN-View Verification

```
CAN-ID    Type    Length    Data
────────────────────────────────────────────────
123h      CC      6         00 0F 00 08 00 16
                            ↑↑↑↑  ↑↑↑↑  ↑↑↑↑
                            S1    S2    S3
                            15cm  8cm   22cm
```

---

## 🛠️ Challenges and Solutions

### 1. CAN Bus No Data

```
Problem  : No data in PCAN-View
Cause    : TX/RX pins swapped on transceiver
Solution : STM32 TX → Transceiver RX
           STM32 RX → Transceiver TX
```

### 2. Sensor 2 Wrong Readings

```
Problem  : Random large values from Sensor 2
Cause    : TIM3 is 16-bit, overflow at 65535
Solution : Added overflow detection:
           if(IC2 > IC1) diff = IC2 - IC1;
           else diff = (65535 - IC1) + IC2;
```

### 3. Unstable Readings

```
Problem  : Readings jumping randomly
Cause    : Sensor interference + electrical noise
Solution : Separate triggers with 100ms delay
           + 3-sample averaging filter
```

### 4. OLED Not Working

```
Problem  : Blank OLED display
Cause    : Wrong I2C address + missing fonts
Solution : I2C scan confirmed 0x3C address
           Enabled fonts in ssd1306_conf.h
```

### 5. Build Errors (25 errors)

```
Problem  : Multiple definition errors
Cause    : Duplicate .c files in Inc folder
Solution : Moved .c files to Src folder only
           Clean rebuild
```

### 6. Grounding Issues

```
Problem  : Erratic system behavior
Cause    : No common ground between components
Solution : Common GND rail on breadboard
           for all components
```

---

## 🔮 Future Work

- [ ] 🔊 Buzzer integration for audio alerts
- [ ] 📈 Kalman filter for improved accuracy
- [ ] 🌡️ Temperature compensation for sound speed
- [ ] 📱 Bluetooth module for mobile app
- [ ] 📋 PCB design for permanent installation
- [ ] 🚘 Real vehicle OBD-II integration
- [ ] 📺 Larger display with graphical UI
- [ ] 💾 Data logging to SD card

---

## 🚀 Getting Started

### Prerequisites

```
- STM32CubeIDE (latest version)
- STM32CubeMX
- PCAN-View (for CAN monitoring)
- PuTTY (for UART debug)
```

### Installation

```bash
# Clone repository
git clone https://github.com/yourusername/STM32-Parking-Sensor.git

# Open in STM32CubeIDE
File → Open Projects from File System → Select folder
```

### Add OLED Library

Download from: https://github.com/afiskon/stm32-ssd1306

```
Copy to Core/Inc/:
  ssd1306.h
  ssd1306_fonts.h
  ssd1306_conf.h

Copy to Core/Src/:
  ssd1306.c
  ssd1306_fonts.c
```

### Configure ssd1306_conf.h

```c
#define SSD1306_USE_I2C
#include "stm32f4xx_hal.h"
#define SSD1306_I2C_PORT    hi2c1
#define SSD1306_I2C_ADDR    (0x3C << 1)
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64
#define SSD1306_INCLUDE_FONT_7x10
```

### Build and Flash

```
1. Project → Clean Project
2. Project → Build Project
3. Run → Debug (F11)
```

### Monitor CAN Data

```
1. Connect USB-CAN adapter
2. Open PCAN-View
3. Set Baud Rate: 500 kbps
4. Connect
5. Watch ID 0x123 frames
```

### Monitor UART Debug

```
1. Open PuTTY
2. Connection type: Serial
3. Speed: 115200
4. COM port: STM32 virtual COM
```

---

## 📁 Project Structure

```
STM32-Parking-Sensor/
│
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── ssd1306.h
│   │   ├── ssd1306_conf.h
│   │   └── ssd1306_fonts.h
│   │
│   └── Src/
│       ├── main.c          ← Main application code
│       ├── ssd1306.c       ← OLED driver
│       └── ssd1306_fonts.c ← Font definitions
│
├── Drivers/                ← STM32 HAL drivers
├── README.md               ← This file
└── Reverse_Sensor_CAN.ioc  ← CubeMX config
```

---

## 📄 License

```
MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any
person obtaining a copy of this software to deal in
the Software without restriction, including the rights
to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software.
```

---

## 👨‍💻 Author

**Your Name**

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Connect-blue?style=for-the-badge&logo=linkedin)](https://linkedin.com/in/yourprofile)
[![GitHub](https://img.shields.io/badge/GitHub-Follow-black?style=for-the-badge&logo=github)](https://github.com/yourusername)

---

## ⭐ Support

If this project helped you, please give it a ⭐ on GitHub!

---

*Built with ❤️ using STM32 + CAN Bus + Embedded C*
