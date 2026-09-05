# Project 05: Smart Assistive Walking Aid (Dual-Elevation Hazard Detection) 🦯✨

An assistive technology mobility aid for the visually impaired. Standard canes only detect ground obstacles; this system continuously monitors hazards at both **head/chest level** and **knee/ground level**, while watching for **sudden drop-offs and cliffs**.

---

## 🌟 Key Features
* **Dual-Elevation 3D Hazard Awareness:**
  * **Upper Sensor (Sensor 1):** Detects hanging tree branches, low signs, and head/chest-level hazards ($< 25\text{ cm}$).
  * **Lower Sensor (Sensor 2):** Detects rocks, curbs, fire hydrants, and knee-level obstacles ($< 25\text{ cm}$).
* **Distinct Differentiated Audio Feedback:**
  * **Upper Hazard:** High-pitch sharp chirps ($1800\text{ Hz}$).
  * **Lower Hazard:** Low-pitch pulsing buzz ($600\text{ Hz}$).
  * **Both Blocked (Full Wall):** Continuous high-urgency tone ($2200\text{ Hz}$).
* **Drop-Off / Pothole Detector (IR Sensor):**
  * Monitors for sudden steps down, curb drops, or uncovered manholes with an alternating two-tone alarm siren.
* **Night Pedestrian Safety Beacon:**
  * When the path is clear, the 🟢 **Green LED** is solid and the 🟡 **Yellow LED** pulses as a safety beacon to keep the user visible to cars in the dark.
* **Live Assistive Dashboard:** Real-time navigation reports printed every 1.5 seconds at **115200 baud**.

---

## 🔌 Pin Connections (Identical to Previous Projects)

| Component | Pin | Arduino Uno Pin | Purpose in Project 05 |
|---|---|---|---|
| **Ultrasonic 1 (Upper Hazard)** | TRIG | **Pin 9** | Chest/Head-level trigger |
| | ECHO | **Pin 8** | Chest/Head-level return |
| **Ultrasonic 2 (Lower Hazard)** | TRIG | **Pin 7** | Ground/Knee-level trigger |
| | ECHO | **Pin 6** | Ground/Knee-level return |
| **IR Sensor (Curb / Drop-Off)** | OUT | **Pin 2** | Cliff / Pit / Fall hazard |
| **Piezo Buzzer** | (+) | **Pin 10** | Differentiated audio cues |
| **Green LED (Path Safe)** | (+) via 220Ω | **Pin 11** | Safe to proceed |
| **Red LED (Upper Obstacle)** | (+) via 220Ω | **Pin 12** | Head/Chest hazard alert |
| **Yellow LED (Lower / Beacon)** | (+) via 220Ω | **Pin 13** | Ground hazard & Night beacon |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo

1. Open `05-assistive-walking-aid.ino` in Arduino IDE.
2. Select **Arduino Uno** and your COM Port, then click **Upload**.
3. Open **Serial Monitor** at **115200 baud**.
4. **Test Safe Path:** When nothing is within 25 cm:
   * 🟢 **Green LED** stays ON.
   * 🟡 **Yellow LED** pulses gently like a pedestrian night beacon.
5. **Test Upper Hazard:** Wave your hand in front of **Sensor 1 (Left)** $(< 25\text{ cm})$:
   * 🔴 **Red LED** turns ON.
   * High-pitch sharp audio chirps sound!
6. **Test Lower Hazard:** Wave your hand in front of **Sensor 2 (Right)** $(< 25\text{ cm})$:
   * 🟡 **Yellow LED** turns ON solid.
   * Low-pitch audio pulses sound!
7. **Test Cliff / Drop-off:** Hover over the **IR Sensor**:
   * 🚨 Dual-tone emergency fall alarm rings!
