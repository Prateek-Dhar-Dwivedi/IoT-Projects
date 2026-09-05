#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

/*
 * =====================================================================================
 * PROJECT 10: Multi-Zone Wireless Security Perimeter (ESP-NOW Mesh Network)
 * BOARD 2: Remote Sentry Node (Zone 1)
 * Microcontroller: NodeMCU (ESP8266)
 * Hardware: 1x Ultrasonic Sensor (HC-SR04) + 1x Alert LED
 * =====================================================================================
 * 
 * 📡 REMOTE SENTRY BEHAVIOR:
 * - Operates as an untethered, wireless perimeter tripwire.
 * - Continuously monitors distance. If someone crosses within 30 cm, it immediately
 *   transmits a high-speed ESP-NOW packet directly to the ESP32 Base Station Hub!
 * =====================================================================================
 */

// --- PIN DEFINITIONS (NodeMCU ESP8266) ---
const int TRIG_PIN = D1; // GPIO 5
const int ECHO_PIN = D2; // GPIO 4
const int LED_PIN  = D4; // Onboard LED (Active LOW)

const int TRIPWIRE_DISTANCE_CM = 30; // Breach trigger distance (< 30 cm)

// ⚠️ REPLACE WITH YOUR ESP32 MASTER HUB'S ACTUAL MAC ADDRESS!
// The MAC address is printed in the ESP32's Serial Monitor upon startup.
uint8_t masterHubAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast (works with any receiver!)

// Data Packet Structure (Matches Master Hub)
typedef struct struct_message {
  int nodeID;
  bool intrusion;
  float distance;
  char zoneName[16];
} struct_message;

struct_message sentData;
unsigned long lastSendTime = 0;

long getDistance();

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print(F("Packet Delivery Status: "));
  if (sendStatus == 0) {
    Serial.println(F("✅ SUCCESSFUL (Ack received)"));
  } else {
    Serial.println(F("❌ FAILED"));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 10: ESP-NOW Remote Sentry Node (Zone 1)  "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // OFF

  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW on ESP8266
  if (esp_now_init() != 0) {
    Serial.println(F("❌ Error initializing ESP-NOW"));
    return;
  }

  // Set ESP-NOW Role as Controller / Transmitter
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer (Master Hub)
  esp_now_add_peer(masterHubAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  Serial.println(F("[Ready] Remote Sentry Armed. Monitoring perimeter...\n"));
}

void loop() {
  long dist = getDistance();

  // Check if perimeter is breached (< 30 cm)
  if (dist > 0 && dist <= TRIPWIRE_DISTANCE_CM) {
    if (millis() - lastSendTime > 1500) { // Cooldown between transmissions
      Serial.print(F("🚨 [TRIPWIRE BREACHED!] Distance: "));
      Serial.print(dist);
      Serial.println(F(" cm -> Transmitting alert to Base Station..."));

      digitalWrite(LED_PIN, LOW); // LED ON (Active LOW)

      // Pack data
      sentData.nodeID = 1;
      sentData.intrusion = true;
      sentData.distance = (float)dist;
      strcpy(sentData.zoneName, "Perimeter Gate");

      // Send packet wirelessly via ESP-NOW
      esp_now_send(masterHubAddress, (uint8_t *) &sentData, sizeof(sentData));

      lastSendTime = millis();
      delay(200);
      digitalWrite(LED_PIN, HIGH);
    }
  }

  delay(40);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 15000);
  if (duration == 0) return 999;
  return duration * 0.0343 / 2;
}
