# ❄️ HydroChill Controller

HydroChill is a **temperature regulation system** using an Arduino Nano, DS18B20 temperature sensor, Peltier module (TEC1-12706), cooling fans, and a heating pad.  
It supports **automatic** and **manual modes** with cooling/heating overrides.

---

## 🔧 Features
- ✅ Automatic cooling when temperature > 28 °C
- ✅ Automatic heating when temperature < 24 °C
- ✅ Manual override buttons for cooling and heating
- ✅ Relay control for external load/indicator
- ✅ Serial Monitor feedback for debugging
- ✅ Uses **L298N motor driver** to control cooling fans
- ✅ Peltier TEC1-12706 integration for active cooling

---

## 🛠️ Components Used
- Arduino Nano
- DS18B20 Temperature Sensor
- Peltier Module (TEC1-12706)
- L298N Motor Driver Module
- Cooling Fans (x2)
- Heating Pad (via MOSFET/Relay)
- 5V Relay Module
- XL4015 Buck Converter
- Push Buttons (Manual Cool & Manual Heat)
- Power Supply (7.4V LiPo battery or DC input)



---

## 🚀 Getting Started

### 1️⃣ Install Dependencies
- Arduino IDE  
- Libraries:  
  - `OneWire`  
  - `DallasTemperature`

### 2️⃣ Upload Code
1. Open `HydroChill.ino` in Arduino IDE.
2. Select **Board: Arduino Nano** and correct COM port.
3. Upload the sketch.

### 3️⃣ Serial Monitor
- Baud Rate: `9600`
- Example output:
=== HydroChill Temp Controller Ready ===
Temperature: 27.5 °C
MODE: IDLE
Fans: OFF
Heater: OFF
Relay: OFF

---

## 📊 Modes of Operation
| Condition             | Fans | Heater | Relay | Mode          |
|------------------------|------|--------|-------|---------------|
| Temp > 28 °C          | ON   | OFF    | ON    | AUTO COOL     |
| Temp < 24 °C          | OFF  | ON     | OFF   | AUTO HEAT     |
| 24 °C ≤ Temp ≤ 28 °C  | OFF  | OFF    | OFF   | IDLE          |
| Manual Cool Button    | ON   | OFF    | ON    | MANUAL COOL   |
| Manual Heat Button    | OFF  | ON     | OFF   | MANUAL HEAT   |
| Both Buttons Pressed  | OFF  | OFF    | OFF   | LOCK          |

---




## 📂 Repository Structure
