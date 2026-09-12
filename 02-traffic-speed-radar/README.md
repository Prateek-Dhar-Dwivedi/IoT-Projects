# Project 02: High-Speed Traffic Radar & Automated Speed Camera Trap 📸🚗

An automated traffic monitoring system that calculates vehicle velocity between dual ultrasonic trap gates, checks against a configurable speed limit, issues automated citations, flashes a simulated speed camera strobe, and detects intersection stop-line violations.

---

## 🌍 Real-World Problem It Solves
Speeding and red-light infractions are leading causes of urban roadway fatalities. Manual traffic enforcement using handheld radar guns is labor-intensive, expensive, and leaves blind spots across cities. This system automates 24/7 vehicle velocity calculation, direction verification, and stop-line compliance at the micro-edge level.

## 🏢 Practical Real-World Applications
* **Automated Municipal Speed Camera Traps:** Calculates vehicle transit velocity between fixed baseline gates and triggers automated high-speed camera shutter snaps.
* **School & Hospital Zone Calming:** Monitors low-speed safety zones and gives immediate audio-visual warnings to drivers exceeding speed limits.
* **Warehouse & Logistics Forklift Radar:** Monitors indoor forklift traffic speeds in distribution centers to prevent warehouse worker collisions.

## 🧠 Engineering & Technical Significance
* **Microsecond Velocity Derivation:** Computes velocity ($v = \frac{\Delta d}{\Delta t}$) via spatial baseline gate timing using microsecond-precision hardware timestamps.
* **Bidirectional Traffic Discrimination:** Evaluates chronological entry/exit gate sequence to flag dangerous wrong-way drivers.

---

## 🌟 Key Features
* **Dual-Gate Velocity Calculation:** Uses microsecond-precision timing between two ultrasonic sensors ($v = \frac{\Delta d}{\Delta t}$) spaced 10 cm apart.
* **Speed Camera Flash Strobe:** High-speed triple strobe on the Yellow LED (Pin 13) simulating a roadside traffic camera photo snap.
* **Police Siren & Ticket Generation:** Formats an official traffic citation directly into the Serial Monitor for any vehicle exceeding 45 km/h.
* **Wrong-Way Driving Detection:** Detects reverse-flow traffic if Gate 2 is entered before Gate 1.
* **Red-Light Stop-Line Enforcement:** The IR sensor monitors the intersection crosswalk line; crossing during a stop triggers a red-light alarm.

---

## 🔌 Pin Connections (Identical to Project 01)

| Component | Pin | Arduino Uno Pin | Function |
|---|---|---|---|
| **Ultrasonic 1 (Trap Point A / Entry)** | TRIG | **Pin 9** | Trigger entry pulse |
| | ECHO | **Pin 8** | Echo entry return |
| **Ultrasonic 2 (Trap Point B / Exit)** | TRIG | **Pin 7** | Trigger exit pulse |
| | ECHO | **Pin 6** | Echo exit return |
| **IR Proximity Sensor** | OUT | **Pin 2** | Red-light stop-line tripwire |
| **Piezo Buzzer** | (+) | **Pin 10** | Shutter click & siren |
| **Green LED** | (+) via 220Ω | **Pin 11** | Safe speed indicator |
| **Red LED** | (+) via 220Ω | **Pin 12** | Speeding violation / Ticket |
| **Yellow LED** | (+) via 220Ω | **Pin 13** | Camera flash strobe |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 📐 Desk Setup
1. Place **Sensor 1** and **Sensor 2** on your desk facing across a simulated road lane.
2. Space the two sensors **10 cm apart** (center-to-center).
3. Place the **IR Sensor** ahead of the sensors to act as the intersection "stop line".

---

## 🎮 How to Test
1. Open `02-traffic-speed-radar.ino` in Arduino IDE.
2. Select **Arduino Uno** and your COM port, then click **Upload**.
3. Open the **Serial Monitor at 115200 baud**.
4. **Test Safe Speed:** Slide a pen or toy car slowly past Sensor 1 then Sensor 2 $\rightarrow$ 🟢 Green LED glows + double chirp + "SAFE DRIVER" log.
5. **Test Speeding:** Slide the object quickly past Sensor 1 then Sensor 2 $\rightarrow$ 🟡 Yellow LED strobes 3 times like a camera flash + 🔴 Red LED turns ON + Siren sounds + 📸 "SPEED TRAP CITATION" prints!
6. **Test Red Light:** Wave your hand over the IR sensor $\rightarrow$ 🚨 "RED LIGHT VIOLATION" alert!
