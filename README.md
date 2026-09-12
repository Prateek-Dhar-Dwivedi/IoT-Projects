# IoT Projects Portfolio 🚀

A complete, production-ready portfolio of **11 embedded systems and IoT projects** built using **Arduino Uno R3**, **ESP32-CAM (OV2640)**, **Dual Ultrasonic sensors (HC-SR04)**, **IR proximity sensor**, **piezo buzzer**, and **LED indicators**.

---

## 📦 Hardware Inventory & Architecture

### 1. Arduino Uno Projects (01 – 09): Universal Zero-Rewiring Breadboard Setup
All 9 Arduino projects run on the **exact same hardware wiring**. Once wired, you can upload and demo any of the 9 projects immediately without changing a single jumper wire!
- **Microcontroller:** 1x Arduino Uno R3
- **Sensors:** 2x HC-SR04 Ultrasonic Distance Sensors, 1x Active IR Proximity / Obstacle Sensor
- **Actuators & Outputs:** 1x Piezo Buzzer (Pin 10), 3x LEDs (Green on Pin 7, Red on Pin 8, Yellow on Pin 9) + 220Ω Resistors
- **Breadboard:** 1x Solderless Half/Full Breadboard + Jumper Wires

### 2. Standalone Computer Vision & Edge AI Projects (10 – 11): Zero Breadboard Wiring
Runs 100% on the standalone **AI-Thinker ESP32-CAM module** and its micro-USB daughterboard with zero external jumper wires or breadboards!
- **Microcontroller & Vision Sensor:** AI-Thinker ESP32-CAM module with 2MP OV2640 Camera
- **USB Interface:** ESP32-CAM-MB micro-USB programmer shield (CH340 driver, automatic upload & reset)
- **Features:** Autonomous SoftAP Hotspots, low-latency MJPEG video streaming, high-power night spotlight LED (GPIO 4), single-frame photo capture, edge AI face detection, biometric face enrollment, and synthesized audio security sirens

---

## 🗂️ Complete 11-Project Portfolio Index

| # | Directory | Project Name | Primary Board | Highlights | Status |
|---|---|---|---|---|---|
| **01** | [`01-air-theremin-reflex-game`](./01-air-theremin-reflex-game/) | Contactless Air Theremin & 2-Player Reflex Game | Arduino Uno | Spatial audio synth & millisecond reaction race | ✅ Complete |
| **02** | [`02-traffic-speed-radar`](./02-traffic-speed-radar/) | High-Speed Traffic Radar & Speed Camera Trap | Arduino Uno | Dual-gate velocity calculation ($v=\frac{\Delta d}{\Delta t}$) & automated speeding citations | ✅ Complete |
| **03** | [`03-smart-parking-assistant`](./03-smart-parking-assistant/) | Dual-Bay Smart Parking & Reverse Proximity Warner | Arduino Uno | Automated barrier gate, lot occupancy stats & reverse parking beeper | ✅ Complete |
| **04** | [`04-air-pin-vault-lock`](./04-air-pin-vault-lock/) | Sci-Fi Touchless Air-PIN Safe / Vault Lock | Arduino Uno | Spatial gesture PIN entry, tamper anti-peep & 10s lockout alarm | ✅ Complete |
| **05** | [`05-assistive-walking-aid`](./05-assistive-walking-aid/) | Smart Blind Walking Aid (Head & Foot Detection) | Arduino Uno | Dual-elevation 3D obstacle avoidance & drop-off detection | ✅ Complete |
| **06** | [`06-posture-distance-monitor`](./06-posture-distance-monitor/) | Ergonomic Posture & Screen Distance Monitor | Arduino Uno | Slouch detection, 20-20-20 eye-rest timer & ergonomic productivity logs | ✅ Complete |
| **07** | [`07-adas-blindspot-alert`](./07-adas-blindspot-alert/) | Vehicle ADAS Dual Blind-Spot & Anti-Drowsiness | Arduino Uno | Left/right side-mirror radar, proximity ticks & anti-sleep horn | ✅ Complete |
| **08** | [`08-industrial-qc-station`](./08-industrial-qc-station/) | Factory Conveyor QC Sizing Inspector | Arduino Uno | High-speed item detection, dimensional tolerance sorting & yield stats | ✅ Complete |
| **09** | [`09-air-gesture-desk-assistant`](./09-air-gesture-desk-assistant/) | Touchless Air-Gesture Smart Controller | Arduino Uno | Left/right hand swipes, hover volume adjustment & air-click play/pause | ✅ Complete |
| **10** | [`10-smart-surveillance-camera`](./10-smart-surveillance-camera/) | Smart Wi-Fi Video Security Sentry & Photo Trap | ESP32-CAM | Autonomous Hotspot, live MJPEG web streaming, high-power spotlight & photo capture | ✅ Complete |
| **11** | [`11-ai-face-recognition-sentry`](./11-ai-face-recognition-sentry/) | Real-Time AI Face Detection & Biometric Security Sentry | ESP32-CAM | Edge AI face tracking, biometric enrollment (Subject 0), audio siren & spotlight strobe | ✅ Complete |
