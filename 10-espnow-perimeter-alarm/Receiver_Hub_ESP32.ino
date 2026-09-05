#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

/*
 * =====================================================================================
 * PROJECT 10: Multi-Zone Wireless Security Perimeter (ESP-NOW Network)
 * BOARD 1: Master Alarm Hub & Base Station
 * Microcontroller: ESP32 Dev Module
 * Hardware:
 *   - Buzzer: GPIO 25 (+) and GND (-) [Only 2 jumper wires!]
 *   - Onboard Blue LED: GPIO 2 (Built-in on ESP32 - NO extra wires needed!)
 *   - Optional External Red LED: GPIO 27 (via 220Ω resistor to GND)
 * =====================================================================================
 */

// --- PIN DEFINITIONS (ESP32 Base Station) ---
const int BUZZER_PIN       = 25; // Main Alarm Siren (+ to GPIO 25, - to GND)
const int ONBOARD_LED_PIN  = 2;  // Built-in ESP32 Blue LED (Status / Alarm)
const int EXT_LED_RED_PIN  = 27; // Optional External Red LED (Zone 1 Breach)

// Data Packet Structure (Must match NodeMCU Sender exactly)
typedef struct struct_message {
  int nodeID;          // 1 = Zone 1 (Ultrasonic Sentry)
  bool intrusion;      // true if perimeter breached
  float distance;      // measured distance in cm
  char zoneName[16];   // "Zone 1 Sentry"
} struct_message;

struct_message incomingData;
volatile bool alertPending = false;
unsigned long lastAlarmTime = 0;
unsigned long lastHeartbeat = 0;
bool alarmActive = false;

// Software sound generator: Works 100% on ANY buzzer (Active or Passive)
// and ANY ESP32 Arduino Core version (2.x or 3.x) with zero library dependencies!
void alarmBeep(int pin, int frequency, int durationMs) {
  long halfPeriodUs = 1000000L / (frequency * 2);
  long cycles = ((long)frequency * durationMs) / 1000L;
  for (long i = 0; i < cycles; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(halfPeriodUs);
    digitalWrite(pin, LOW);
    delayMicroseconds(halfPeriodUs);
  }
}

// Siren sound pattern
void soundPoliceSiren() {
  for (int freq = 900; freq <= 2000; freq += 150) {
    alarmBeep(BUZZER_PIN, freq, 15);
  }
  for (int freq = 2000; freq >= 900; freq -= 150) {
    alarmBeep(BUZZER_PIN, freq, 15);
  }
}

// ESP-NOW Receive Callback (Supports both Arduino ESP32 Core 2.x and Core 3.x)
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
#else
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingDataBytes, int len) {
#endif
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  alertPending = true; // Signal main loop to process (keeps ISR fast & safe)
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n========================================================"));
  Serial.println(F("  PROJECT 10: ESP32 MASTER ALARM HUB (ESP-NOW RECEIVER) "));
  Serial.println(F("========================================================"));

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  pinMode(EXT_LED_RED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(ONBOARD_LED_PIN, LOW);
  digitalWrite(EXT_LED_RED_PIN, LOW);

  // Power-on Self Test: Chirp buzzer and flash onboard LED
  Serial.println(F("[Self-Test] Testing Buzzer and Onboard LED..."));
  digitalWrite(ONBOARD_LED_PIN, HIGH);
  alarmBeep(BUZZER_PIN, 1800, 100);
  delay(100);
  alarmBeep(BUZZER_PIN, 2400, 150);
  digitalWrite(ONBOARD_LED_PIN, LOW);
  Serial.println(F("[Self-Test] Self-test complete! Buzzer is functional.\n"));

  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Force Wi-Fi Radio to Channel 1 (Must match ESP8266 transmitter)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  // Display Master Hub MAC address
  Serial.print(F("📡 Base Station MAC Address: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("📻 Wi-Fi Radio Channel     : "));
  Serial.println(WiFi.channel());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("❌ Fatal Error: ESP-NOW Initialization Failed!"));
    return;
  }

  // Register Receive Callback
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    esp_now_register_recv_cb(OnDataRecv);
  #else
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  #endif

  // System Armed indicator
  digitalWrite(ONBOARD_LED_PIN, HIGH);
  Serial.println(F("🟢 [SYSTEM ARMED] Base Station is listening on Channel 1...\n"));
}

void loop() {
  // Check if a wireless alert packet was received
  if (alertPending) {
    alertPending = false;

    Serial.println(F("--------------------------------------------------------"));
    Serial.print(F("🚨 [WIRELESS ALERT RECEIVED] from Node #"));
    Serial.println(incomingData.nodeID);
    Serial.print(F("   Zone Name       : ")); Serial.println(incomingData.zoneName);
    Serial.print(F("   Distance Tripped: ")); Serial.print(incomingData.distance); Serial.println(F(" cm"));
    Serial.println(F("   Alarm State     : 🔴 PERIMETER BREACH DETECTED!"));
    Serial.println(F("--------------------------------------------------------"));

    if (incomingData.intrusion) {
      alarmActive = true;
      lastAlarmTime = millis();

      // Visual Alert: Rapid strobe onboard LED + External Red LED
      for (int i = 0; i < 4; i++) {
        digitalWrite(ONBOARD_LED_PIN, HIGH);
        digitalWrite(EXT_LED_RED_PIN, HIGH);
        soundPoliceSiren();
        digitalWrite(ONBOARD_LED_PIN, LOW);
        digitalWrite(EXT_LED_RED_PIN, LOW);
        delay(60);
      }
      digitalWrite(ONBOARD_LED_PIN, HIGH); // Keep solid until cleared
      digitalWrite(EXT_LED_RED_PIN, HIGH);
    }
  }

  // Auto-clear alarm after 4 seconds of peace
  if (alarmActive && (millis() - lastAlarmTime > 4000)) {
    alarmActive = false;
    digitalWrite(EXT_LED_RED_PIN, LOW);
    digitalWrite(ONBOARD_LED_PIN, HIGH); // Re-arm (solid ON)
    Serial.println(F("🟢 Perimeter Clear. Master Hub re-armed and vigilant.\n"));
  }

  // Heartbeat status log every 3 seconds so user knows base station is alive
  if (!alarmActive && (millis() - lastHeartbeat > 3000)) {
    lastHeartbeat = millis();
    Serial.println(F("[Base Station Hub] 🟢 Armed on Channel 1 | Listening for wireless packets..."));
  }

  delay(20);
}
