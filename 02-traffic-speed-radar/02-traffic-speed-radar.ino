/*
 * =====================================================================================
 * PROJECT 02: High-Speed Traffic Radar & Automated Speed Camera Trap
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * ZERO RE-WIRING NEEDED (Same as Project 01!):
 * - Ultrasonic 1 (Entry Gate / Trap A) : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Exit Gate  / Trap B) : TRIG = Pin 7, ECHO = Pin 6
 * - IR Sensor    (Stop-Line Camera)    : OUT  = Pin 2
 * - Buzzer       (Camera Shutter/Alarm): (+)  = Pin 10
 * - Green LED    (Safe Speed OK)       : (+)  = Pin 11
 * - Red LED      (Speed Violation / Cit): (+) = Pin 12
 * - Yellow LED   (Camera Strobe Flash) : (+)  = Pin 13
 * 
 * SETUP ON TABLE:
 * Place Sensor 1 and Sensor 2 facing across your desk, separated by 10 cm.
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_1 = 9;
const int ECHO_1 = 8;
const int TRIG_2 = 7;
const int ECHO_2 = 6;
const int IR_PIN = 2;
const int BUZZER_PIN = 10;

const int LED_SAFE_GREEN  = 11; // Safe speed
const int LED_VIOL_RED    = 12; // Speed / Red light violation
const int LED_FLASH_YEL   = 13; // Speed camera flash strobe

// --- RADAR CONFIGURATION ---
const float SENSOR_DISTANCE_CM = 10.0; // Distance between Sensor 1 & Sensor 2 in cm
const float SPEED_LIMIT_KMH    = 45.0; // Speed limit threshold in simulated km/h
const int   DETECTION_RANGE_CM = 18;   // Maximum distance to detect vehicle (cm)

// Function declarations
long getDistance(int trigPin, int echoPin);
void triggerCameraFlash();
void playViolationSiren();
void playSafeChirp();
void printTicket(float speedKmh, unsigned long transitMs, bool isViolation);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 02: High-Speed Radar & Speed Camera Trap "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);
  pinMode(IR_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_SAFE_GREEN, OUTPUT);
  pinMode(LED_VIOL_RED, OUTPUT);
  pinMode(LED_FLASH_YEL, OUTPUT);

  // Startup Calibration Test
  digitalWrite(LED_SAFE_GREEN, HIGH);
  digitalWrite(LED_VIOL_RED, HIGH);
  digitalWrite(LED_FLASH_YEL, HIGH);
  tone(BUZZER_PIN, 1000, 100); delay(120);
  tone(BUZZER_PIN, 1500, 150); delay(180);
  digitalWrite(LED_SAFE_GREEN, LOW);
  digitalWrite(LED_VIOL_RED, LOW);
  digitalWrite(LED_FLASH_YEL, LOW);

  Serial.println(F("[Ready] Speed Trap Armed!"));
  Serial.print(F("-> Sensor Spacing : ")); Serial.print(SENSOR_DISTANCE_CM); Serial.println(F(" cm"));
  Serial.print(F("-> Speed Limit    : ")); Serial.print(SPEED_LIMIT_KMH); Serial.println(F(" km/h"));
  Serial.println(F("-> Slide object past Sensor 1 then Sensor 2 to measure speed.\n"));
}

void loop() {
  // 1. Check for Stop-Line / Red Light Violation via IR Sensor
  if (digitalRead(IR_PIN) == LOW) {
    Serial.println(F("\n🚨 [RED LIGHT VIOLATION] Vehicle crossed intersection stop line!"));
    digitalWrite(LED_VIOL_RED, HIGH);
    tone(BUZZER_PIN, 1800, 400); delay(450);
    digitalWrite(LED_VIOL_RED, LOW);
    while (digitalRead(IR_PIN) == LOW); // Wait until clear
    delay(200);
    return;
  }

  // 2. Poll Sensor 1 (Entry Gate)
  long dist1 = getDistance(TRIG_1, ECHO_1);

  if (dist1 > 0 && dist1 <= DETECTION_RANGE_CM) {
    unsigned long t1 = micros(); // Entry timestamp in microseconds
    digitalWrite(LED_FLASH_YEL, HIGH); // Armed indicator
    
    // Wait for vehicle to reach Sensor 2 (timeout after 2.5 seconds)
    bool captured = false;
    unsigned long timeoutStart = millis();

    while (millis() - timeoutStart < 2500) {
      long dist2 = getDistance(TRIG_2, ECHO_2);

      if (dist2 > 0 && dist2 <= DETECTION_RANGE_CM) {
        unsigned long t2 = micros(); // Exit timestamp
        captured = true;
        digitalWrite(LED_FLASH_YEL, LOW);

        // Calculate time elapsed
        unsigned long deltaMicros = t2 - t1;
        float transitSeconds = (float)deltaMicros / 1000000.0;
        unsigned long transitMs = deltaMicros / 1000;

        // Speed in cm/s = distance_cm / seconds
        float speedCmPerSec = SENSOR_DISTANCE_CM / transitSeconds;

        // Scale to simulated km/h (1 cm/s in table-scale ~ 0.5 km/h simulated)
        // E.g., 10 cm in 0.10s = 100 cm/s = 50 km/h
        float speedKmh = speedCmPerSec * 0.5;

        // Speed Evaluation
        if (speedKmh > SPEED_LIMIT_KMH) {
          // OVERSPEED VIOLATION!
          triggerCameraFlash();
          printTicket(speedKmh, transitMs, true);
          playViolationSiren();
        } else {
          // SAFE SPEED
          digitalWrite(LED_SAFE_GREEN, HIGH);
          printTicket(speedKmh, transitMs, false);
          playSafeChirp();
          delay(1000);
          digitalWrite(LED_SAFE_GREEN, LOW);
        }
        break;
      }
      delay(5); // Fast polling loop
    }

    digitalWrite(LED_FLASH_YEL, LOW);

    if (!captured) {
      Serial.println(F("[Radar] Vehicle entered Gate 1 but stopped or turned back."));
    }

    delay(1000); // Debounce to allow vehicle to clear radar zone
  }

  // 3. Check for Reverse / Wrong-Way Traffic (Sensor 2 triggered first)
  long dist2Check = getDistance(TRIG_2, ECHO_2);
  if (dist2Check > 0 && dist2Check <= DETECTION_RANGE_CM) {
    Serial.println(F("⚠️  [WRONG WAY WARNING] Vehicle traveling against traffic!"));
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_VIOL_RED, HIGH);
      tone(BUZZER_PIN, 500, 100);
      delay(120);
      digitalWrite(LED_VIOL_RED, LOW);
      delay(80);
    }
    delay(1000);
  }

  delay(30);
}

// =====================================================================================
// HELPER FUNCTIONS
// =====================================================================================

void triggerCameraFlash() {
  // Strobe Yellow LED 3 times like an automated speed camera flash
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_FLASH_YEL, HIGH);
    tone(BUZZER_PIN, 2500, 30);
    delay(40);
    digitalWrite(LED_FLASH_YEL, LOW);
    delay(40);
  }
}

void playViolationSiren() {
  digitalWrite(LED_VIOL_RED, HIGH);
  for (int freq = 600; freq <= 1400; freq += 40) {
    tone(BUZZER_PIN, freq, 15);
    delay(15);
  }
  for (int freq = 1400; freq >= 600; freq -= 40) {
    tone(BUZZER_PIN, freq, 15);
    delay(15);
  }
  noTone(BUZZER_PIN);
  delay(1500);
  digitalWrite(LED_VIOL_RED, LOW);
}

void playSafeChirp() {
  tone(BUZZER_PIN, 1000, 80); delay(100);
  tone(BUZZER_PIN, 1400, 120); delay(150);
}

void printTicket(float speedKmh, unsigned long transitMs, bool isViolation) {
  Serial.println(F("\n=================================================="));
  if (isViolation) {
    Serial.println(F("📸 SPEED TRAP CITATION: VIOLATION DETECTED!"));
  } else {
    Serial.println(F("✅ VEHICLE SPEED LOG: WITHIN LEGAL LIMIT"));
  }
  Serial.println(F("=================================================="));
  Serial.print(F("Transit Time     : ")); Serial.print(transitMs); Serial.println(F(" ms"));
  Serial.print(F("Measured Speed   : ")); Serial.print(speedKmh, 1); Serial.println(F(" km/h"));
  Serial.print(F("Speed Limit      : ")); Serial.print(SPEED_LIMIT_KMH, 1); Serial.println(F(" km/h"));
  Serial.print(F("Action Taken     : "));
  if (isViolation) {
    Serial.println(F("OVERSPEED TICKET ISSUED [PHOTO CAPTURED]"));
  } else {
    Serial.println(F("CLEAR - SAFE DRIVER"));
  }
  Serial.println(F("==================================================\n"));
}

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 15000); // 15ms timeout (~2.5m max)
  if (duration == 0) return 999;
  return duration * 0.0343 / 2;
}
