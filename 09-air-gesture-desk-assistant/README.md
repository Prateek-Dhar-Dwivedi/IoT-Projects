# Project 09: Touchless "Air Gesture" Smart Controller 🖐️🎵

A futuristic Human-Computer Interaction (HCI) smart desk controller. Controls media playback touchlessly using air swipes, hover height levels for volume adjustment, and optical air-clicks.

---

## 🌍 Real-World Problem It Solves
Interacting with computer interfaces, infotainment systems, or smart appliances when hands are dirty, wet, greasy, or sterile (cooking in kitchens, repairing vehicle engines, performing sterile surgery) leads to damaged touchscreens, germ transmission, and broken sterile barriers. This project implements a touchless spatial Human-Computer Interface (HCI) controlled entirely through mid-air gestures.

## 🏢 Practical Real-World Applications
* **Hospital Surgical Suite Display Control:** Surgeons review MRI/CT radiology scans, zoom images, and toggle screens without touching unsterilized mice or keyboards.
* **Smart Kitchen Hands-Free Appliance Hub:** Control recipe tablets, adjust kitchen exhaust fans, or change songs with dough- or sauce-covered hands.
* **Automotive HUD & Infotainment Gesture Control:** Allows drivers to adjust audio volume or skip music tracks with quick air waves without diverting visual attention from the road.

## 🧠 Engineering & Technical Significance
* **Spatiotemporal Vector Velocity Parsing:** Discriminates swipe direction (Swipe Right vs. Swipe Left) by analyzing millisecond arrival delta ($\Delta t$) across spatial sensor gates.
* **Analog Proximity-to-Level Mapping:** Translates continuous vertical hand elevations ($5\text{ cm} – 30\text{ cm}$) into stepped volume control intervals ($0\% – 100\%$) with hysteresis filtering.

---

## 🌟 Key Features
* **Zero Wire Changes:** Runs on the exact same Arduino Uno circuit as Projects 01–08.
* **Air Swipe Navigation:**
  * 👉 **Swipe Right (Left ➔ Right):** Skips to **Next Track (`>>`)** with rising melody + Green LED sweep.
  * 👈 **Swipe Left (Right ➔ Left):** Goes to **Previous Track (`<<`)** with falling melody + Red LED sweep.
* **Hover Height Volume Control:**
  * 🔊 **Hover High (16–30 cm):** Steps volume UP with ascending beeps and an ASCII volume bar.
  * 🔉 **Hover Low (5–14 cm):** Steps volume DOWN with descending beeps.
* **Air-Click Play / Pause (IR Sensor):**
  * Hovering hand over the center IR sensor toggles **Play ↔ Pause** with state LED indication.
* **Serial Media Commands:** Formats clean, standardized media commands at **115200 baud** that can be piped into Spotify, YouTube, or VLC.

---

## 🔌 Pin Connections (Identical to Previous Projects)

| Component | Pin | Arduino Uno Pin | Purpose in Project 09 |
|---|---|---|---|
| **Ultrasonic 1 (Left Zone)** | TRIG / ECHO | **Pin 9** / **Pin 8** | Left gesture & volume sensor |
| **Ultrasonic 2 (Right Zone)**| TRIG / ECHO | **Pin 7** / **Pin 6** | Right gesture sensor |
| **IR Sensor (Air Click)** | OUT | **Pin 2** | Play / Pause toggle |
| **Piezo Buzzer** | (+) | **Pin 10** | Audio feedback & chirp |
| **Green LED (Next / Vol +)**| (+) via 220Ω | **Pin 11** | Next track indicator |
| **Red LED (Prev / Vol -)** | (+) via 220Ω | **Pin 12** | Prev track indicator |
| **Yellow LED (Play State)** | (+) via 220Ω | **Pin 13** | Play/Pause status LED |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo

1. Open `09-air-gesture-desk-assistant.ino` in Arduino IDE.
2. Select **Arduino Uno** and your COM Port, then click **Upload**.
3. Open **Serial Monitor** at **115200 baud**.
4. **Test Swipe Right:** Swiftly swipe your hand across from Sensor 1 (Left) to Sensor 2 (Right) $\rightarrow$ 🟢 Green LED flashes + rising chirp + `⏭️ NEXT TRACK >>`!
5. **Test Swipe Left:** Swiftly swipe from Sensor 2 (Right) to Sensor 1 (Left) $\rightarrow$ 🔴 Red LED flashes + falling chirp + `⏮️ << PREVIOUS TRACK`!
6. **Test Volume Control:** Hold your hand steady over the Left sensor:
   * Raise hand higher (20 cm) $\rightarrow$ `🔊 VOL + [████████--] 80%`
   * Lower hand closer (8 cm) $\rightarrow$ `🔉 VOL - [████------] 40%`
7. **Test Play/Pause:** Tap your hand over the **IR sensor** $\rightarrow$ 🟡 Yellow LED toggles between `▶️ PLAYING` and `⏸️ PAUSED`!
