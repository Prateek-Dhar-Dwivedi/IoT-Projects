#include <Arduino.h>

/*
 * =====================================================================================
 * PROJECT 05: Smart Assistive Walking Aid (Head & Foot Detection Monitor)
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * SENSORS:
 * - Sensor 1 (Left)  : HEAD Level Detector (Trig 9, Echo 8)
 * - Sensor 2 (Right) : FOOT Level Detector (Trig 7, Echo 6)
 * - IR Sensor        : Drop-off / Pothole (Pin 2)
 * =====================================================================================
 */

const int TRIG_UPPER = 9;
const int ECHO_UPPER = 8;
const int TRIG_LOWER = 7;
const int ECHO_LOWER = 6;
const int IR_DROPOFF = 2;
const int BUZZER_PIN = 10;

const int LED_SAFE_GREEN     = 11; // Path Clear
const int LED_UPPER_RED      = 12; // Head Level Alert
const int LED_BEACON_YELLOW  = 13; // Foot Level Alert

// Detection threshold (cm)
const int HAZARD_THRESHOLD_CM = 22; 

enum AlertState { STATE_CLEAR, STATE_HEAD, STATE_FOOT, STATE_BOTH };
AlertState lastState = STATE_CLEAR;

unsigned long lastBuzzTime = 0;
long getDistance(int trigPin, int echoPin);

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

  digitalWrite(LED_SAFE_GREEN, HIGH);
  tone(BUZZER_PIN, 1000, 80); delay(100);

  Serial.println(F("[Ready] Wave hand over Sensor 1 for HEAD or Sensor 2 for FOOT!\n"));
}

void loop() {
  // 1. Check IR Sensor (Drop-off / Step-down)
  if (digitalRead(IR_DROPOFF) == LOW) {
    Serial.println(F("🚨 [POTHOLE / DROP-OFF ALERT] Step-down or cliff detected underfoot!"));
    digitalWrite(LED_SAFE_GREEN, LOW);
    digitalWrite(LED_UPPER_RED, HIGH);
    digitalWrite(LED_BEACON_YELLOW, HIGH);
    tone(BUZZER_PIN, 1500, 100); delay(120);
    tone(BUZZER_PIN, 800, 150); delay(160);
    while (digitalRead(IR_DROPOFF) == LOW);
    digitalWrite(LED_UPPER_RED, LOW);
    digitalWrite(LED_BEACON_YELLOW, LOW);
    digitalWrite(LED_SAFE_GREEN, HIGH);
    delay(200);
    return;
  }

  // 2. Read Distances from both sensors
  long distHead = getDistance(TRIG_UPPER, ECHO_UPPER);
  long distFoot = getDistance(TRIG_LOWER, ECHO_LOWER);

  bool headDetected = (distHead >= 4 && distHead <= HAZARD_THRESHOLD_CM);
  bool footDetected = (distFoot >= 4 && distFoot <= HAZARD_THRESHOLD_CM);

  // 3. Determine Current Alert State
  AlertState currentState = STATE_CLEAR;
  if (headDetected && footDetected) currentState = STATE_BOTH;
  else if (headDetected)            currentState = STATE_HEAD;
  else if (footDetected)            currentState = STATE_FOOT;
  else                              currentState = STATE_CLEAR;

  // 4. Print to Serial Monitor ONLY when the state changes
  if (currentState != lastState) {
    if (currentState == STATE_HEAD) {
      Serial.print(F("👤 ⚠️ [HEAD LEVEL ALERT] Someone at HEAD! Distance: "));
      Serial.print(distHead);
      Serial.println(F(" cm"));
    } 
    else if (currentState == STATE_FOOT) {
      Serial.print(F("🦶 ⚠️ [FOOT LEVEL ALERT] Someone at FOOT! Distance: "));
      Serial.print(distFoot);
      Serial.println(F(" cm"));
    } 
    else if (currentState == STATE_BOTH) {
      Serial.print(F("🛑 🚨 [FULL BODY ALERT] Obstacle at BOTH Head ("));
      Serial.print(distHead);
      Serial.print(F(" cm) & Foot ("));
      Serial.print(distFoot);
      Serial.println(F(" cm)!"));
    } 
    else {
      Serial.println(F("🟢 ✅ [PATH CLEAR] All Clear - Safe to Walk"));
    }
    lastState = currentState;
  }

  // 5. LED and Audio Feedback
  if (currentState != STATE_CLEAR) {
    digitalWrite(LED_SAFE_GREEN, LOW);
    digitalWrite(LED_UPPER_RED, headDetected ? HIGH : LOW);
    digitalWrite(LED_BEACON_YELLOW, footDetected ? HIGH : LOW);

    // Friendly spaced beep every 350ms (never continuous noise)
    if (millis() - lastBuzzTime > 350) {
      if (currentState == STATE_BOTH)      tone(BUZZER_PIN, 2000, 60);
      else if (currentState == STATE_HEAD) tone(BUZZER_PIN, 1800, 50); // Sharp high chirp
      else                                 tone(BUZZER_PIN, 600, 60);  // Low pitch beep
      lastBuzzTime = millis();
    }
  } else {
    // Completely silent when path is clear
    noTone(BUZZER_PIN);
    digitalWrite(LED_SAFE_GREEN, HIGH);
    digitalWrite(LED_UPPER_RED, LOW);
    digitalWrite(LED_BEACON_YELLOW, LOW);
  }

  delay(30);
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
