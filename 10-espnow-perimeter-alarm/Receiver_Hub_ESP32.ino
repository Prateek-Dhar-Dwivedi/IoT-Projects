#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

/*
 * =====================================================================================
 * PROJECT 10: Multi-Zone Wireless Security Perimeter (ESP-NOW Mesh Network)
 * BOARD 1: Master Alarm Hub & Base Station
 * Microcontroller: ESP32 Dev Module
 * Hardware: Buzzer + 3x LEDs (Zone 1 Red, Zone 2 Yellow, System Armed Green)
 * =====================================================================================
 * 
 * 📡 ESP-NOW PROTOCOL:
 * - Direct peer-to-peer wireless packet transmission (sub-millisecond latency).
 * - Operates entirely WITHOUT a Wi-Fi router or internet connection!
 * - Listens for intrusion alert packets transmitted from remote NodeMCU sentry nodes.
 * =====================================================================================
 */

// --- PIN DEFINITIONS (ESP32 Base Station) ---
const int BUZZER_PIN    = 25; // Main Alarm Siren
const int LED_ZONE1_RED = 27; // Zone 1 Intrusion Alert
const int LED_ZONE2_YEL = 33; // Zone 2 Intrusion Alert
const int LED_ARMED_GRN = 26; // System Armed / Secure

// Data Packet Structure (Must match Sender Node structure exactly)
typedef struct struct_message {
  int nodeID;          // 1 = Zone 1 (Ultrasonic), 2 = Zone 2 (IR)
  bool intrusion;      // true if perimeter tripped
  float distance;      // measured distance
  char zoneName[16];   // e.g. "Front Perimeter", "Back Door"
} struct_message;

struct_message incomingData;
unsigned long lastAlarmTime = 0;
bool alarmActive = false;

// Callback function executed when wireless data is received via ESP-NOW
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  Serial.println(F("\n=================================================="));
  Serial.print(F("🚨 [WIRELESS ALERT RECEIVED] from Node #"));
  Serial.println(incomingData.nodeID);
  Serial.println(F("=================================================="));
  Serial.print(F(" Zone Name       : ")); Serial.println(incomingData.zoneName);
  Serial.print(F(" Intrusion Status: "));
  if (incomingData.intrusion) {
    Serial.println(F("🔴 BREACH DETECTED!"));
  } else {
    Serial.println(F("🟢 ALL CLEAR"));
  }
  Serial.print(F(" Sensor Distance : ")); Serial.print(incomingData.distance); Serial.println(F(" cm"));
  Serial.println(F("==================================================\n"));

  if (incomingData.intrusion) {
    alarmActive = true;
    lastAlarmTime = millis();

    // Trigger Zone-Specific Visual Indicators
    if (incomingData.nodeID == 1) {
      digitalWrite(LED_ZONE1_RED, HIGH);
    } else if (incomingData.nodeID == 2) {
      digitalWrite(LED_ZONE2_YEL, HIGH);
    }
    digitalWrite(LED_ARMED_GRN, LOW);

    // Sound High-Urgency Police Siren
    for (int freq = 900; freq <= 2200; freq += 100) {
      tone(BUZZER_PIN, freq, 15);
      delay(15);
    }
    for (int freq = 2200; freq >= 900; freq -= 100) {
      tone(BUZZER_PIN, freq, 15);
      delay(15);
    }
    noTone(BUZZER_PIN);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 10: ESP-NOW Wireless Security Base Hub   "));
  Serial.println(F("=================================================="));

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_ZONE1_RED, OUTPUT);
  pinMode(LED_ZONE2_YEL, OUTPUT);
  pinMode(LED_ARMED_GRN, OUTPUT);

  // Startup Test
  digitalWrite(LED_ARMED_GRN, HIGH);
  tone(BUZZER_PIN, 1200, 100); delay(150);
  tone(BUZZER_PIN, 1800, 150); delay(200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Print Base Station MAC Address
  Serial.print(F("📡 Base Station MAC Address: "));
  Serial.println(WiFi.macAddress());
  Serial.println(F("💡 [Tip] Copy this MAC address and paste into your Sender Node code!\n"));

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("❌ Error initializing ESP-NOW"));
    return;
  }

  // Register Receive Callback function
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  Serial.println(F("[Ready] Master Hub Armed & Listening for Wireless Node Alerts...\n"));
}

void loop() {
  // Auto-clear alarm after 5 seconds of peace
  if (alarmActive && (millis() - lastAlarmTime > 5000)) {
    alarmActive = false;
    digitalWrite(LED_ZONE1_RED, LOW);
    digitalWrite(LED_ZONE2_YEL, LOW);
    digitalWrite(LED_ARMED_GRN, HIGH);
    Serial.println(F("🟢 Perimeter Secure. System re-armed.\n"));
  }

  delay(50);
}
