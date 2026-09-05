# Project 04: Sci-Fi "Air-PIN" Touchless Vault & Safe Lock 🔐🛸

A contactless spatial gesture passcode security system. Unlocks only when a user enters the correct 3-digit air gesture PIN sequence across dual ultrasonic sensors and submits via an optical IR sensor.

---

## 🌟 Key Features
* **Contactless Air-PIN Keypad:** Uses spatial distance detection across Left and Right zones to input passcode digits touchlessly.
* **Default Passcode:** `[ LEFT -> RIGHT -> LEFT ]` (or `1 - 2 - 1`).
* **Visual Keypad Feedback:** Yellow LED (Pin 13) strobes and keypad tones confirm each registered digit.
* **Automated Lockout Security:** 
  * Incorrect code triggers red flashing lights and police siren tones.
  * 3 consecutive failed attempts triggers an emergency **10-second security lockdown**.
* **Sci-Fi Unlock Fanfare:** Green LED (Pin 11) illuminates solid with an electronic victory chime upon access authorization.
* **Auto/Manual Re-lock:** Wave over the IR sensor while unlocked to instantly re-lock the vault.

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
| **Yellow LED (Key Strobe)** | (+) via 220Ω | **Pin 13** | Keypad input strobe |
| **Power Rails** | VCC / GND | `5V` / `GND` | Breadboard power rails |

---

## 🎮 How to Test & Demo

1. Open `04-air-pin-vault-lock.ino` in Arduino IDE.
2. Select **Arduino Uno** and your COM Port, then click **Upload**.
3. Open **Serial Monitor** at **115200 baud**.
4. The **Red LED** is ON (Vault is Locked).
5. **Enter Passcode:**
   * Step 1: Hover hand over **LEFT sensor** (< 15 cm) $\rightarrow$ Yellow flashes + Beep (Digit 1 recorded).
   * Step 2: Hover hand over **RIGHT sensor** (< 15 cm) $\rightarrow$ Yellow flashes + Beep (Digit 2 recorded).
   * Step 3: Hover hand over **LEFT sensor** (< 15 cm) $\rightarrow$ Yellow flashes + Beep (Digit 3 recorded).
   * Step 4: Wave hand over **IR sensor** to hit **ENTER**!
6. **Result:** 🟢 Green LED lights up + Sci-Fi chime plays + 🔓 **ACCESS GRANTED** prints!
7. **Test Intruder Alert:** Try entering the wrong code (e.g. Right, Right, Right) and hit Enter $\rightarrow$ 🚨 Siren alarm rings!
8. **Lockdown Test:** Fail 3 times in a row $\rightarrow$ 10-second security alarm lockdown!
