#include <Arduino.h>

/*
 * =====================================================================================
 * PROJECT 05: Smart Assistive Walking Aid (Dual-Elevation Hazard Detection)
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * ZERO RE-WIRING (Same circuit as Projects 01, 02, 03 & 04!):
 * - Ultrasonic 1 (Upper Hazard / Head & Chest) : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Lower Hazard / Ground & Knee): TRIG = Pin 7, ECHO = Pin 6
 * - IR Sensor    (Curb / Drop-off / Pothole)   : OUT  = Pin 2
 * - Piezo Buzzer (Differentiated Audio Cues)   : (+)  = Pin 10
 * - Green LED    (Path Clear / Safe to Walk)   : (+)  = Pin 11
 * - Red LED      (Upper Collision Warning)     : (+)  = Pin 12
 * - Yellow LED   (Ground Hazard / Night Beacon): (+)  = Pin 13
 * 
 * 🦯 HOW IT WORKS:
 * 1. UPPER HAZARD (Head / Chest Level - Ultrasonic 1):
 *    - Distance < 25 cm -> High-Pitch Sharp Beep (1800 Hz) + RED LED ON!
 * 2. LOWER HAZARD (Knee / Ground Level - Ultrasonic 2):
 *    - Distance < 25 cm -> Low-Pitch Pulsing Tone (600 Hz) + YELLOW LED ON!
 * 3. FALL / DROP-OFF HAZARD (Staircase / Pothole - IR Sensor):
 *    - IR sensor trips (optical barrier) -> Emergency Fall Alarm Siren!
 * 4. ALL CLEAR:
 *    - GREEN LED stays ON + Yellow pulses gently as a night visibility beacon for cars.
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_UPPER = 9;
const int ECHO_UPPER = 8;
const int TRIG_LOWER = 7;
const int ECHO_LOWER = 6;
const int IR_DROPOFF = 2;
const int BUZZER_PIN = 10;

const int LED_SAFE_GREEN     = 11; // All clear
const int LED_UPPER_RED      = 12; // Head / Chest obstacle
const int LED_BEACON_YELLOW  = 13; // Ground hazard & Night safety beacon

// --- SENSITIVITY THRESHOLDS (cm) ---
const int UPPER_HAZARD_CM = 25; // Detect overhanging obstacles < 25cm
const int LOWER_HAZARD_CM = 25; // Detect ground obstacles < 25cm

// Timing tracking
unsigned long lastPrintTime = 0;
unsigned long lastBeaconBlink = 0;
bool beaconState = false;

long getDistance(int trigPin, int echoPin);
void playUpperAlarm();
void playLowerAlarm();
void playDropoffAlarm();
void printAssistDashboard(bool upperHazard, bool lowerHazard, bool dropoffHazard, long distUp, long distLow);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 05: Smart Assistive Walking Aid System   "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_UPPER, OUTPUT);
  pinMode(ECHO_UPPER, INPUT);
  pinMode(TRIG_LOWER, OUTPUT);
  pinMode(ECHO_LOWER, INPUT);
  pinMode(IR_DROPOFF, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_SAFE_GREEN, OUTPUT);
  pinMode(LED_UPPER_RED, OUTPUT);
  pinMode(LED_BEACON_YELLOW, OUTPUT);

  // Power-on safety check
  digitalWrite(LED_SAFE_GREEN, HIGH);
  digitalWrite(LED_UPPER_RED, HIGH);
  digitalWrite(LED_BEACON_YELLOW, HIGH);
  tone(BUZZER_PIN, 800, 100); delay(120);
  tone(BUZZER_PIN, 1400, 150); delay(180);
  digitalWrite(LED_SAFE_GREEN, LOW);
  digitalWrite(LED_UPPER_RED, LOW);
  digitalWrite(LED_BEACON_YELLOW, LOW);

  Serial.println(F("[Ready] Assistive Sensors Armed. Walking Aid Active.\n"));
}

void loop() {
  // 1. Measure Dual-Elevation Distances
  long distUpper = getDistance(TRIG_UPPER, ECHO_UPPER);
  long distLower = getDistance(TRIG_LOWER, ECHO_LOWER);

  bool upperHazard = (distUpper > 0 && distUpper <= UPPER_HAZARD_CM);
  bool lowerHazard = (distLower > 0 && distLower <= LOWER_HAZARD_CM);
  bool dropoffHazard = (digitalRead(IR_DROPOFF) == LOW);

  // 2. Evaluate Hazards by Priority
  if (dropoffHazard) {
    // Highest Priority: Sudden drop-off / staircase / manhole!
    digitalWrite(LED_SAFE_GREEN, LOW);
    digitalWrite(LED_UPPER_RED, HIGH);
    digitalWrite(LED_BEACON_YELLOW, HIGH);
    playDropoffAlarm();
  } 
  else if (upperHazard && lowerHazard) {
    // Critical: Full-body wall or massive obstacle directly in front!
    digitalWrite(LED_SAFE_GREEN, LOW);
    digitalWrite(LED_UPPER_RED, HIGH);
    digitalWrite(LED_BEACON_YELLOW, HIGH);
    tone(BUZZER_PIN, 2200, 80);
    delay(100);
  }
  else if (upperHazard) {
    // Upper body / head-level obstacle (overhanging branch, signboard)
    digitalWrite(LED_SAFE_GREEN, LOW);
    digitalWrite(LED_UPPER_RED, HIGH);
    digitalWrite(LED_BEACON_YELLOW, LOW);
    playUpperAlarm();
  }
  else if (lowerHazard) {
    // Lower body / knee-level obstacle (hydrant, boulder, curb)
    digitalWrite(LED_SAFE_GREEN, LOW);
    digitalWrite(LED_UPPER_RED, LOW);
    digitalWrite(LED_BEACON_YELLOW, HIGH);
    playLowerAlarm();
  }
  else {
    // PATH IS CLEAR! Safe to walk
    noTone(BUZZER_PIN);
    digitalWrite(LED_SAFE_GREEN, HIGH);
    digitalWrite(LED_UPPER_RED, LOW);

    // Night Beacon Blink (gentle pulse every 800ms)
    if (millis() - lastBeaconBlink > 800) {
      beaconState = !beaconState;
      digitalWrite(LED_BEACON_YELLOW, beaconState ? HIGH : LOW);
      lastBeaconBlink = millis();
    }
  }

  // 3. Live Assistive Dashboard (every 1.5s)
  if (millis() - lastPrintTime > 1500) {
    printAssistDashboard(upperHazard, lowerHazard, dropoffHazard, distUpper, distLower);
    lastPrintTime = millis();
  }

  delay(30);
}

// =====================================================================================
// AUDIO CUES & ALARMS
// =====================================================================================

void playUpperAlarm() {
  // Sharp high-pitch chirp for head/chest hazards
  tone(BUZZER_PIN, 1800, 60);
  delay(80);
  noTone(BUZZER_PIN);
  delay(40);
}

void playLowerAlarm() {
  // Low-pitch pulse for ground/knee obstacles
  tone(BUZZER_PIN, 600, 80);
  delay(100);
  noTone(BUZZER_PIN);
  delay(80);
}

void playDropoffAlarm() {
  // Alternating siren for edge / cliff / drop-off
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 1500, 60); delay(70);
    tone(BUZZER_PIN, 700, 60); delay(70);
  }
}

// =====================================================================================
// SERIAL MONITOR DASHBOARD
// =====================================================================================

void printAssistDashboard(bool upperHazard, bool lowerHazard, bool dropoffHazard, long distUp, long distLow) {
  Serial.println(F("--------------------------------------------------"));
  Serial.println(F("     🦯 SMART ASSISTIVE WALKING AID MONITOR       "));
  Serial.println(F("--------------------------------------------------"));
  Serial.print(F(" Upper (Chest/Head) : "));
  if (upperHazard) {
    Serial.print(F("⚠️  [OBSTACLE DETECTED] ("));
  } else {
    Serial.print(F("✅ [PATH CLEAR]       ("));
  }
  Serial.print(distUp); Serial.println(F(" cm)"));

  Serial.print(F(" Lower (Knee/Ground): "));
  if (lowerHazard) {
    Serial.print(F("⚠️  [OBSTACLE DETECTED] ("));
  } else {
    Serial.print(F("✅ [PATH CLEAR]       ("));
  }
  Serial.print(distLow); Serial.println(F(" cm)"));

  Serial.print(F(" Drop-off / Pit     : "));
  if (dropoffHazard) {
    Serial.println(F("🚨 [FALL HAZARD / CURB DETECTED!]"));
  } else {
    Serial.println(F("✅ [GROUND SURFACE SAFE]"));
  }

  Serial.print(F(" Navigation Status  : "));
  if (!upperHazard && !lowerHazard && !dropoffHazard) {
    Serial.println(F("🟢 SAFE TO PROCEED"));
  } else {
    Serial.println(F("🛑 CAUTION: HAZARD IN PATH"));
  }
  Serial.println(F("--------------------------------------------------\n"));
}

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 15000); // 15ms timeout
  if (duration == 0) return 999;
  return duration * 0.0343 / 2;
}
