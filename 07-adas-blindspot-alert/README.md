# Project 07: Vehicle ADAS Dual Blind-Spot & Driver Alert System 🚗⚠️

A simulation of automotive **Advanced Driver Assistance Systems (ADAS)**. It provides real-time side-mirror blind spot monitoring for safe highway lane changes, detects critical side-collision proximity, and actively monitors driver alertness to prevent asleep-at-the-wheel accidents.

---

## 🌟 Key Features
* **Zero Wire Changes:** Runs on the exact same Arduino Uno circuit as Projects 01–06.
* **Dual Blind-Spot Monitoring (BSD):**
  * 🟡 **Sensor 1 (Left Mirror):** Lights the Yellow LED when a vehicle is in the left blind spot ($< 20\text{ cm}$).
  * 🔴 **Sensor 2 (Right Mirror):** Lights the Red LED when a vehicle is in the right blind spot ($< 20\text{ cm}$).
  * 🟢 **All Clear (Center):** Green LED glows solid when both sides are clear (`SAFE TO CHANGE LANES`).
* **Side-Collision Warning:** If a vehicle moves dangerously close ($< 10\text{ cm}$) on either side, the buzzer emits a rapid clicking proximity alarm.
* **Driver Drowsiness Alert (Anti-Sleep):**
  * The IR sensor monitors driver vigilance.
  * If the driver's head slumps or turns away from the road for $> 2.5\text{ seconds}$, a high-intensity police wake-up siren sounds and all LEDs flash!
* **Live Automotive Telemetry:** Real-time lane merge advice printed to the Serial Monitor at **115200 baud**.

---

## 🔌 Pin Connections (Identical to Projects 01–06)

| Component | Pin | Arduino Uno Pin | ADAS Function |
|---|---|---|---|
| **Ultrasonic 1 (Left Mirror)** | TRIG / ECHO | **Pin 9** / **Pin 8** | Left blind-spot sensor |
| **Ultrasonic 2 (Right Mirror)**| TRIG / ECHO | **Pin 7** / **Pin 6** | Right blind-spot sensor |
| **IR Sensor (Driver Vigilance)**| OUT | **Pin 2** | Driver face/eye presence monitor |
| **Piezo Buzzer** | (+) | **Pin 10** | Collision tick & wake-up siren |
| **Green LED (All Clear)** | (+) via 220Ω | **Pin 11** | Safe to merge lanes |
| **Red LED (Right Warning)** | (+) via 220Ω | **Pin 12** | Right mirror alert |
| **Yellow LED (Left Warning)** | (+) via 220Ω | **Pin 13** | Left mirror alert |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo

1. Open `07-adas-blindspot-alert.ino` in Arduino IDE.
2. Select **Arduino Uno** and your COM Port, then click **Upload**.
3. Open **Serial Monitor** at **115200 baud**.
4. **All Clear:** Place a hand near the IR sensor (simulating the driver present). 🟢 **Green LED** stays ON (`SAFE TO CHANGE LANES`).
5. **Left Blind Spot:** Bring your second hand in front of **Sensor 1 (Left)** $\rightarrow$ 🟡 **Yellow LED** turns ON (Left mirror warning).
6. **Right Blind Spot:** Move your hand to **Sensor 2 (Right)** $\rightarrow$ 🔴 **Red LED** turns ON (Right mirror warning).
7. **Collision Alert:** Bring hand $< 10\text{ cm}$ to either sensor $\rightarrow$ rapid collision ticks sound!
8. **Drowsiness / Fall Asleep Test:** Remove your hand from the **IR sensor** for 2.5 seconds $\rightarrow$ 🚨 High-intensity wake-up siren sounds and all LEDs strobe!
