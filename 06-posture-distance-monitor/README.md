# Project 06: Ergonomic Posture & Screen Distance Monitor (IoT Web Dashboard) 🪑📱

A smart desktop ergonomic health monitor built on the **ESP32**. It measures your eye-to-screen distance to prevent digital eye strain, tracks posture/slouching deviations, runs a 20-20-20 eye-rest timer, and **hosts a live real-time HTML5 dark-mode Web Dashboard accessible from your phone over Wi-Fi without needing a router**.

---

## 🌟 Key Features
* **Built-in Wi-Fi Access Point:** The ESP32 broadcasts its own Wi-Fi network (`Smart-Posture-Monitor`). Connect your phone or laptop to view live graphs and stats at `http://192.168.4.1`.
* **Screen Distance Sensing (Sensor 1):** Warns if your face leans closer than 38 cm to the display.
* **Slouching Detection (Sensor 2):** Calibrates to your upright sitting posture and alerts if your torso slumps away from the baseline.
* **20-20-20 Eye Rest Timer:** Automatically tracks continuous desk work and signals a break.
* **Touchless IR Snooze:** Wave your hand over the IR sensor to reset the break timer and recalibrate your posture without touching anything!

---

## 🔌 Pin Connection Table (ESP32 Dev Module)

| Component | Component Pin | ESP32 Pin | Purpose |
|---|---|---|---|
| **Ultrasonic 1 (Screen/Face)** | VCC / GND | `VIN (5V)` / `GND` | Power |
| | TRIG / ECHO | **GPIO 18** / **GPIO 19** | Screen distance trigger/echo |
| **Ultrasonic 2 (Torso/Posture)**| VCC / GND | `VIN (5V)` / `GND` | Power |
| | TRIG / ECHO | **GPIO 21** / **GPIO 22** | Torso distance trigger/echo |
| **IR Proximity Sensor** | OUT | **GPIO 23** | Touchless snooze & calibrate |
| **Piezo Buzzer** | (+) / (-) | **GPIO 25** / `GND` | Posture audio reminder |
| **Green LED (Good Posture)** | (+) via 220Ω | **GPIO 26** / `GND` | Good posture indicator |
| **Red LED (Warning)** | (+) via 220Ω | **GPIO 27** / `GND` | Slouching / Too close alert |
| **Yellow LED (Break Alert)** | (+) via 220Ω | **GPIO 33** / `GND` | 20-20-20 Eye break indicator |

---

## 🚀 How to Build & Connect to Dashboard

1. Wire your **ESP32** using the pin table above.
2. In the Arduino IDE:
   * Select **Tools > Board > ESP32 Dev Module**.
   * Select your **COM Port** and click **Upload**.
3. **Connect via Phone / PC:**
   * Look for the Wi-Fi network: **`Smart-Posture-Monitor`**
   * Password: **`12345678`**
4. Open any browser (Chrome, Safari) and go to:  
   👉 **`http://192.168.4.1`**
5. Sit upright in your chair, click **"Calibrate Upright Sitting Posture"** on the dashboard (or wave over the IR sensor).
6. Watch your screen distance, posture, and rest timer update live on your phone screen!
