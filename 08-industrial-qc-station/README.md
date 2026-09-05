# Project 08: Industrial Assembly Line & Quality Control (QC) Sizing Inspector 🏭📦

An automated industrial manufacturing quality-control (QC) station. Simulates an automated factory conveyor where components enter an optical gate, get scanned for dimensional tolerances by dual ultrasonic sensors, and are automatically routed to **Pass (Shipping)** or **Reject (Defect Bin)** with live statistical yield tracking.

---

## 🌟 Key Features
* **Zero Wire Changes:** Runs on the exact same Arduino Uno circuit as Projects 01–07.
* **Optical Item Arrival Trigger (IR Sensor):** Detects when a product arrives on the simulated conveyor belt.
* **Dual-Axis Dimensional Profiling:**
  * **Sensor 1 (Height):** Scans vertical height (allowed: 6.0 cm to 15.0 cm).
  * **Sensor 2 (Width):** Scans lateral thickness/depth (allowed: 6.0 cm to 15.0 cm).
* **Automated Decision Engine:**
  * 🟢 **Green LED + Melodic Chime:** Inspection Passed! Part is within engineering spec.
  * 🔴 **Red LED + Warning Horn:** Defect Detected! Part is too tall, too short, or misaligned.
* **Live Factory Yield Telemetry:** Real-time production report printed to the Serial Monitor showing Total Count, Pass %, and Defect Rate!

---

## 🔌 Pin Connections (Identical to Previous Projects)

| Component | Pin | Arduino Uno Pin | Purpose in Project 08 |
|---|---|---|---|
| **IR Sensor (Part Arrival)** | OUT | **Pin 2** | Optical conveyor gate trigger |
| **Ultrasonic 1 (Height)** | TRIG / ECHO | **Pin 9** / **Pin 8** | Height measurement sensor |
| **Ultrasonic 2 (Width)** | TRIG / ECHO | **Pin 7** / **Pin 6** | Width measurement sensor |
| **Piezo Buzzer** | (+) | **Pin 10** | Pass chime / Defect alarm |
| **Green LED (QC Passed)** | (+) via 220Ω | **Pin 11** | Quality approved indicator |
| **Red LED (QC Rejected)** | (+) via 220Ω | **Pin 12** | Defect reject indicator |
| **Yellow LED (Scan Active)** | (+) via 220Ω | **Pin 13** | Inspection in progress strobe |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo

1. Open `08-industrial-qc-station.ino` in Arduino IDE.
2. Select **Board: Arduino Uno** and your COM Port, then click **Upload**.
3. Open **Serial Monitor** at **115200 baud**.
4. **Test a "Good" Part:**
   * Place a small object (6 to 14 cm) in front of the ultrasonic sensors.
   * Wave your hand over the **IR sensor** (simulating the part arriving at the station).
   * 🟡 Yellow LED flashes (Scanning...) $\rightarrow$ 🟢 **Green LED** lights up + happy chime!
   * Monitor prints: `🟢 PASSED -> APPROVED FOR SHIPPING` with updated Yield %.
5. **Test a "Defective" Part:**
   * Place an object very close (< 5 cm) or remove it completely so it's out of range (> 16 cm).
   * Wave over the **IR sensor**.
   * 🔴 **Red LED** latches + Defect alarm sounds!
   * Monitor prints: `🔴 REJECTED -> DEFECT EJECTED`.
