#include <Arduino.h>

/*
 * =====================================================================================
 * PROJECT 09: Touchless "Air Gesture" Smart Desk Assistant & Media Controller
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * ZERO RE-WIRING (Same circuit as Projects 01 to 08!):
 * - Ultrasonic 1 (Left Gesture Zone)       : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Right Gesture Zone)      : TRIG = Pin 7, ECHO = Pin 6
 * - IR Sensor    (Air Click / Play-Pause)  : OUT  = Pin 2
 * - Piezo Buzzer (Audio Feedback Tones)    : (+)  = Pin 10
 * - Green LED    (Next Track / Volume UP)  : (+)  = Pin 11
 * - Red LED      (Prev Track / Volume DOWN): (+)  = Pin 12
 * - Yellow LED   (Gesture Strobe / Mute)   : (+)  = Pin 13
 * 
 * 🖐️ GESTURES SUPPORTED:
 * 1. SWIPE RIGHT (Left -> Right) : "NEXT TRACK >>"   (Green sweeps + rising chirp)
 * 2. SWIPE LEFT  (Right -> Left) : "<< PREV TRACK"   (Red sweeps + falling chirp)
 * 3. HOVER HIGH  (18cm - 30cm)   : "VOLUME UP +"     (Ascending beeps + Green LED)
 * 4. HOVER LOW   ( 5cm - 14cm)   : "VOLUME DOWN -"   (Descending beeps + Red LED)
 * 5. AIR CLICK   (IR Sensor)     : "PLAY / PAUSE"    (Yellow toggles + click sound)
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_LEFT  = 9;
const int ECHO_LEFT  = 8;
const int TRIG_RIGHT = 7;
const int ECHO_RIGHT = 6;
const int IR_CLICK   = 2;
const int BUZZER_PIN = 10;

const int LED_NEXT_GREEN = 11; // Next / Vol Up
const int LED_PREV_RED   = 12; // Prev / Vol Down
const int LED_STATE_YEL  = 13; // Play/Pause State

// Gesture threshold range (cm)
const int GESTURE_MIN_CM = 4;
const int GESTURE_MAX_CM = 25;

// Media Player Virtual State
bool isPlaying = true;
int volumeLevel = 6; // Range: 0 to 10
unsigned long lastSwipeTime = 0;
unsigned long lastHoverTime = 0;

long getDistance(int trigPin, int echoPin);
void handleSwipeRight();
void handleSwipeLeft();
void handleVolumeAdjust(long dist);
void togglePlayPause();
void printVolumeBar();

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 09: Touchless Air-Gesture Controller    "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);
  pinMode(IR_CLICK, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_NEXT_GREEN, OUTPUT);
  pinMode(LED_PREV_RED, OUTPUT);
  pinMode(LED_STATE_YEL, OUTPUT);

  // Power-on swipe test
  digitalWrite(LED_NEXT_GREEN, HIGH); delay(100); digitalWrite(LED_NEXT_GREEN, LOW);
  digitalWrite(LED_STATE_YEL, HIGH);   delay(100); digitalWrite(LED_STATE_YEL, LOW);
  digitalWrite(LED_PREV_RED, HIGH);   delay(100); digitalWrite(LED_PREV_RED, LOW);
  digitalWrite(LED_STATE_YEL, HIGH); // Default: Playing (Yellow ON)

  Serial.println(F("[Ready] Gesture Sensors Armed!"));
  Serial.println(F("-> Swipe Left-to-Right: NEXT TRACK >>"));
  Serial.println(F("-> Swipe Right-to-Left: << PREVIOUS TRACK"));
  Serial.println(F("-> Hover High/Low     : VOLUME UP / DOWN"));
  Serial.println(F("-> Wave IR Sensor     : PLAY / PAUSE\n"));
}

void loop() {
  // 1. Center Air-Click: Play/Pause Toggle via IR Sensor
  if (digitalRead(IR_CLICK) == LOW) {
    togglePlayPause();
    while (digitalRead(IR_CLICK) == LOW);
    delay(300);
    return;
  }

  // 2. Read distances
  long distLeft  = getDistance(TRIG_LEFT, ECHO_LEFT);
  long distRight = getDistance(TRIG_RIGHT, ECHO_RIGHT);

  unsigned long now = millis();

  // 3. Swipe Detection (Cooldown of 600ms between swipes)
  if (now - lastSwipeTime > 600) {
    // Check for Swipe Right (Left sensor triggered first)
    if (distLeft >= GESTURE_MIN_CM && distLeft <= GESTURE_MAX_CM) {
      unsigned long swipeStart = millis();
      while (millis() - swipeStart < 400) { // 400ms window to reach right sensor
        long dR = getDistance(TRIG_RIGHT, ECHO_RIGHT);
        if (dR >= GESTURE_MIN_CM && dR <= GESTURE_MAX_CM) {
          handleSwipeRight();
          lastSwipeTime = millis();
          return;
        }
        delay(10);
      }
    }

    // Check for Swipe Left (Right sensor triggered first)
    if (distRight >= GESTURE_MIN_CM && distRight <= GESTURE_MAX_CM) {
      unsigned long swipeStart = millis();
      while (millis() - swipeStart < 400) {
        long dL = getDistance(TRIG_LEFT, ECHO_LEFT);
        if (dL >= GESTURE_MIN_CM && dL <= GESTURE_MAX_CM) {
          handleSwipeLeft();
          lastSwipeTime = millis();
          return;
        }
        delay(10);
      }
    }
  }

  // 4. Hover Height Volume Control (If hand holds steady over Left Sensor)
  if (now - lastSwipeTime > 700 && now - lastHoverTime > 350) {
    if (distLeft >= 5 && distLeft <= 30) {
      handleVolumeAdjust(distLeft);
      lastHoverTime = millis();
    }
  }

  delay(25);
}

// =====================================================================================
// GESTURE HANDLERS
// =====================================================================================

void handleSwipeRight() {
  Serial.println(F("\n👉 [GESTURE: SWIPE RIGHT] ⏭️ NEXT TRACK >>"));
  digitalWrite(LED_NEXT_GREEN, HIGH);
  tone(BUZZER_PIN, 800, 50); delay(60);
  tone(BUZZER_PIN, 1200, 50); delay(60);
  tone(BUZZER_PIN, 1600, 80); delay(100);
  digitalWrite(LED_NEXT_GREEN, LOW);
}

void handleSwipeLeft() {
  Serial.println(F("\n👈 [GESTURE: SWIPE LEFT] ⏮️ << PREVIOUS TRACK"));
  digitalWrite(LED_PREV_RED, HIGH);
  tone(BUZZER_PIN, 1600, 50); delay(60);
  tone(BUZZER_PIN, 1200, 50); delay(60);
  tone(BUZZER_PIN, 800, 80); delay(100);
  digitalWrite(LED_PREV_RED, LOW);
}

void handleVolumeAdjust(long dist) {
  if (dist > 16 && dist <= 30) {
    // Hover High: Volume UP
    if (volumeLevel < 10) volumeLevel++;
    Serial.print(F("🔊 [HOVER HIGH: VOL +] "));
    printVolumeBar();
    digitalWrite(LED_NEXT_GREEN, HIGH);
    tone(BUZZER_PIN, 1000 + (volumeLevel * 100), 40);
    delay(50);
    digitalWrite(LED_NEXT_GREEN, LOW);
  } 
  else if (dist >= 5 && dist <= 14) {
    // Hover Low: Volume DOWN
    if (volumeLevel > 0) volumeLevel--;
    Serial.print(F("🔉 [HOVER LOW: VOL -]  "));
    printVolumeBar();
    digitalWrite(LED_PREV_RED, HIGH);
    tone(BUZZER_PIN, 1000 + (volumeLevel * 100), 40);
    delay(50);
    digitalWrite(LED_PREV_RED, LOW);
  }
}

void togglePlayPause() {
  isPlaying = !isPlaying;
  digitalWrite(LED_STATE_YEL, isPlaying ? HIGH : LOW);
  Serial.println(F("\n=================================================="));
  if (isPlaying) {
    Serial.println(F("▶️ [AIR CLICK] MEDIA PLAYING (Resume)"));
    tone(BUZZER_PIN, 1200, 70); delay(80);
    tone(BUZZER_PIN, 1500, 120);
  } else {
    Serial.println(F("⏸️ [AIR CLICK] MEDIA PAUSED"));
    tone(BUZZER_PIN, 1500, 70); delay(80);
    tone(BUZZER_PIN, 1000, 120);
  }
  Serial.println(F("==================================================\n"));
}

void printVolumeBar() {
  Serial.print(F("Volume: ["));
  for (int i = 0; i < 10; i++) {
    if (i < volumeLevel) Serial.print(F("█"));
    else                 Serial.print(F("-"));
  }
  Serial.print(F("] "));
  Serial.print(volumeLevel * 10);
  Serial.println(F("%"));
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
