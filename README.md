# Sound-Reactive LED System with Time Control (RTC + I2C LCD + Arduino)

This project drives a **bar of LEDs** that react to sound while a **DS3231 RTC** enforces allowed hours. A **potentiometer** sets sensitivity and a **16×2 I2C LCD** shows live time/status. During “DJ Time” (08:00–15:00) LEDs dance to audio; after hours the LEDs turn **OFF** and a **buzzer** alerts.

## 🧩 Components
- Arduino UNO/Nano
- DS3231 RTC module (I2C)
- 16×2 LCD with I2C backpack (PCF8574, `0x27` or `0x3F`)
- Sound sensor (analog out)
- Potentiometer (10k)
- Buzzer
- 6 × LEDs (with resistors) on PWM pins D3, D5, D6, D9, D10, D11
- Wires, breadboard, 5 V power

## 📚 Libraries
`RTClib`, `LiquidCrystal_I2C`, `Wire`  
(Install via Arduino IDE → Tools → Manage Libraries)

## 🔌 Wiring (UNO/Nano)
- **Sound AO → A1**, **Pot → A0**
- **Buzzer → D7**
- **LEDs → D3, D5, D6, D9, D10, D11** (each through a resistor)
- **LCD I2C → SDA A4, SCL A5** (addr `0x27` or `0x3F`)
- **DS3231 → I2C (SDA A4, SCL A5)**

## ⏱️ Time Window
Default active window: **08:00–15:00**.  
Edit in code:
```cpp
int startHour = 8,  startMinute = 0;
int endHour   = 15, endMinute   = 0;
