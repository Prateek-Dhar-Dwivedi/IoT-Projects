# Project 01: Contactless "Air Theremin" & 2-Player Reflex Reaction Game

A contactless electronic musical synthesizer and competitive millisecond reflex reaction game using two ultrasonic sensors, an IR proximity sensor, a buzzer, and 3 LEDs.

---

## 🎯 Microcontroller Selection: Why Arduino Uno?

> **Recommended Board:** **Arduino Uno R3**

| Feature | **Arduino Uno (Recommended)** | **ESP32 / NodeMCU (ESP8266)** |
|---|---|---|
| **Sensor Logic Level** | **Native 5V** — Direct connection to HC-SR04 | **3.3V** — Requires resistor voltage divider on Echo pins |
| **Audio Generation** | Built-in hardware `tone()` support | Requires LEDC PWM timer setup |
| **Pin Simplicity** | Standard digital pins (no boot traps) | Must avoid strapping pins (GPIO0, 2, 15) |
| **Resource Allocation** | Ideal for non-networked standalone builds | Saved for IoT, Web Dashboards & ESP-NOW projects |

---

## 🛠️ Required Hardware
1. **Arduino Uno**
2. **2x Ultrasonic Sensors (HC-SR04)**
3. **1x IR Obstacle / Proximity Sensor** (Active LOW)
4. **1x Buzzer** (Piezo buzzer)
5. **3x LEDs** (Green for Player 1, Red for Player 2, Blue/Yellow for Status)
6. **3x 220Ω Resistors** (for LEDs)
7. Breadboard & Jumper wires

---

## 🔌 Pin Connection Table (Arduino Uno)

| Component | Component Pin | Arduino Uno Pin | Notes |
|---|---|---|---|
| **Ultrasonic 1 (Pitch / Player 1)** | VCC | `5V` | Breadboard power rail |
| | GND | `GND` | Breadboard ground rail |
| | TRIG | `Pin 9` | Trigger pulse |
| | ECHO | `Pin 8` | Echo pulse (5V safe on Uno) |
| **Ultrasonic 2 (Tempo / Player 2)** | VCC | `5V` | Breadboard power rail |
| | GND | `GND` | Breadboard ground rail |
| | TRIG | `Pin 7` | Trigger pulse |
| | ECHO | `Pin 6` | Echo pulse (5V safe on Uno) |
| **IR Proximity Sensor** | VCC | `5V` | Breadboard power rail |
| | GND | `GND` | Breadboard ground rail |
| | OUT | `Pin 2` | Digital Out (Active LOW) |
| **Buzzer** | Positive (+) | `Pin 10` | PWM / Tone output |
| | Negative (-) | `GND` | Ground rail |
| **LED 1 (Player 1 - Green)** | Anode (+) | `Pin 11` (via 220Ω) | Player 1 indicator |
| | Cathode (-) | `GND` | Ground rail |
| **LED 2 (Player 2 - Red)** | Anode (+) | `Pin 12` (via 220Ω) | Player 2 indicator |
| | Cathode (-) | `GND` | Ground rail |
| **LED 3 (Status - Blue/Yellow)** | Anode (+) | `Pin 5` (via 220Ω) | "GO" & rhythm pulse |
| | Cathode (-) | `GND` | Ground rail |

---

## 🚀 How to Run and Test

1. Connect the Arduino Uno to your PC via USB.
2. Open the file `01-air-theremin-reflex-game.ino` in **Arduino IDE**.
3. Select **Board: Arduino Uno** and the appropriate **COM Port**.
4. Click **Upload** (Ctrl + U).
5. Open the **Serial Monitor** (Ctrl + Shift + M) and set baud rate to **115200 baud**.

---

## 🎮 How to Play / Demo

### Mode 1: Air Theremin (Synthesizer)
- **Control Pitch:** Move your left hand closer or further from **Sensor 1** (between 4 cm and 35 cm) to play notes from low bass to high treble.
- **Control Rhythm:** Move your right hand over **Sensor 2** to adjust the staccato pulse tempo.
- **Change Scale / Octave:** Quick-tap/wave your hand over the **IR Sensor** to switch between Standard, High, and Low pitch presets.

### Mode 2: 2-Player Reflex Reaction Game
- **Enter Game:** Hold your hand over the **IR Sensor for 1.5 seconds**. You will hear a double beep indicating game mode.
- **Rules:**
  1. Both players place hands about 20 cm away from their sensor.
  2. Listen to the 3-2-1 countdown beeps.
  3. Wait during the random pause (do **not** move early or you will be disqualified for a false start!).
  4. When you hear the high **"GO!"** beep and the Status LED turns ON, slap or hover your hand over your sensor (< 15 cm).
  5. The first player to react wins! The system flashes the winner's LED and prints the reaction time (in milliseconds) to the Serial Monitor.
