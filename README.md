# 🌡️ Smart Fan System — Temperature Controlled IoT

An Arduino-based smart fan controller that reads temperature/humidity via BME280, drives a relay-controlled fan, displays data on an I2C LCD, and optionally sends sensor data to the cloud (ThingSpeak) via ESP8266 WiFi.

---

## 📦 Hardware Required

| Component | Quantity |
|-----------|----------|
| Arduino Nano | 1 |
| BME280 Sensor (I2C) | 1 |
| ESP8266 (ESP-01 or NodeMCU) | 1 |
| Relay Module (5V, single channel) | 1 |
| LCD 16×2 with I2C backpack (PCF8574) | 1 |
| DC Fan (5V or 12V) | 1 |
| Jumper Wires | several |
| Breadboard | 1 |
| 5V Power Supply | 1 |

---

## 🔌 Wiring

### BME280 → Arduino Nano (I2C)
| BME280 | Arduino Nano |
|--------|-------------|
| VCC    | 3.3V        |
| GND    | GND         |
| SDA    | A4          |
| SCL    | A5          |

> Default I2C address: `0x76`. If not detected, try `0x77` (change in code).

### LCD (I2C) → Arduino Nano
| LCD Module | Arduino Nano |
|------------|-------------|
| VCC        | 5V          |
| GND        | GND         |
| SDA        | A4          |
| SCL        | A5          |

> Default I2C address: `0x27`. Try `0x3F` if screen stays blank.

### Relay Module → Arduino Nano
| Relay | Arduino Nano |
|-------|-------------|
| IN    | D7          |
| VCC   | 5V          |
| GND   | GND         |

### ESP8266 → Arduino Nano (SoftwareSerial)
| ESP8266 | Arduino Nano |
|---------|-------------|
| TX      | D10 (RX)    |
| RX      | D11 (TX) ⚠️ |
| VCC     | 3.3V (!)    |
| GND     | GND         |

> ⚠️ ESP8266 RX is 3.3V logic! Use a voltage divider (1kΩ + 2kΩ) on D11 → ESP RX.

---

## 🚀 Setup & Upload

### 1. Install Libraries (Arduino IDE → Manage Libraries)
- `Adafruit BME280 Library`
- `Adafruit Unified Sensor`
- `LiquidCrystal I2C` (by Frank de Brabander)

### 2. Configure WiFi & Cloud
Edit the top of `smart_fan_system.ino`:
```cpp
const char* WIFI_SSID     = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
const char* TS_API_KEY    = "YOUR_THINGSPEAK_WRITE_KEY";
```

### 3. ThingSpeak Setup (free)
1. Register at [thingspeak.com](https://thingspeak.com)
2. Create a new Channel with 3 fields: Temperature, Humidity, Pressure
3. Copy the **Write API Key** into the code

### 4. Upload
- Board: **Arduino Nano**
- Processor: **ATmega328P (Old Bootloader)** if upload fails
- Port: your COM port

---

## ⚙️ How It Works

```
BME280 → reads temp/hum/pressure every 2s
     ↓
Arduino → checks threshold (28°C ON / 25°C OFF)
     ↓
Relay → switches fan ON or OFF
     ↓
LCD → displays current temp, humidity, fan state
     ↓
ESP8266 → every 60s sends data to ThingSpeak cloud
```

**Hysteresis logic** prevents rapid ON/OFF switching near the threshold:
- Fan turns **ON**  when temp ≥ 28°C
- Fan turns **OFF** when temp ≤ 25°C

---

## 📁 Project Structure

```
smart-fan-system/
├── src/
│   └── smart_fan_system.ino   ← main Arduino sketch
├── docs/
│   └── wiring_diagram.png     ← (add your diagram here)
├── README.md
└── LICENSE
```

---

## 🛠️ Troubleshooting

| Problem | Fix |
|---------|-----|
| BME280 not found | Check I2C address (0x76 vs 0x77), check wiring |
| LCD blank | Check I2C address (0x27 vs 0x3F), adjust contrast pot |
| Fan doesn't switch | Check relay wiring, verify RELAY_PIN = 7 |
| ESP8266 not connecting | Check baud rate (try 9600), verify AT firmware |
| Upload fails | Select "Old Bootloader" in board options |

---

## 📜 License

MIT License — free to use, modify, and distribute.

---

*Built for learning embedded systems & IoT. Perfect for TEKNOFEST and university projects.*
