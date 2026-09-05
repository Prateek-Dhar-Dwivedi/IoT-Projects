# Project 04: Sci-Fi "Air-PIN" Touchless Vault & Safe Lock 🔐🛸

A contactless spatial gesture passcode security system. Unlocks only when a user enters the correct 3-digit air gesture PIN sequence across dual ultrasonic sensors and submits via an optical IR sensor.

---

## 🌟 Key Features
* **Contactless Air-PIN Keypad:** Uses spatial distance detection across Left and Right zones to input passcode digits touchlessly.
* **Default Passcode:** `[ LEFT -> RIGHT -> LEFT ]` (or `1 - 2 - 1`).
* **5-Second Input Cooldown Gap:** After registering a digit, the system flashes the Yellow LED and pauses for 5 seconds so you have plenty of time to reposition your hand without accidental multi-triggers.
* **Automated Lockout Security:** 
  * Incorrect code triggers red flashing lights and police siren tones.
  * 3 consecutive failed attempts triggers an emergency **10-second security lockdown**.
* **Sci-Fi Unlock Fanfare:** Green LED (Pin 11) illuminates solid with an electronic victory chime upon access authorization.
* **Manual Re-Lock:** Wave over the IR sensor while unlocked to immediately lock the vault again.

---

## 🔌 Pin Connections (Identical to Projects 01, 02 & 03)

| Component | Pin | Arduino Uno Pin | Function |
|---|---|---|---|
| **Ultrasonic 1 (Keypad Left / 1)** | TRIG | **Pin 9** | Left digit sensor trigger |
| | ECHO | **Pin 8** | Left digit sensor echo |
| **Ultrasonic 2 (Keypad Right / 2)** | TRIG | **Pin 7** | Right digit sensor trigger |
| | ECHO | **Pin 6** | Right digit sensor echo |
| **IR Sensor (Enter Key / Lock)** | OUT | **Pin 2** | Submit code / Re-lock |
| **Piezo Buzzer** | (+) | **Pin 10** | Keypad feedback & alarm |
| **Green LED (Unlocked)** | (+) via 220Ω | **Pin 11** | Access Granted |
| **Red LED (Locked / Alarm)** | (+) via 220Ω | **Pin 12** | Vault Locked / Intruder |
| **Yellow LED (Cooldown / Strobe)**| (+) via 220Ω | **Pin 13** | Keypad input & 5s cooldown |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Unlock the Vault (Step-by-Step)

1. Open **Serial Monitor** at **115200 baud**.
2. Notice the **Red LED** is ON (Vault is Locked).
3. **Enter Digit 1:** Hover hand 5–10 cm over **LEFT sensor** $\rightarrow$ Beep! (Digit 1 recorded).
4. **Wait 5 Seconds:** The **Yellow LED** will blink slowly. Wait until it stops.
5. **Enter Digit 2:** Hover hand 5–10 cm over **RIGHT sensor** $\rightarrow$ Beep! (Digit 2 recorded).
6. **Wait 5 Seconds:** Yellow LED blinks for cooldown.
7. **Enter Digit 3:** Hover hand 5–10 cm over **LEFT sensor** again $\rightarrow$ Beep! (Digit 3 recorded).
8. **SUBMIT:** Wave your hand over the **IR sensor**.
9. **Result:** 🟢 Green LED lights up + Sci-Fi chime plays + 🔓 **ACCESS GRANTED** prints!
