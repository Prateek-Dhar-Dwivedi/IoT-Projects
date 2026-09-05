# Project 01: Contactless "Air Theremin" & 2-Player Reflex Reaction Game 🎶⚡

An interactive dual-mode embedded systems project built on the **Arduino Uno** using ultrasonic ranging, infrared sensing, audio tone synthesis, and visual LED feedback.

---

## 🌟 Features

### 1. Contactless "Air Theremin" (Audio Synthesizer)
* **Pitch Control:** Waving your hand closer or further from **Sensor 1** modulates the audio frequency from deep bass (260 Hz) to high treble (1050 Hz).
* **3-Zone Visual Pitch Meter:**
  * 🟢 **Close (4 cm – 15 cm):** **Green LED** lights up (Low pitch).
  * 🟡 **Middle (15 cm – 25 cm):** **Yellow LED** (Pin 13) lights up and pulses (Mid pitch).
  * 🔴 **Far (25 cm – 38 cm):** **Red LED** lights up (High pitch).
* **Tempo Modulation:** Waving your second hand over **Sensor 2** chops the tone into rhythmic staccato pulses.
* **Octave Shifting:** Tap your hand over the **IR Sensor** to cycle pitch scales (Standard, High, Low).

### 2. Competitive 2-Player Reaction Reflex Game
* **Activation:** Hold hand over the **IR Sensor for 1.5 seconds**.
* **Countdown:** Yellow LED flashes for a 3-2-1 countdown sequence.
* **False-Start Detection:** Any player who breaks cover before the "GO!" signal is automatically disqualified for a foul.
* **The "GO!" Signal:** A high-frequency tone sounds and the Yellow LED illuminates solid.
* **Precision Timing:** Players race to hover over their sensor; the winner's LED flashes and their exact reaction time is measured in milliseconds.

---

## 🛠️ Required Hardware

| Component | Quantity | Description |
|---|---|---|
| **Arduino Uno R3** | 1 | Microcontroller board |
| **HC-SR04 Ultrasonic Sensors** | 2 | Non-contact distance measurement |
| **IR Proximity Sensor** | 1 | Active LOW digital optical barrier |
| **Piezo Buzzer** | 1 | Audio output |
| **LEDs** | 3 | 1x Green, 1x Red, 1x Yellow (or Blue) |
| **220Ω Resistors** | 3 | Current limiting for LEDs |
| **Breadboard & Jumper Wires** | 1 set | For modular wiring |

---

## 🔌 Pin Connection Table (Arduino Uno)

| Component | Pin | Arduino Uno Pin | Purpose |
|---|---|---|---|
| **Power Rails** | VCC | `5V` | Common breadboard `(+)` rail |
| | GND | `GND` | Common breadboard `(-)` rail |
| **Ultrasonic 1 (Pitch / P1)** | TRIG | **Pin 9** | Trigger pulse |
| | ECHO | **Pin 8** | Echo return (5V safe) |
| **Ultrasonic 2 (Tempo / P2)** | TRIG | **Pin 7** | Trigger pulse |
| | ECHO | **Pin 6** | Echo return (5V safe) |
| **IR Sensor** | OUT | **Pin 2** | Digital out (Active LOW) |
| **Piezo Buzzer** | (+) | **Pin 10** | Hardware Tone / PWM output |
| | (-) | `GND` | Ground rail |
| **Green LED (P1 / Close)** | (+) via 220Ω | **Pin 11** | Player 1 indicator / Zone 1 |
| | (-) | `GND` | Ground rail |
| **Red LED (P2 / Far)** | (+) via 220Ω | **Pin 12** | Player 2 indicator / Zone 3 |
| | (-) | `GND` | Ground rail |
| **Yellow LED (Status / Mid)** | (+) via 220Ω | **Pin 13** | "GO!" signal / Zone 2 |
| | (-) | `GND` | Ground rail |

---

## 📋 Breadboard Wiring Guide

```text
Arduino 5V  ──────────────────────────────────────> Red (+) Rail
Arduino GND ──────────────────────────────────────> Blue (-) Rail

[Green LED]  : Pin 11 ──[220Ω]──> Anode (+) | Cathode (-) ──> Blue (-) Rail
[Red LED]    : Pin 12 ──[220Ω]──> Anode (+) | Cathode (-) ──> Blue (-) Rail
[Yellow LED] : Pin 13 ──[220Ω]──> Anode (+) | Cathode (-) ──> Blue (-) Rail

[Buzzer]     : Pin 10 ──────────> (+)       | (-)         ──> Blue (-) Rail
[IR Sensor]  : Pin 2  ──────────> OUT       | VCC->5V     | GND->GND
[Sensor 1]   : Trig->9, Echo->8             | VCC->5V     | GND->GND
[Sensor 2]   : Trig->7, Echo->6             | VCC->5V     | GND->GND
```

---

## 🚀 How to Build & Run

1. Connect the components following the connection table above.
2. Connect your Arduino Uno to your computer via USB.
3. Open `01-air-theremin-reflex-game.ino` in the **Arduino IDE**.
4. Select **Tools > Board > Arduino Uno** and pick your **COM Port**.
5. Click **Upload** (`Ctrl + U`).
6. Open **Tools > Serial Monitor** (`Ctrl + Shift + M`) and set baud rate to **115200**.
