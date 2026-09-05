#include <Arduino.h>

/*
 * =====================================================================================
 * PROJECT 04: Sci-Fi "Air-PIN" Touchless Vault & Safe Lock
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * PASSCODE: [ LEFT -> RIGHT -> LEFT ] (1 - 2 - 1)
 * 
 * ✨ FEATURE: 5-Second Cooldown Gap between digits!
 * - After you hover over a sensor, it locks in your digit and beeps.
 * - The system gives you a clear 5-SECOND GAP (with countdown in Serial Monitor)
 *   so you have plenty of time to move your hand away and prepare the next digit!
 * - Once you input all 3 digits, wave over the IR Sensor to SUBMIT.
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_1 = 9;
const int ECHO_1 = 8;
const int TRIG_2 = 7;
const int ECHO_2 = 6;
const int IR_ENTER = 2;
const int BUZZER_PIN = 10;

const int LED_UNLOCKED_GREEN = 11; // Access Granted
const int LED_LOCKED_RED     = 12; // Locked / Intruder Alarm
const int LED_INPUT_YELLOW   = 13; // Keypad strobe / Cooldown indicator

// --- PASSCODE CONFIGURATION ---
// 1 = Left Sensor, 2 = Right Sensor
const int SECRET_PIN[3] = {1, 2, 1}; 
int enteredPin[3] = {0, 0, 0};
int digitsEntered = 0;
int failedAttempts = 0;
bool isVaultUnlocked = false;

// Cooldown timer tracking
unsigned long lastDigitTime = 0;
const unsigned long DIGIT_COOLDOWN_MS = 5000; // 5-second gap

// --- FUNCTION DECLARATIONS ---
long getDistance(int trigPin, int echoPin);
void recordDigit(int digit);
void checkPasscode();
void lockVault();
void playUnlockMelody();
void playAlarmSiren();
void triggerLockdown();

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 04: Sci-Fi Air-PIN Touchless Vault Lock  "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);
  pinMode(IR_ENTER, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_UNLOCKED_GREEN, OUTPUT);
  pinMode(LED_LOCKED_RED, OUTPUT);
  pinMode(LED_INPUT_YELLOW, OUTPUT);

  // Initial state: Vault is LOCKED
  lockVault();

  Serial.println(F("[Ready] Touchless Vault Armed & Locked."));
  Serial.println(F("-> Secret Passcode: LEFT -> RIGHT -> LEFT"));
  Serial.println(F("-> 5-second gap between digits to position your hand."));
  Serial.println(F("-> Wave hand over IR Sensor to SUBMIT code.\n"));
}

void loop() {
  // If vault is currently unlocked, allow manual re-lock via IR
  if (isVaultUnlocked) {
    if (digitalRead(IR_ENTER) == LOW) {
      Serial.println(F("[Vault] Re-locked manually via IR Sensor."));
      lockVault();
      while (digitalRead(IR_ENTER) == LOW);
      delay(300);
    }
    return;
  }

  // Check if we are still inside the 5-second cooldown gap
  if (millis() - lastDigitTime < DIGIT_COOLDOWN_MS) {
    // Flash Yellow LED slowly during the 5s cooldown
    digitalWrite(LED_INPUT_YELLOW, (millis() % 500 < 250) ? HIGH : LOW);
    delay(20);
    return; // Don't accept new sensor inputs during the 5s gap
  } else {
    digitalWrite(LED_INPUT_YELLOW, LOW);
  }

  // 1. Check Sensor 1 (Digit 1 = Left Sensor < 15cm)
  long dist1 = getDistance(TRIG_1, ECHO_1);
  if (dist1 > 0 && dist1 <= 15) {
    recordDigit(1);
    return;
  }

  // 2. Check Sensor 2 (Digit 2 = Right Sensor < 15cm)
  long dist2 = getDistance(TRIG_2, ECHO_2);
  if (dist2 > 0 && dist2 <= 15) {
    recordDigit(2);
    return;
  }

  // 3. Check IR Sensor (ENTER / SUBMIT Key)
  if (digitalRead(IR_ENTER) == LOW) {
    if (digitsEntered > 0) {
      checkPasscode();
    } else {
      Serial.println(F("[Keypad] No digits entered yet! Hover over Left/Right sensors first."));
      tone(BUZZER_PIN, 400, 100);
      delay(250);
    }
    while (digitalRead(IR_ENTER) == LOW);
    delay(300);
  }

  delay(30);
}

// =====================================================================================
// PASSCODE HANDLING
// =====================================================================================

void recordDigit(int digit) {
  if (digitsEntered < 3) {
    enteredPin[digitsEntered] = digit;
    digitsEntered++;
    lastDigitTime = millis(); // Start 5-second cooldown

    // Confirmation beep & flash
    digitalWrite(LED_INPUT_YELLOW, HIGH);
    tone(BUZZER_PIN, 1000 + (digit * 300), 120);
    delay(150);
    digitalWrite(LED_INPUT_YELLOW, LOW);

    Serial.print(F(">>> Keypad: Digit "));
    Serial.print(digitsEntered);
    Serial.print(F(" accepted ["));
    Serial.print(digit == 1 ? F("LEFT SENSOR") : F("RIGHT SENSOR"));
    Serial.print(F("] -> ("));
    Serial.print(digitsEntered);
    Serial.println(F("/3 digits entered)"));

    if (digitsEntered == 3) {
      Serial.println(F("💡 All 3 digits entered! Wave over IR SENSOR to SUBMIT code."));
    } else {
      Serial.println(F("⏳ Waiting 5 seconds before next digit can be entered..."));
    }
  } else {
    Serial.println(F("⚠️ Keypad full (3/3 digits). Wave over IR Sensor to SUBMIT!"));
    tone(BUZZER_PIN, 350, 150);
    delay(200);
  }
}

void checkPasscode() {
  Serial.println(F("\n=================================================="));
  Serial.print(F("Verifying Passcode: [ "));
  for (int i = 0; i < 3; i++) {
    Serial.print(enteredPin[i]);
    Serial.print(F(" "));
  }
  Serial.println(F("]..."));

  bool match = true;
  if (digitsEntered != 3) {
    match = false;
  } else {
    for (int i = 0; i < 3; i++) {
      if (enteredPin[i] != SECRET_PIN[i]) {
        match = false;
        break;
      }
    }
  }

  if (match) {
    // ACCESS GRANTED
    Serial.println(F("🔓 ACCESS GRANTED! Passcode Verified."));
    Serial.println(F(">>> VAULT UNLOCKED! Welcome, Commander. <<<"));
    Serial.println(F("==================================================\n"));
    
    isVaultUnlocked = true;
    failedAttempts = 0;
    digitalWrite(LED_LOCKED_RED, LOW);
    digitalWrite(LED_UNLOCKED_GREEN, HIGH);
    playUnlockMelody();
  } else {
    // ACCESS DENIED
    failedAttempts++;
    Serial.print(F("🚨 ACCESS DENIED: INVALID PASSCODE! (Attempt "));
    Serial.print(failedAttempts);
    Serial.println(F("/3)"));
    Serial.println(F("==================================================\n"));

    playAlarmSiren();

    if (failedAttempts >= 3) {
      triggerLockdown();
    } else {
      lockVault();
    }
  }

  // Reset digits buffer
  digitsEntered = 0;
  for (int i = 0; i < 3; i++) enteredPin[i] = 0;
}

void lockVault() {
  isVaultUnlocked = false;
  digitsEntered = 0;
  for (int i = 0; i < 3; i++) enteredPin[i] = 0;
  digitalWrite(LED_UNLOCKED_GREEN, LOW);
  digitalWrite(LED_INPUT_YELLOW, LOW);
  digitalWrite(LED_LOCKED_RED, HIGH); // Red means Locked
  tone(BUZZER_PIN, 400, 120); delay(140);
  tone(BUZZER_PIN, 250, 200);
}

void playUnlockMelody() {
  int notes[] = {523, 659, 784, 1046}; // C5, E5, G5, C6
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_INPUT_YELLOW, HIGH);
    tone(BUZZER_PIN, notes[i], 120);
    delay(140);
    digitalWrite(LED_INPUT_YELLOW, LOW);
  }
}

void playAlarmSiren() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_LOCKED_RED, HIGH);
    tone(BUZZER_PIN, 1500, 100);
    delay(100);
    digitalWrite(LED_LOCKED_RED, LOW);
    tone(BUZZER_PIN, 800, 100);
    delay(100);
  }
  digitalWrite(LED_LOCKED_RED, HIGH);
}

void triggerLockdown() {
  Serial.println(F("\n⛔ [SECURITY LOCKDOWN] 3 Failed Attempts! System Locked for 10s!"));
  for (int i = 10; i >= 1; i--) {
    Serial.print(F("Lockdown remaining: ")); Serial.print(i); Serial.println(F("s"));
    digitalWrite(LED_LOCKED_RED, HIGH);
    digitalWrite(LED_INPUT_YELLOW, HIGH);
    tone(BUZZER_PIN, 2200, 200);
    delay(250);
    digitalWrite(LED_LOCKED_RED, LOW);
    digitalWrite(LED_INPUT_YELLOW, LOW);
    delay(750);
  }
  Serial.println(F("Lockdown lifted. Ready for retry.\n"));
  failedAttempts = 0;
  lockVault();
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
