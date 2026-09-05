#include <Arduino.h>

/*
 * =====================================================================================
 * PROJECT 08: Industrial Assembly Line & Quality Control (QC) Sizing Inspector
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * ZERO RE-WIRING (Same circuit as Projects 01 to 07!):
 * - IR Sensor    (Conveyor Item Arrival Detector) : OUT  = Pin 2
 * - Ultrasonic 1 (Product Height Profiler)        : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Product Width/Depth Profiler)   : TRIG = Pin 7, ECHO = Pin 6
 * - Piezo Buzzer (QC Pass Chime / Reject Alarm)   : (+)  = Pin 10
 * - Green LED    (QC PASSED / Quality Approved)   : (+)  = Pin 11
 * - Red LED      (QC REJECTED / Defective Part)   : (+)  = Pin 12
 * - Yellow LED   (Inspection Strobe / In Progress): (+)  = Pin 13
 * 
 * 🏭 HOW IT WORKS:
 * 1. An item (box, pen, part) rolls down the conveyor and trips the IR Sensor.
 * 2. Yellow LED turns ON as the dual ultrasonic sensors scan the item's dimensions.
 * 3. TOLERANCE CHECK (Allowed: 7.0 cm to 15.0 cm):
 *    - IN SPEC  -> 🟢 Green LED pulses + Happy 2-tone chime + Logged as PASSED!
 *    - DEFECTIVE-> 🔴 Red LED latches + Loud Reject buzzer alarm + Defect logged!
 * 4. Serial Monitor logs live factory metrics (Total Inspected, Pass %, Rejects).
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int IR_CONVEYOR  = 2;
const int TRIG_HEIGHT  = 9;
const int ECHO_HEIGHT  = 8;
const int TRIG_WIDTH   = 7;
const int ECHO_WIDTH   = 6;
const int BUZZER_PIN   = 10;

const int LED_PASS_GREEN = 11; // Passed QC
const int LED_FAIL_RED   = 12; // Defective part
const int LED_SCAN_YEL   = 13; // Inspection active

// --- QC SPECIFICATION TOLERANCES (cm) ---
const int MIN_ALLOWABLE_CM = 6;  // Minimum allowable dimension
const int MAX_ALLOWABLE_CM = 15; // Maximum allowable dimension

// Factory Production Counters
int totalInspected = 0;
int totalPassed    = 0;
int totalRejected  = 0;

long getDistance(int trigPin, int echoPin);
void inspectProduct();
void playPassChime();
void playRejectAlarm();
void printFactoryMetrics(bool passed, long h, long w);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 08: Industrial QC & Dimension Inspector  "));
  Serial.println(F("=================================================="));

  pinMode(IR_CONVEYOR, INPUT_PULLUP);
  pinMode(TRIG_HEIGHT, OUTPUT);
  pinMode(ECHO_HEIGHT, INPUT);
  pinMode(TRIG_WIDTH, OUTPUT);
  pinMode(ECHO_WIDTH, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PASS_GREEN, OUTPUT);
  pinMode(LED_FAIL_RED, OUTPUT);
  pinMode(LED_SCAN_YEL, OUTPUT);

  // Power-on calibration
  digitalWrite(LED_PASS_GREEN, HIGH);
  digitalWrite(LED_SCAN_YEL, HIGH);
  digitalWrite(LED_FAIL_RED, HIGH);
  tone(BUZZER_PIN, 1200, 100); delay(120);
  digitalWrite(LED_PASS_GREEN, LOW);
  digitalWrite(LED_SCAN_YEL, LOW);
  digitalWrite(LED_FAIL_RED, LOW);

  Serial.println(F("[Ready] Quality Control Inspection Station Armed."));
  Serial.println(F("-> Slide product across IR Sensor to trigger dimensional inspection.\n"));
}

void loop() {
  // Check if a product arrives at the optical inspection gate (IR Sensor)
  if (digitalRead(IR_CONVEYOR) == LOW) {
    inspectProduct();
    // Wait until the product clears the station
    while (digitalRead(IR_CONVEYOR) == LOW);
    delay(400); // Debounce
  }

  delay(30);
}

// =====================================================================================
// INSPECTION LOGIC
// =====================================================================================

void inspectProduct() {
  totalInspected++;

  // Visual scan in progress
  digitalWrite(LED_PASS_GREEN, LOW);
  digitalWrite(LED_FAIL_RED, LOW);
  digitalWrite(LED_SCAN_YEL, HIGH);

  tone(BUZZER_PIN, 2000, 40); // Scan beep
  delay(150);

  // Measure Height and Width
  long measuredHeight = getDistance(TRIG_HEIGHT, ECHO_HEIGHT);
  long measuredWidth  = getDistance(TRIG_WIDTH, ECHO_WIDTH);

  // Validate Dimensions against Specs
  bool heightValid = (measuredHeight >= MIN_ALLOWABLE_CM && measuredHeight <= MAX_ALLOWABLE_CM);
  bool widthValid  = (measuredWidth  >= MIN_ALLOWABLE_CM && measuredWidth  <= MAX_ALLOWABLE_CM);

  digitalWrite(LED_SCAN_YEL, LOW);

  if (heightValid && widthValid) {
    // QC PASSED!
    totalPassed++;
    digitalWrite(LED_PASS_GREEN, HIGH);
    digitalWrite(LED_FAIL_RED, LOW);
    printFactoryMetrics(true, measuredHeight, measuredWidth);
    playPassChime();
    delay(800);
    digitalWrite(LED_PASS_GREEN, LOW);
  } else {
    // QC REJECTED (DEFECT)
    totalRejected++;
    digitalWrite(LED_PASS_GREEN, LOW);
    digitalWrite(LED_FAIL_RED, HIGH);
    printFactoryMetrics(false, measuredHeight, measuredWidth);
    playRejectAlarm();
    delay(800);
    digitalWrite(LED_FAIL_RED, LOW);
  }
}

// =====================================================================================
// AUDIO FEEDBACK
// =====================================================================================

void playPassChime() {
  tone(BUZZER_PIN, 1046, 80); delay(90);
  tone(BUZZER_PIN, 1318, 120); delay(140);
}

void playRejectAlarm() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 400, 80);
    delay(100);
  }
}

// =====================================================================================
// FACTORY PRODUCTION METRICS (SERIAL MONITOR)
// =====================================================================================

void printFactoryMetrics(bool passed, long h, long w) {
  float passRate = ((float)totalPassed / totalInspected) * 100.0;
  float defectRate = ((float)totalRejected / totalInspected) * 100.0;

  Serial.println(F("=================================================="));
  Serial.print(F("📦 INSPECTION REPORT: PART #")); Serial.println(totalInspected);
  Serial.println(F("=================================================="));
  Serial.print(F(" Measured Height : ")); Serial.print(h); 
  Serial.println((h >= MIN_ALLOWABLE_CM && h <= MAX_ALLOWABLE_CM) ? F(" cm [IN SPEC]") : F(" cm [OUT OF SPEC ❌]"));

  Serial.print(F(" Measured Width  : ")); Serial.print(w);
  Serial.println((w >= MIN_ALLOWABLE_CM && w <= MAX_ALLOWABLE_CM) ? F(" cm [IN SPEC]") : F(" cm [OUT OF SPEC ❌]"));

  Serial.print(F(" Decision        : "));
  if (passed) {
    Serial.println(F("🟢 PASSED -> APPROVED FOR SHIPPING"));
  } else {
    Serial.println(F("🔴 REJECTED -> DEFECT EJECTED"));
  }

  Serial.println(F("--------------------------------------------------"));
  Serial.print(F(" Total Inspected : ")); Serial.println(totalInspected);
  Serial.print(F(" Yield (Pass %)  : ")); Serial.print(passRate, 1); Serial.println(F(" %"));
  Serial.print(F(" Defect Rate     : ")); Serial.print(defectRate, 1); Serial.println(F(" %"));
  Serial.println(F("==================================================\n"));
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
