# Project 05: Smart Assistive Walking Aid (Dual-Elevation Hazard Detection) 🦯✨

An assistive technology mobility aid for the visually impaired. Standard canes only detect ground obstacles; this system continuously monitors hazards at both **head/chest level** and **knee/ground level**, while watching for **sudden drop-offs and cliffs**.

---

## 🌟 Key Features
* **Live "HEAD" vs "FOOT" Detection:**
  * 👤 **Sensor 1 (Upper):** Detects head-level hazards (overhanging branches, signboards) and prints `[HEAD LEVEL ALERT] Someone at HEAD!`.
  * 🦶 **Sensor 2 (Lower):** Detects foot/knee obstacles (curbs, rocks, fire hydrants) and prints `[FOOT LEVEL ALERT] Someone at FOOT!`.
  * 🛑 **Both Triggered:** Warns of a solid wall or large barrier directly ahead.
* **Quiet-By-Default Audio Engine:**
  * Completely silent when path is clear.
  * Emits spaced-out, non-intrusive chirps (never infinite screeching buzz).
* **Drop-Off / Pothole Detector (IR Sensor):**
  * Monitors for sudden downward stairs, manholes, or curbs with a two-tone alert chirp.
* **Visual Navigation Indicators:**
  * 🟢 **Green LED:** Path is clear and safe to walk.
  * 🔴 **Red LED:** Head/Chest level obstacle alert.
  * 🟡 **Yellow LED:** Ground/Foot obstacle alert.

---

## 🔌 Pin Connections (Identical to Previous Projects)

| Component | Pin | Arduino Uno Pin | Purpose in Project 05 |
|---|---|---|---|
| **Ultrasonic 1 (HEAD Level)** | TRIG | **Pin 9** | Upper hazard trigger |
| | ECHO | **Pin 8** | Upper hazard echo |
| **Ultrasonic 2 (FOOT Level)** | TRIG | **Pin 7** | Lower hazard trigger |
| | ECHO | **Pin 6** | Lower hazard echo |
| **IR Sensor (Curb / Drop-Off)** | OUT | **Pin 2** | Pothole / Cliff barrier |
| **Piezo Buzzer** | (+) | **Pin 10** | Differentiated audio chirps |
| **Green LED (Path Safe)** | (+) via 220Ω | **Pin 11** | Safe to walk |
| **Red LED (Head Obstacle)** | (+) via 220Ω | **Pin 12** | Head level warning |
| **Yellow LED (Foot Obstacle)**| (+) via 220Ω | **Pin 13** | Foot level warning |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo

1. Open **Serial Monitor** at **115200 baud**.
2. **Safe Path:** 🟢 Green LED is ON, buzzer is silent, and monitor prints `🟢 [PATH CLEAR] All Clear - Safe to Walk`.
3. **Head Obstacle:** Place hand in front of **Sensor 1 (Left)** $\rightarrow$ 🔴 Red LED lights up, high chirp sounds, and monitor announces `👤 [HEAD LEVEL ALERT] Someone at HEAD!`.
4. **Foot Obstacle:** Place hand in front of **Sensor 2 (Right)** $\rightarrow$ 🟡 Yellow LED lights up, low beep sounds, and monitor announces `🦶 [FOOT LEVEL ALERT] Someone at FOOT!`.
5. **Drop-Off Test:** Hover over the **IR Sensor** $\rightarrow$ 🚨 Pothole / Drop-off alarm triggers!
