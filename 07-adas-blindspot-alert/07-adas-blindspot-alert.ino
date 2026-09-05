#include <Arduino.h>

/*
 * =====================================================================================
 * PROJECT 07: Vehicle ADAS Dual Blind-Spot & Driver Alert System
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * ZERO RE-WIRING (Exact same circuit as Projects 01 to 06!):
 * - Ultrasonic 1 (Left Blind-Spot Sensor)   : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Right Blind-Spot Sensor)  : TRIG = Pin 7, ECHO = Pin 6
 * - IR Sensor    (Driver Vigilance / Eyes)  : OUT  = Pin 2
 * - Piezo Buzzer (Turn Alert / Wake-Up Horn): (+)  = Pin 10
 * - Green LED    (Safe to Merge / All Clear): (+)  = Pin 11
 * - Red LED      (Right Blind-Spot Warning) : (+)  = Pin 12
 * - Yellow LED   (Left Blind-Spot Warning)  : (+)  = Pin 13
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_LEFT  = 9;
const int ECHO_LEFT  = 8;
const int TRIG_RIGHT = 7;
const int ECHO_RIGHT = 6;
const int IR_DRIVER  = 2;
const int BUZZER_PIN = 10;

const int LED_SAFE_GREEN  = 11; // Clear to change lanes
const int LED_RIGHT_RED   = 12; // Right mirror warning
const int LED_LEFT_YEL    = 13; // Left mirror warning

// --- ADAS CONFIGURATION ---
const int BLINDSPOT_THRESHOLD_CM = 20; // Blind-spot trigger zone (< 20 cm)
const int COLLISION_RISK_CM      = 10; // Critical collision distance (< 10 cm)

// Drowsiness tracking (Nod-off timer)
unsigned long eyeLossTime = 0;
bool isDriverDrowsy = false;
const unsigned long DROWSY_TIMEOUT_MS = 2500; // 2.5 seconds without driver face = Wake-up alarm!

unsigned long lastPrintTime = 0;
unsigned long lastChirpTime = 0;

long getDistance(int trigPin, int echoPin);
void playDrowsinessSiren();
void playCollisionTick();
void printADASTelemetry(bool leftBlind, bool rightBlind, bool drowsy, long distL, long distR);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 07: Vehicle ADAS Dual Blind-Spot System  "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
  pinMode(IR_DRIVER, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_SAFE_GREEN, OUTPUT);
  pinMode(LED_RIGHT_RED, OUTPUT);
  pinMode(LED_LEFT_YEL, OUTPUT);

  // Self-test animation
  digitalWrite(LED_LEFT_YEL, HIGH);   delay(100); digitalWrite(LED_LEFT_YEL, LOW);
  digitalWrite(LED_SAFE_GREEN, HIGH); delay(100); digitalWrite(LED_SAFE_GREEN, LOW);
  digitalWrite(LED_RIGHT_RED, HIGH);  delay(100); digitalWrite(LED_RIGHT_RED, LOW);
  tone(BUZZER_PIN, 1200, 100); delay(120);

  Serial.println(F("[Ready] Vehicle Safety Systems Armed. Driving Mode Active.\n"));
}

void loop() {
  // 1. Measure Left and Right Blind Spots
  long distLeft  = getDistance(TRIG_LEFT, ECHO_LEFT);
  long distRight = getDistance(TRIG_RIGHT, ECHO_RIGHT);

  bool leftBlocked  = (distLeft > 0 && distLeft <= BLINDSPOT_THRESHOLD_CM);
  bool rightBlocked = (distRight > 0 && distRight <= BLINDSPOT_THRESHOLD_CM);
  bool criticalProximity = (distLeft > 0 && distLeft <= COLLISION_RISK_CM) || 
                           (distRight > 0 && distRight <= COLLISION_RISK_CM);

  // 2. Driver Attention / Drowsiness Detection (IR Sensor)
  // Active LOW means driver face/hand is detected
  if (digitalRead(IR_DRIVER) == HIGH) {
    // Driver attention lost / head slumped
    if (eyeLossTime == 0) {
      eyeLossTime = millis();
    } else if (millis() - eyeLossTime > DROWSY_TIMEOUT_MS) {
      isDriverDrowsy = true;
    }
  } else {
    // Driver is attentive
    eyeLossTime = 0;
    isDriverDrowsy = false;
  }

  // 3. Handle Driver Drowsiness Emergency (Highest Priority!)
  if (isDriverDrowsy) {
    Serial.println(F("🚨 [DROWSINESS ALERT] DRIVER HEAD SLUMPED / ASLEEP! WAKE UP!"));
    playDrowsinessSiren();
    return;
  }

  // 4. Update Mirror Indicators (LEDs)
  digitalWrite(LED_LEFT_YEL, leftBlocked ? HIGH : LOW);
  digitalWrite(LED_RIGHT_RED, rightBlocked ? HIGH : LOW);

  if (!leftBlocked && !rightBlocked) {
    digitalWrite(LED_SAFE_GREEN, HIGH);
    noTone(BUZZER_PIN);
  } else {
    digitalWrite(LED_SAFE_GREEN, LOW);

    // Collision warning beep if critically close (< 10cm)
    if (criticalProximity && (millis() - lastChirpTime > 150)) {
      playCollisionTick();
      lastChirpTime = millis();
    }
  }

  // 5. Live Telemetry to Serial Monitor (every 1.5s)
  if (millis() - lastPrintTime > 1500) {
    printADASTelemetry(leftBlocked, rightBlocked, isDriverDrowsy, distLeft, distRight);
    lastPrintTime = millis();
  }

  delay(30);
}

// =====================================================================================
// AUDIO ALARMS
// =====================================================================================

void playCollisionTick() {
  tone(BUZZER_PIN, 2400, 30);
}

void playDrowsinessSiren() {
  // Strobe all LEDs and sound piercing anti-sleep siren
  for (int freq = 800; freq <= 2000; freq += 100) {
    tone(BUZZER_PIN, freq, 20);
    digitalWrite(LED_LEFT_YEL, HIGH);
    digitalWrite(LED_RIGHT_RED, HIGH);
    digitalWrite(LED_SAFE_GREEN, LOW);
    delay(20);
  }
  for (int freq = 2000; freq >= 800; freq -= 100) {
    tone(BUZZER_PIN, freq, 20);
    digitalWrite(LED_LEFT_YEL, LOW);
    digitalWrite(LED_RIGHT_RED, LOW);
    digitalWrite(LED_SAFE_GREEN, HIGH);
    delay(20);
  }
  noTone(BUZZER_PIN);
}

// =====================================================================================
// SERIAL TELEMETRY DASHBOARD
// =====================================================================================

void printADASTelemetry(bool leftBlind, bool rightBlind, bool drowsy, long distL, long distR) {
  Serial.println(F("--------------------------------------------------"));
  Serial.println(F("        🚗 VEHICLE ADAS SAFETY TELEMETRY          "));
  Serial.println(F("--------------------------------------------------"));
  Serial.print(F(" Left Blind Spot  : "));
  if (leftBlind) {
    Serial.print(F("⚠️  [VEHICLE IN BLIND SPOT] ("));
  } else {
    Serial.print(F("✅ [CLEAR]                 ("));
  }
  Serial.print(distL); Serial.println(F(" cm)"));

  Serial.print(F(" Right Blind Spot : "));
  if (rightBlind) {
    Serial.print(F("⚠️  [VEHICLE IN BLIND SPOT] ("));
  } else {
    Serial.print(F("✅ [CLEAR]                 ("));
  }
  Serial.print(distR); Serial.println(F(" cm)"));

  Serial.print(F(" Driver Status    : "));
  if (drowsy) {
    Serial.println(F("🚨 [DROWSY / UNRESPONSIVE - ALARM TRIGGERED]"));
  } else {
    Serial.println(F("🟢 [ATTENTIVE & AWAKE]"));
  }

  Serial.print(F(" Lane Change Safe : "));
  if (!leftBlind && !rightBlind) {
    Serial.println(F("🟢 SAFE TO CHANGE LANES (LEFT & RIGHT)"));
  } else if (!leftBlind && rightBlind) {
    Serial.println(F("🟡 SAFE TO MERGE LEFT ONLY [RIGHT BLOCKED]"));
  } else if (leftBlind && !rightBlind) {
    Serial.println(F("🟡 SAFE TO MERGE RIGHT ONLY [LEFT BLOCKED]"));
  } else {
    Serial.println(F("🔴 DO NOT CHANGE LANES [BOTH SIDES BLOCKED]"));
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
