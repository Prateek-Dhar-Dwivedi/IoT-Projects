# Project 10: Multi-Zone Wireless Security Perimeter (ESP-NOW Network) 📡🚨

A distributed wireless security system connecting an **ESP32 Master Hub** to remote **NodeMCU (ESP8266) sentry nodes** using the ultra-fast **ESP-NOW** peer-to-peer radio protocol (operating completely without a router or Wi-Fi network).

---

## 🌟 Key Features
* **Zero Router Needed:** Uses ESP-NOW raw 2.4 GHz radio frames for sub-millisecond wireless packet transfer across separate battery/USB powered devices.
* **Remote Perimeter Sentry (NodeMCU):** Positioned across the room with an ultrasonic tripwire. When breached, it transmits an instant intrusion packet.
* **Master Base Station (ESP32):** Receives the wireless alert, identifies which remote zone was breached, triggers zone-specific LED indicators, and sounds the main siren horn.
* **Expandable Mesh Topology:** Add as many remote sentries (NodeMCU, ESP-12E) as you want to monitor multiple doors, windows, and gates simultaneously!

---

## 🔌 Hardware Setup (Zero-Waste, Minimal Jumper Wires)

### Device A: Master Hub (ESP32)
*Total wires needed: Only 2 jumper wires!*
| Component | Pin | ESP32 Pin | Function |
|---|---|---|---|
| **Piezo Buzzer** | (+) | **GPIO 25** | Main police siren |
| | (-) | **GND** | Ground |
| **Onboard Blue LED** | Internal | **GPIO 2** | Built-in Armed / Alert strobe (0 wires needed!) |
| *Optional Red LED* | (+) via 220Ω | **GPIO 27** | Zone 1 Breach Indicator |
| *Optional Green LED*| (+) via 220Ω | **GPIO 26** | System Armed Indicator |

### Device B: Remote Sentry (NodeMCU ESP-12E / ESP8266)
*Total wires needed: Only 4 jumper wires!*
| Component | Pin | NodeMCU Pin | Function |
|---|---|---|---|
| **Ultrasonic Sensor (HC-SR04)** | VCC | **VIN** (or **VU**) | Direct 5V from USB port (no external power needed!) |
| | GND | **GND** | Sensor Ground |
| | TRIG | **D1** (GPIO 5) | Ultrasonic Trigger Pulse |
| | ECHO | **D2** (GPIO 4) | Ultrasonic Echo Pulse |
| **Onboard Blue LED** | Internal | **D4** (GPIO 2) | Built-in Breach Transmission indicator (0 wires needed!) |

---

## 🚀 How to Run & Demo

1. **Upload `Receiver_Hub_ESP32.ino` to the ESP32:**
   * Open Serial Monitor at **115200 baud**.
   * Note the MAC address printed at startup (by default, the code uses broadcast `FF:FF:FF:FF:FF:FF` so it works out-of-the-box with any node!).
2. **Upload `Sender_Node_NodeMCU.ino` to the NodeMCU:**
   * Place the NodeMCU across your desk or in another room connected to a USB power bank or phone charger.
3. **Trigger the Alarm:**
   * Walk or wave your hand in front of the remote NodeMCU's ultrasonic sensor ($< 30\text{ cm}$).
   * Instantly, without any Wi-Fi router, the **ESP32 Master Hub across the room flashes the Red LED and sounds the intruder siren!**
