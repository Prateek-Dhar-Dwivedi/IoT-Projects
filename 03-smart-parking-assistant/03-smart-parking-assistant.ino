/*
 * =====================================================================================
 * PROJECT 03: Dual-Bay Smart Parking Lot & Reverse Proximity Warner
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * ZERO RE-WIRING (Same circuit as Project 01 & 02!):
 * - Ultrasonic 1 (Parking Bay A)        : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Parking Bay B)        : TRIG = Pin 7, ECHO = Pin 6
 * - IR Sensor    (Entry Gate Barrier)   : OUT  = Pin 2
 * - Piezo Buzzer (Reverse Warner/Gate)  : (+)  = Pin 10
 * - Green LED    (Lot Vacant / Slots OK): (+)  = Pin 11
 * - Red LED      (Lot FULL / Stop Alert): (+)  = Pin 12
 * - Yellow LED   (Gate Open / Warning)  : (+)  = Pin 13
 * 
 * 🚗 HOW IT WORKS:
 * 1. SLOT MONITORING:
 *    - Ultrasonic 1 monitors Bay A (< 12cm = Occupied).
 *    - Ultrasonic 2 monitors Bay B (< 12cm = Occupied).
 *    - 2 Slots Free -> GREEN LED ON ("VACANT")
 *    - 1 Slot Free  -> YELLOW LED ON ("1 SLOT LEFT")
 *    - 0 Slots Free -> RED LED ON ("LOT FULL")
 * 
 * 2. ENTRY GATE (IR Sensor):
 *    - Car arrives at gate -> Welcome chime & Yellow LED flashes!
 *    - If LOT FULL -> Warning buzz & Red LED flashes (Entry Denied).
 * 
 * 3. REVERSE PARKING ASSISTANT (Buzzer):
 *    - When parking a car into a bay:
 *      * 12cm - 20cm: Slow beep (approaching curb)
 *      *  6cm - 12cm: Fast rapid beep (getting close)
 *      *   < 6cm    : Continuous SOLID tone (STOP! Collision risk!)
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_A = 9;
const int ECHO_A = 8;
const int TRIG_B = 7;
const int ECHO_B = 6;
const int IR_GATE = 2;
const int BUZZER_PIN = 10;

const int LED_VACANT_GREEN = 11; // Slots available
const int LED_FULL_RED     = 12; // Lot full / Stop
const int LED_WARN_YELLOW  = 13; // Gate open / Caution

// --- PARKING THRESHOLDS (cm) ---
const int OCCUPIED_THRESHOLD_CM = 12; // Dist < 12cm means bay is occupied
const int CRITICAL_STOP_CM      = 6;  // Dist < 6cm = Emergency STOP alarm

// Timing & state tracking
unsigned long lastDashboardPrint = 0;
bool gateWasTriggered = false;

long getDistance(int trigPin, int echoPin);
void handleReverseAssist(long distA, long distB);
void handleEntryGate(int freeSlots);
void printDashboard(bool occA, bool occB, long distA, long distB, int freeSlots);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 03: Dual-Bay Smart Parking Lot System    "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_A, OUTPUT);
  pinMode(ECHO_A, INPUT);
  pinMode(TRIG_B, OUTPUT);
  pinMode(ECHO_B, INPUT);
  pinMode(IR_GATE, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_VACANT_GREEN, OUTPUT);
  pinMode(LED_FULL_RED, OUTPUT);
  pinMode(LED_WARN_YELLOW, OUTPUT);

  // Power-on sequence
  digitalWrite(LED_VACANT_GREEN, HIGH);
  digitalWrite(LED_WARN_YELLOW, HIGH);
  digitalWrite(LED_FULL_RED, HIGH);
  tone(BUZZER_PIN, 800, 100); delay(120);
  tone(BUZZER_PIN, 1200, 150); delay(180);
  digitalWrite(LED_VACANT_GREEN, LOW);
  digitalWrite(LED_WARN_YELLOW, LOW);
  digitalWrite(LED_FULL_RED, LOW);

  Serial.println(F("[Ready] Parking Lot Sensors Active.\n"));
}

void loop() {
  // 1. Measure Bay Distances
  long distA = getDistance(TRIG_A, ECHO_A);
  long distB = getDistance(TRIG_B, ECHO_B);

  // Determine occupancy
  bool bayAOccupied = (distA > 0 && distA <= OCCUPIED_THRESHOLD_CM);
  bool bayBOccupied = (distB > 0 && distB <= OCCUPIED_THRESHOLD_CM);

  int freeSlots = 2 - (bayAOccupied ? 1 : 0) - (bayBOccupied ? 1 : 0);

  // 2. Capacity LED Status
  if (freeSlots == 2) {
    // Completely Free -> Green ON
    digitalWrite(LED_VACANT_GREEN, HIGH);
    digitalWrite(LED_WARN_YELLOW, LOW);
    digitalWrite(LED_FULL_RED, LOW);
  } else if (freeSlots == 1) {
    // 1 Slot Free -> Yellow ON
    digitalWrite(LED_VACANT_GREEN, LOW);
    digitalWrite(LED_WARN_YELLOW, HIGH);
    digitalWrite(LED_FULL_RED, LOW);
  } else {
    // Lot Full -> Red ON
    digitalWrite(LED_VACANT_GREEN, LOW);
    digitalWrite(LED_WARN_YELLOW, LOW);
    digitalWrite(LED_FULL_RED, HIGH);
  }

  // 3. Entry Gate Trigger (IR Sensor)
  if (digitalRead(IR_GATE) == LOW) {
    handleEntryGate(freeSlots);
  }

  // 4. Reverse Parking Proximity Audio Assistance
  handleReverseAssist(distA, distB);

  // 5. Live Dashboard to Serial Monitor (every 1.5 seconds)
  if (millis() - lastDashboardPrint > 1500) {
    printDashboard(bayAOccupied, bayBOccupied, distA, distB, freeSlots);
    lastDashboardPrint = millis();
  }

  delay(40);
}

// =====================================================================================
// ENTRY GATE LOGIC (IR SENSOR)
// =====================================================================================
void handleEntryGate(int freeSlots) {
  if (freeSlots > 0) {
    // Welcome car into parking lot
    Serial.println(F("\n>>> [ENTRY GATE] Car Detected! GATE OPEN -> WELCOME! <<<"));
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_WARN_YELLOW, HIGH);
      tone(BUZZER_PIN, 1200 + (i * 200), 80);
      delay(100);
      digitalWrite(LED_WARN_YELLOW, LOW);
      delay(50);
    }
  } else {
    // Parking lot is full!
    Serial.println(F("\n>>> [ENTRY GATE] ACCESS DENIED: PARKING LOT IS FULL! <<<"));
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_FULL_RED, HIGH);
      tone(BUZZER_PIN, 250, 200);
      delay(220);
      digitalWrite(LED_FULL_RED, LOW);
      delay(80);
    }
  }
  while (digitalRead(IR_GATE) == LOW); // Wait for vehicle to clear gate
  delay(300);
}

// =====================================================================================
// REVERSE PARKING PROXIMITY BEEP LOGIC
// =====================================================================================
void handleReverseAssist(long distA, long distB) {
  // Find the closest object in either bay
  long minDistance = min(distA, distB);

  if (minDistance > 0 && minDistance < CRITICAL_STOP_CM) {
    // Critical Proximity (< 6cm): SOLID CONTINUOUS TONE!
    tone(BUZZER_PIN, 2000); // Continuous tone
    digitalWrite(LED_FULL_RED, HIGH);
  } 
  else if (minDistance >= CRITICAL_STOP_CM && minDistance <= 12) {
    // Very Close (6 - 12cm): Rapid Beep
    tone(BUZZER_PIN, 1600, 40);
    delay(40);
    noTone(BUZZER_PIN);
    delay(40);
  } 
  else if (minDistance > 12 && minDistance <= 22) {
    // Approaching (12 - 22cm): Slow Beep
    tone(BUZZER_PIN, 1000, 60);
    delay(60);
    noTone(BUZZER_PIN);
    delay(180);
  } 
  else {
    // Far away / clear: Silence
    noTone(BUZZER_PIN);
  }
}

// =====================================================================================
// SERIAL MONITOR DASHBOARD
// =====================================================================================
void printDashboard(bool occA, bool occB, long distA, long distB, int freeSlots) {
  Serial.println(F("--------------------------------------------------"));
  Serial.println(F("        🅿️ SMART PARKING LOT DASHBOARD            "));
  Serial.println(F("--------------------------------------------------"));
  Serial.print(F(" Bay A [Slot 1] : "));
  if (occA) {
    Serial.print(F("[🔴 OCCUPIED]  (Dist: "));
  } else {
    Serial.print(F("[🟢 VACANT]    (Dist: "));
  }
  Serial.print(distA); Serial.println(F(" cm)"));

  Serial.print(F(" Bay B [Slot 2] : "));
  if (occB) {
    Serial.print(F("[🔴 OCCUPIED]  (Dist: "));
  } else {
    Serial.print(F("[🟢 VACANT]    (Dist: "));
  }
  Serial.print(distB); Serial.println(F(" cm)"));

  Serial.print(F(" Capacity       : "));
  Serial.print(freeSlots);
  Serial.print(F(" / 2 Free  -> "));
  if (freeSlots == 2)      Serial.println(F("[🟢 ALL SLOTS VACANT]"));
  else if (freeSlots == 1) Serial.println(F("[🟡 1 SLOT REMAINING]"));
  else                     Serial.println(F("[🔴 FULL - NO VACANCY]"));
  Serial.println(F("--------------------------------------------------\n"));
}

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 20000); // 20ms timeout
  if (duration == 0) return 999;
  return duration * 0.0343 / 2;
}
