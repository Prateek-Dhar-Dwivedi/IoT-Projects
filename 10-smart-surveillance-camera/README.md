# Project 10: Smart Wi-Fi Video Security Sentry & Photo Trap 🚨📹

A high-performance, standalone wireless video surveillance sentry and optical intruder trap built using the **ESP32-CAM (AI-Thinker module + OV2640 2MP Camera + ESP32-CAM-MB USB programmer shield)**.

---

## 🌟 Key Features
* **Zero-Setup Autonomous Wi-Fi Hotspot (SoftAP):** The camera generates its own Wi-Fi network (`ESP32-CAM-Sentry`) on boot. You can connect your phone, tablet, or laptop directly to **`http://192.168.4.1`** without needing any home router, Wi-Fi password, or internet connection!
* **Low-Latency Live Video Stream:** Delivers high-framerate, sub-100ms MJPEG video streaming directly into any modern web browser (iOS Safari, Android Chrome, Windows/Mac Edge & Firefox).
* **On-Demand High-Power Flashlight Strobe:** Control the blinding onboard spotlight LED (**GPIO 4**) directly from the web interface for night-vision illumination.
* **Instant Photo Capture:** Snap high-resolution OV2640 still frames with a single click, view them instantly on the dashboard, or download them to your device.
* **Dynamic Resolution Switching:** Switch resolutions on-the-fly between **QVGA** (ultra-fast 30+ FPS), **VGA** (smooth balance), and **SVGA** (crisp detail) right from the UI buttons.

---

## 🔌 Hardware Setup

### Components:
* **Microcontroller & Camera:** AI-Thinker ESP32-CAM module with OV2640 camera sensor.
* **USB Programmer Daughterboard:** ESP32-CAM-MB shield (plugs onto the bottom of the ESP32-CAM, providing a direct Micro-USB port, auto-reset, and CH340 serial chip).
* **Power:** Direct Micro-USB cable from PC, 5V phone charger, or USB power bank.

### Onboard Actuators & Indicators:
| Actuator / Component | Pin | Function |
|---|---|---|
| **High-Power Spotlight LED** | **GPIO 4** | Blinding white flash LED for night surveillance |
| **Status Indicator LED** | **GPIO 33** | Internal red LED (Active LOW) |
| **OV2640 Camera Sensor** | 8-bit DVP Bus | 2 Megapixel optical image sensor |

---

## ⚙️ Arduino IDE Configuration

To compile and flash the ESP32-CAM, set the following parameters under the **Tools** menu in Arduino IDE:

| Setting | Value |
|---|---|
| **Board** | **AI Thinker ESP32-CAM** |
| **CPU Frequency** | `240MHz (WiFi/BT)` |
| **Flash Frequency** | `80MHz` |
| **Flash Mode** | `QIO` |
| **Partition Scheme** | **Huge APP (3MB No OTA/1MB SPIFFS)** *(Crucial!)* |
| **Core Debug Level** | `None` |
| **PSRAM** | **Enabled** |
| **Port** | Choose your active COM port |

---

## 🚀 How to Run & Demo

1. **Mount the ESP32-CAM:**
   * Plug the ESP32-CAM module on top of the **ESP32-CAM-MB** shield (align the pins carefully).
   * Connect the board to your PC using a standard Micro-USB cable.

2. **Upload the Code:**
   * Open [`Smart_Sentry_Camera.ino`](./Smart_Sentry_Camera.ino) in Arduino IDE.
   * Verify the **Tools** menu matches the table above (especially **Partition Scheme $\rightarrow$ Huge APP** and **PSRAM $\rightarrow$ Enabled**).
   * Click **Upload**.

3. **Open the Live Web Dashboard:**
   * On your smartphone or laptop, open your Wi-Fi settings.
   * Connect to the Wi-Fi network:
     * **SSID:** `ESP32-CAM-Sentry`
     * **Password:** `password123`
   * Open your browser and navigate to: **`http://192.168.4.1`**

4. **Test the Sentry Features:**
   * **Live Stream:** Watch the real-time video feed.
   * **Spotlight:** Tap **💡 Spotlight: OFF** $\rightarrow$ the high-power flash LED instantly turns ON, turning night into day! Tap again to turn OFF.
   * **Snap Photo:** Tap **📸 Snap Photo** $\rightarrow$ a full-resolution JPEG capture is rendered immediately with a **💾 Download Image** button.
   * **Resolution Switcher:** Tap **QVGA**, **VGA**, or **SVGA** to watch the video resolution update dynamically in real time.
