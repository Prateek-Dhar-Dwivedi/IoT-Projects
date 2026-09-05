# Project 03: Dual-Bay Smart Parking Lot & Reverse Proximity Warner 🅿️🚗

An automated smart garage / multi-bay parking management system that detects slot occupancy in real time, manages entry gate access via an optical barrier, guides drivers with reverse parking audio beeps, and displays a live parking lot dashboard.

---

## 🌟 Key Features
* **Dual-Bay Occupancy Sensing:** 
  * Ultrasonic Sensor A monitors **Parking Bay 1**.
  * Ultrasonic Sensor B monitors **Parking Bay 2**.
* **3-Stage Capacity Indicator:**
  * 🟢 **Green LED:** 2 Slots Free (Vacant).
  * 🟡 **Yellow LED:** 1 Slot Free (Limited Capacity).
  * 🔴 **Red LED:** 0 Slots Free (Parking Lot FULL).
* **Automated Entry Gate (IR Sensor):**
  * When a vehicle arrives at the gate, it checks lot vacancy:
    * If slots are free: Welcome chime plays and Yellow LED pulses ("GATE OPEN").
    * If lot is full: Access denied buzzer sounds and Red LED flashes ("LOT FULL").
* **Reverse Parking Audio Guidance:**
  * Beeps faster as an obstacle backs closer to the wall ($12\text{ cm} - 22\text{ cm} \rightarrow 6\text{ cm} - 12\text{ cm}$).
  * Sounds a continuous emergency tone at $< 6\text{ cm}$ (Collision warning).
* **Live Serial Dashboard:**
  * Real-time slot status, distance measurements, and capacity printed at 115200 baud.

---

## 🔌 Pin Connections (Identical to Projects 01 & 02)

| Component | Pin | Arduino Uno Pin | Purpose |
|---|---|---|---|
| **Ultrasonic 1 (Bay A / Slot 1)** | TRIG | **Pin 9** | Bay 1 ultrasonic trigger |
| | ECHO | **Pin 8** | Bay 1 ultrasonic echo |
| **Ultrasonic 2 (Bay B / Slot 2)** | TRIG | **Pin 7** | Bay 2 ultrasonic trigger |
| | ECHO | **Pin 6** | Bay 2 ultrasonic echo |
| **IR Sensor (Entry Gate)** | OUT | **Pin 2** | Vehicle arrival detector |
| **Piezo Buzzer** | (+) | **Pin 10** | Parking beeper & gate chime |
| **Green LED (Vacant)** | (+) via 220Ω | **Pin 11** | 2 Slots free |
| **Yellow LED (Limited)** | (+) via 220Ω | **Pin 13** | 1 Slot free / Gate open |
| **Red LED (Lot Full)** | (+) via 220Ω | **Pin 12** | 0 Slots free / Full stop |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo
1. Open `03-smart-parking-assistant.ino` in Arduino IDE.
2. Select **Board: Arduino Uno** and your COM Port, then click **Upload**.
3. Open **Serial Monitor** at **115200 baud**.
4. **Test Bay A:** Place a small box/hand in front of Sensor 1 (< 12 cm) $\rightarrow$ LED turns **Yellow** ("1 Slot Left").
5. **Test Bay B:** Place an object in front of Sensor 2 as well $\rightarrow$ LED turns **Red** ("LOT FULL").
6. **Test Entry Gate:** 
   * Wave hand over IR sensor when slots are free $\rightarrow$ Happy welcome chime!
   * Wave hand over IR sensor when both slots are occupied $\rightarrow$ Low error buzz ("ACCESS DENIED")!
7. **Test Reverse Warner:** Slowly bring your hand closer to either sensor $\rightarrow$ Beeps get faster until $< 6\text{ cm}$, when a solid alarm tone sounds!
