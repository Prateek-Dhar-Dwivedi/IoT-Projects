# Project 06: Ergonomic Posture & Screen Distance Monitor 🪑✨

A smart desktop ergonomic health monitor built on the **Arduino Uno** using the **exact same breadboard wiring as Projects 01–05**. It measures eye-to-screen distance to prevent digital eye strain, tracks torso slouching deviations from a baseline, runs a 20-20-20 eye-rest timer, and features touchless IR snoozing.

---

## 🌟 Key Features
* **Zero Wire Changes:** Runs on the exact same Arduino Uno circuit as previous projects.
* **Screen Distance Sensing (Sensor 1):** Warns if your face leans closer than 38 cm to the display.
* **Slouching Detection (Sensor 2):** Calibrates to your upright sitting baseline and alerts if your torso slumps forward or backward.
* **20-20-20 Eye Rest Timer:** Automatically tracks continuous desk work and turns on the Yellow LED to signal an eye break.
* **Touchless IR Snooze & Re-calibration:** Wave your hand over the IR sensor to reset the break timer and recalibrate your posture without touching any buttons!
* **Live Ergonomic Dashboard:** Real-time feedback printed to the Serial Monitor every 1.5 seconds at **115200 baud**.

---

## 🔌 Pin Connections (Identical to Projects 01–05)

| Component | Pin | Arduino Uno Pin | Purpose in Project 06 |
|---|---|---|---|
| **Ultrasonic 1 (Screen/Face)** | TRIG | **Pin 9** | Screen distance trigger |
| | ECHO | **Pin 8** | Screen distance return |
| **Ultrasonic 2 (Torso/Posture)**| TRIG | **Pin 7** | Torso posture trigger |
| | ECHO | **Pin 6** | Torso posture return |
| **IR Sensor (Touchless Snooze)**| OUT | **Pin 2** | Snooze & recalibrate |
| **Piezo Buzzer** | (+) | **Pin 10** | Gentle posture chirp |
| **Green LED (Good Posture)** | (+) via 220Ω | **Pin 11** | Excellent posture |
| **Red LED (Warning)** | (+) via 220Ω | **Pin 12** | Slouching / Too close |
| **Yellow LED (Eye Break)** | (+) via 220Ω | **Pin 13** | 20-20-20 Break alert |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo

1. Open `06-posture-distance-monitor.ino` in Arduino IDE.
2. Select **Board: Arduino Uno** and your COM Port, then click **Upload**.
3. Open **Serial Monitor** at **115200 baud**.
4. **Auto-Calibration:** Sit upright at normal desk distance $\rightarrow$ the system chirps and locks in your posture baseline.
5. **Good Posture:** 🟢 **Green LED** is ON (`EXCELLENT POSTURE`).
6. **Test Screen Distance:** Lean closer to Sensor 1 (< 38 cm) $\rightarrow$ 🔴 **Red LED** turns ON and monitor warns `TOO CLOSE TO SCREEN! LEAN BACK`.
7. **Test Slouching:** Slump your torso closer or further from Sensor 2 $\rightarrow$ 🔴 **Red LED** warns `SLOUCHING DETECTED! SIT UPRIGHT`.
8. **Test Touchless Snooze:** Wave your hand over the **IR sensor** to dismiss break reminders and reset your posture baseline!
