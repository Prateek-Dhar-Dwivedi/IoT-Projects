# Project 11: Real-Time AI Face Detection & Biometric Security Sentry 🛡️👁️

An autonomous, edge-computed Computer Vision and Biometric Security Sentry running directly on the **AI-Thinker ESP32-CAM** mounted on the **ESP32-CAM-MB micro-USB daughterboard**.

Features live low-latency MJPEG video streaming, client-side Edge AI facial detection and spatial tracking, on-the-fly facial biometric enrollment (**"Subject 0"**), automated unauthorized intruder classification, dynamic synthesized Web Audio security sirens, and automated high-power spotlight strobe defense (**GPIO 4**).

---

## 🌍 Real-World Problem It Solves

Traditional CCTV security cameras are purely passive: they record hundreds of hours of raw, unindexed video footage without understanding what is happening in their field of view. When an intrusion occurs, building owners only discover the security breach hours or days after the incident when manually scrubbing through footage.

Conversely, traditional PIR motion sensors generate overwhelming numbers of false alarms: they trigger whenever a house pet moves, curtains rustle in the AC airflow, or shadows pass across a window. 

This project solves this dilemma by bringing **Edge Artificial Intelligence and Biometric Vision Verification directly to the sensor node**:
1. **Zero Cloud Latency & Total Privacy:** Video streams and facial feature vectors never leave your private local network. No video is uploaded to cloud servers, eliminating corporate subscriptions, cloud latency, and third-party data privacy breaches.
2. **False-Alarm Immunity via Biometric Distinction:** Rather than triggering indiscriminately on simple thermal or pixel noise, the system actively scans for human facial geometry.
3. **Instant Real-Time Deterrence:** Upon identifying an unverified human face, the sentry immediately executes deterrent actions in real time: flashing the high-power onboard spotlight, sounding an audible alert siren, and capturing an unalterable timestamped snapshot.

---

## 🏢 Practical Real-World Applications

* **High-Security Facility & Server Room Gatekeeper:** Restricts entry or issues immediate alerts when unverified personnel enter server rooms, laboratories, or vaults without authorized escort.
* **Smart Home Doorbell & Porch Sentry:** Greets enrolled homeowners with an authorized green status banner while actively warning delivery couriers or porch pirates with an automated strobe and warning chime.
* **Industrial Construction Site & Restricted Hazard Sentry:** Detects unauthorized workers entering restricted heavy-machinery zones or high-voltage switchgear areas.
* **Elderly & Patient Room Wandering Monitor:** Distinguishes between resident patients and unfamiliar visitors or alerts nursing staff if a patient leaves their room at night.
* **Retail Anti-Theft & Cash Register Surveillance:** Detects human presence behind employee-only checkout counters or stockrooms after business hours.

---

## 🧠 Engineering & Technical Significance

* **Zero-Breadboard, Single-Board Architecture:** Operates 100% on the ESP32-CAM module and its micro-USB programmer shield with **zero external jumper wires, no external breadboards, and no additional sensors**.
* **Edge Computer Vision with Subsampled Chrominance Modeling:** Employs an ultra-fast normalized skin-chroma probability clustering algorithm based on the Kovac/Chai color distribution model ($R > G > B$, $|R - G| \ge 12$, normalized $r/g$ coordinates) coupled with geometric aspect ratio filtering. Subsampled spatial scanning allows processing at 12–20 FPS directly in the browser canvas.
* **16-Point Spatial Biometric Signature Matching:** When a user clicks **"Enroll Face"**, the system captures a 16-point normalized spatial luminance and chroma feature vector $\vec{v}$. Incoming faces are matched using vector cosine similarity:
  $$\text{Similarity}(\vec{u}, \vec{v}) = \frac{\vec{u} \cdot \vec{v}}{\|\vec{u}\| \|\vec{v}\|} \times 100\%$$
  Faces exceeding 78% cosine similarity are validated as **Subject 0 (Authorized)**; unverified profiles trigger **Threat Level HIGH**.
* **Synthesized Web Audio API Alarm:** Leverages the client browser's native `AudioContext` to synthesize an alarm siren using frequency sweeps ($800\text{ Hz} \to 1400\text{ Hz}$) through connected phone/PC speakers without needing physical buzzers.
* **Hardware Brownout Bypass & FreeRTOS Dual-Core Execution:** Uses `WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);` to bypass transient USB current drops during radio transmission, running video DMA capture and HTTP streaming concurrently.
* **Dual Wi-Fi Mode with Autonomous SoftAP Hotspot:** Automatically connects to your local 2.4 GHz Wi-Fi network, and seamlessly falls back to an independent SoftAP Access Point (`ESP32-CAM-AI-SENTRY` / `password123`) if Wi-Fi is unavailable.

---

## 🔌 Hardware Architecture & Pinout

| ESP32-CAM Pin | Physical Hardware Component | Functional Purpose |
|:---:|:---:|:---|
| **GPIO 4** | On-Board High-Power Flash LED | Active security spotlight & automated intruder strobe |
| **GPIO 33** | On-Board Miniature Red LED | System armed heartbeat blink (Active LOW) |
| **GPIO 0, 5, 18, 19, 21, 22, 23, 25, 34-36, 39** | OV2640 2MP Camera Sensor | DVP 8-bit parallel video bus, XCLK, PCLK & VSYNC |
| **GPIO 26, 27** | SCCB (I2C) Interface | Camera sensor register configuration |
| **GPIO 1, 3** | CH340 USB-UART Shield | Serial Monitor debugging (115200 Baud) & sketch flashing |

---

## 🛠️ Arduino IDE Flashing Guide

1. Mount the **ESP32-CAM** firmly onto the **ESP32-CAM-MB micro-USB programmer shield**.
2. Connect the shield to your PC using a micro-USB data cable.
3. Open Arduino IDE and select:
   * **Board:** `ESP32 Dev Module` (under `esp32`)
   * **Port:** COM port corresponding to your CH340 chip
   * **Upload Speed:** `115200` *(Crucial: do not use 460800 or 921600 with the MB shield)*
   * **Flash Frequency:** `40MHz`
   * **Flash Mode:** `DIO`
   * **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)`
   * **PSRAM:** `Enabled`
4. Click **Upload** and wait for `Leaving... Hard resetting via RTS pin...`.
5. Open the **Serial Monitor** at **115200 baud** and press the **RST button** on the shield to view boot diagnostics.

---

## 💻 Interactive Web Dashboard User Guide

1. Open your browser and navigate to the IP address printed in the Serial Monitor (e.g., `http://192.168.x.x` or `http://192.168.4.1` on hotspot).
2. **Face Scanning:** Point the camera at a face. The AI vision engine will detect the face and draw a targeting reticle with real-time tracking coordinates and confidence metric.
3. **Biometric Enrollment:** Click **"Enroll Face (Subject 0)"**. The camera memorizes your facial profile and emits an audible success chime.
4. **Intruder Testing:** Have another person look into the camera or test with different facial profiles. The system immediately draws a flashing red **ALERT: INTRUDER** box, sounds the audio siren, pulses the spotlight, and logs the incident in the **Live Security Incident Log**.
5. **Evidence Snapshots:** Click **"Snap Evidence"** at any time to capture a still photograph and download it directly to your phone or computer.
