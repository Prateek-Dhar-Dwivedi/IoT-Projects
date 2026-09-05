#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

/*
 * =====================================================================================
 * PROJECT 10: Multi-Zone Wireless Security Perimeter (ESP-NOW Network)
 * BOARD 2: Remote Sentry Node (Zone 1)
 * Microcontroller: NodeMCU (ESP-12E / ESP8266)
 * Hardware:
 *   - Ultrasonic Sensor HC-SR04:
 *       VCC  -> VIN (or VU) on NodeMCU [Direct 5V from USB! NO external supply needed]
 *       GND  -> GND on NodeMCU
 *       TRIG -> D1 (GPIO 5)
 *       ECHO -> D2 (GPIO 4)
 *   - Onboard Blue LED: D4 (GPIO 2, Active LOW - built into NodeMCU!)
 * =====================================================================================
 * 
 * 📡 REMOTE SENTRY BEHAVIOR:
 * - Operates as a completely untethered, wireless perimeter sentry.
 * - Continuously reads distance. When someone crosses within 30 cm, it instantly
 *   transmits a sub-millisecond ESP-NOW alert packet directly to the ESP32 Base Hub!
 * =====================================================================================
 */

// --- PIN DEFINITIONS (NodeMCU ESP8266) ---
const int TRIG_PIN = D1; // GPIO 5
const int ECHO_PIN = D2; // GPIO 4
const int LED_PIN  = D4; // Built-in Onboard Blue LED (Active LOW)

const int TRIPWIRE_DISTANCE_CM = 30; // Breach trigger threshold (< 30 cm)

// Broadcast address: Works with ANY listening ESP32 on Channel 1!
// (Or replace with your ESP32's specific MAC address printed on its Serial Monitor)
uint8_t masterHubAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Data Packet Structure (Must match ESP32 Base Hub exactly)
typedef struct struct_message {
  int nodeID;          // 1 = Zone 1 (Ultrasonic Sentry)
  bool intrusion;      // true if perimeter breached
  float distance;      // measured distance in cm
  char zoneName[16];   // "Zone 1 Sentry"
} struct_message;

struct_message sentData;
unsigned long lastSendTime = 0;
unsigned long lastHeartbeat = 0;

long getDistance();

// Callback when ESP-NOW packet is dispatched
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print(F("   ↳ Packet Transmission Status: "));
  if (sendStatus == 0) {
    Serial.println(F("✅ DELIVERED"));
  } else {
    Serial.println(F("ℹ️ DISPATCHED (Broadcast)"));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n========================================================"));
  Serial.println(F("  PROJECT 10: NODEMCU REMOTE SENTRY (ZONE 1 SENDER)     "));
  Serial.println(F("========================================================"));

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Default OFF (active LOW) - protects bootloader
  
  // Power-on Self Test: Blink Onboard Blue LED 2 times
  Serial.println(F("[Self-Test] Blinking Onboard Blue LED (D4)..."));
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, LOW);  // Active LOW = ON
    delay(100);
    digitalWrite(LED_PIN, HIGH); // OFF
    delay(100);
  }
  Serial.println(F("[Self-Test] LED functional. Initializing Wi-Fi radio...\n"));

  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Initialize ESP-NOW on ESP8266
  if (esp_now_init() != 0) {
    Serial.println(F("❌ Fatal Error: ESP-NOW Initialization Failed!"));
    return;
  }

  // Set ESP-NOW Role as Controller / Transmitter
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Register peer (Master Hub broadcast on Channel 1)
  // Note: Channel 1 is explicitly locked here inside the peer registration!
  esp_now_add_peer(masterHubAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  Serial.println(F("🟢 [SYSTEM ARMED] Sentry active on Channel 1. Monitoring perimeter...\n"));
}

void loop() {
  long dist = getDistance();

  // Check if perimeter is breached (< 30 cm and valid reading)
  if (dist > 0 && dist <= TRIPWIRE_DISTANCE_CM) {
    if (millis() - lastSendTime > 1200) { // Cooldown between alert packets
      Serial.print(F("🚨 [PERIMETER BREACH!] Distance: "));
      Serial.print(dist);
      Serial.println(F(" cm -> Transmitting ESP-NOW packet to Master Hub..."));

      digitalWrite(LED_PIN, LOW); // Turn ON Onboard Blue LED (Active LOW)

      // Pack data structure
      sentData.nodeID = 1;
      sentData.intrusion = true;
      sentData.distance = (float)dist;
      strcpy(sentData.zoneName, "Zone 1 Sentry");

      // Send packet wirelessly via ESP-NOW
      esp_now_send(masterHubAddress, (uint8_t *) &sentData, sizeof(sentData));

      lastSendTime = millis();
      delay(200);
      digitalWrite(LED_PIN, HIGH); // Turn OFF
    }
  }

  // Continuous Heartbeat: Prints live distance every 800ms
  // This guarantees user can SEE the sensor is alive on Serial Monitor!
  if (millis() - lastHeartbeat > 800) {
    lastHeartbeat = millis();
    Serial.print(F("[Zone 1 Live] Distance: "));
    if (dist <= 0 || dist >= 400) {
      Serial.print(F("-- (Out of Range / Clear)"));
    } else {
      Serial.print(dist);
      Serial.print(F(" cm"));
    }
    Serial.print(F(" | Status: "));
    if (dist > 0 && dist <= TRIPWIRE_DISTANCE_CM) {
      Serial.println(F("🔴 BREACH!"));
    } else {
      Serial.println(F("🟢 CLEAR"));
    }
  }

  delay(40);
  yield();
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout (~400cm)
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

