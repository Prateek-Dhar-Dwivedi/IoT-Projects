#include <Arduino.h>

/*
 * =====================================================================================
 * PROJECT 06: Ergonomic Posture & Screen Distance Monitor
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * ZERO RE-WIRING (Same circuit as Projects 01, 02, 03, 04 & 05!):
 * - Ultrasonic 1 (Screen-to-Face Distance) : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Torso-to-Posture Monitor): TRIG = Pin 7, ECHO = Pin 6
 * - IR Sensor    (Touchless Snooze/Calib) : OUT  = Pin 2
 * - Piezo Buzzer (Gentle Posture Reminder): (+)  = Pin 10
 * - Green LED    (Good Posture / Healthy) : (+)  = Pin 11
 * - Red LED      (Slouching / Too Close)  : (+)  = Pin 12
 * - Yellow LED   (20-20-20 Rest Alert)    : (+)  = Pin 13
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_SCREEN = 9;
const int ECHO_SCREEN = 8;
const int TRIG_TORSO  = 7;
const int ECHO_TORSO  = 6;
const int IR_SNOOZE   = 2;
const int BUZZER_PIN  = 10;

const int LED_GOOD_GREEN = 11; // Excellent posture
const int LED_WARN_RED   = 12; // Slouching / Too close
const int LED_REST_YEL   = 13; // 20-20-20 Eye break alert

// --- ERGONOMIC THRESHOLDS (cm) ---
const int MIN_SCREEN_DIST_CM  = 38; // Closer than 38 cm = Eye strain warning
const int SLOUCH_THRESHOLD_CM = 12; // Deviation from calibrated posture

// --- SYSTEM STATE ---
long currentScreenDist = 50;
long currentTorsoDist  = 40;
long calibratedTorso   = 40;
bool isCalibrated      = false;
bool tooCloseWarning   = false;
bool slouchingWarning  = false;
bool breakNeeded       = false;

unsigned long timerStart = 0;
const unsigned long BREAK_INTERVAL_MS = 60000; // 60s for demo (set to 1200000 for 20m)
unsigned long lastBuzz = 0;
unsigned long lastPrintTime = 0;

long getDistance(int trigPin, int echoPin);
void printPostureDashboard();

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 06: Ergonomic Posture & Screen Monitor   "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_SCREEN, OUTPUT);
  pinMode(ECHO_SCREEN, INPUT);
  pinMode(TRIG_TORSO, OUTPUT);
  pinMode(ECHO_TORSO, INPUT);
  pinMode(IR_SNOOZE, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GOOD_GREEN, OUTPUT);
  pinMode(LED_WARN_RED, OUTPUT);
  pinMode(LED_REST_YEL, OUTPUT);

  // Power-on test
  digitalWrite(LED_GOOD_GREEN, HIGH);
  digitalWrite(LED_WARN_RED, HIGH);
  digitalWrite(LED_REST_YEL, HIGH);
  tone(BUZZER_PIN, 1000, 100); delay(120);
  digitalWrite(LED_GOOD_GREEN, LOW);
  digitalWrite(LED_WARN_RED, LOW);
  digitalWrite(LED_REST_YEL, LOW);

  timerStart = millis();
  Serial.println(F("[Ready] Posture Monitor Active. Sit upright to auto-calibrate baseline!\n"));
}

void loop() {
  // 1. Measure Distances
  currentScreenDist = getDistance(TRIG_SCREEN, ECHO_SCREEN);
  currentTorsoDist  = getDistance(TRIG_TORSO, ECHO_TORSO);

  // Auto-calibrate upright baseline on first valid reading
  if (!isCalibrated && currentTorsoDist >= 20 && currentTorsoDist <= 80) {
    calibratedTorso = currentTorsoDist;
    isCalibrated = true;
    Serial.print(F("🎯 [Calibrated] Upright baseline set to: "));
    Serial.print(calibratedTorso); Serial.println(F(" cm"));
    tone(BUZZER_PIN, 1200, 80);
  }

  // 2. Evaluate Screen Distance (Eye strain warning)
  tooCloseWarning = (currentScreenDist > 0 && currentScreenDist < MIN_SCREEN_DIST_CM);

  // 3. Evaluate Slouching
  slouchingWarning = false;
  if (isCalibrated && currentTorsoDist > 0 && abs(currentTorsoDist - calibratedTorso) > SLOUCH_THRESHOLD_CM) {
    slouchingWarning = true;
  }

  // 4. 20-20-20 Eye Break Timer Check
  if (millis() - timerStart > BREAK_INTERVAL_MS) {
    breakNeeded = true;
    digitalWrite(LED_REST_YEL, HIGH);
  }

  // 5. Touchless Snooze / Re-calibration via IR Sensor
  if (digitalRead(IR_SNOOZE) == LOW) {
    timerStart = millis();
    breakNeeded = false;
    calibratedTorso = currentTorsoDist;
    digitalWrite(LED_REST_YEL, LOW);
    
    tone(BUZZER_PIN, 1200, 100); delay(120);
    tone(BUZZER_PIN, 1600, 150);
    Serial.println(F("✋ [Touchless Snooze] Break dismissed & posture re-calibrated!"));
    while (digitalRead(IR_SNOOZE) == LOW);
    delay(300);
  }

  // 6. Update Visual & Audio Indicators
  if (tooCloseWarning) {
    digitalWrite(LED_GOOD_GREEN, LOW);
    digitalWrite(LED_WARN_RED, HIGH);
    if (millis() - lastBuzz > 1200) {
      tone(BUZZER_PIN, 1500, 80); // High pitch for screen warning
      lastBuzz = millis();
    }
  } 
  else if (slouchingWarning) {
    digitalWrite(LED_GOOD_GREEN, LOW);
    digitalWrite(LED_WARN_RED, HIGH);
    if (millis() - lastBuzz > 1400) {
      tone(BUZZER_PIN, 800, 100); // Low pitch for slouching
      lastBuzz = millis();
    }
  } 
  else {
    // Excellent Posture!
    digitalWrite(LED_GOOD_GREEN, HIGH);
    digitalWrite(LED_WARN_RED, LOW);
    noTone(BUZZER_PIN);
  }

  // 7. Live Dashboard in Serial Monitor (every 1.5s)
  if (millis() - lastPrintTime > 1500) {
    printPostureDashboard();
    lastPrintTime = millis();
  }

  delay(40);
}

// =====================================================================================
// SERIAL DASHBOARD
// =====================================================================================
void printPostureDashboard() {
  Serial.println(F("--------------------------------------------------"));
  Serial.println(F("       🪑 ERGONOMIC POSTURE MONITOR DASHBOARD     "));
  Serial.println(F("--------------------------------------------------"));
  Serial.print(F(" Screen Distance : "));
  Serial.print(currentScreenDist);
  Serial.print(F(" cm  -> "));
  if (tooCloseWarning) Serial.println(F("⚠️  [TOO CLOSE! LEAN BACK]"));
  else                 Serial.println(F("✅ [HEALTHY DISTANCE]"));

  Serial.print(F(" Torso Distance  : "));
  Serial.print(currentTorsoDist);
  Serial.print(F(" cm (Baseline: "));
  Serial.print(calibratedTorso);
  Serial.print(F(" cm) -> "));
  if (slouchingWarning) Serial.println(F("⚠️  [SLOUCHING DETECTED]"));
  else                  Serial.println(F("✅ [UPRIGHT]"));

  Serial.print(F(" Overall Status  : "));
  if (!tooCloseWarning && !slouchingWarning) {
    Serial.println(F("🟢 EXCELLENT POSTURE"));
  } else {
    Serial.println(F("🔴 POSTURE CORRECTION NEEDED"));
  }

  Serial.print(F(" Eye Break Timer : "));
  if (breakNeeded) {
    Serial.println(F("🟡 [TIME TO REST EYES! Wave IR to Snooze]"));
  } else {
    long secLeft = (BREAK_INTERVAL_MS - (millis() - timerStart)) / 1000;
    Serial.print(secLeft); Serial.println(F("s until next 20-20-20 break"));
  }
  Serial.println(F("--------------------------------------------------\n"));
}

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 18000);
  if (duration == 0) return 999;
  return duration * 0.0343 / 2;
}
