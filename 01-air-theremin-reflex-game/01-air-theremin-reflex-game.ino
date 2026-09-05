/*
 * =====================================================================================
 * PROJECT 01: Contactless "Air Theremin" & 2-Player Reflex Reaction Game
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * 
 * MODES:
 * 1. AIR THEREMIN (Default):
 *    - Hand over Sensor 1 (Left) controls musical pitch/frequency (200Hz - 1200Hz)
 *    - Hand over Sensor 2 (Right) controls tempo / pulse length
 *    - Tap IR Sensor to cycle octave scale presets (Low, Medium, High)
 * 
 * 2. 2-PLAYER REFLEX REACTION GAME:
 *    - Hold hand over IR sensor for 1.5s to enter Game Mode!
 *    - 3 countdown beeps... then random pause (1.5s - 4.5s)
 *    - "GO!" beep sounds & Status LED turns ON
 *    - Player 1 (Sensor 1) and Player 2 (Sensor 2) race to hover hand (< 15cm)
 *    - First to react wins! Winner's LED flashes and reaction time (ms) is printed.
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
// Ultrasonic Sensor 1 (Pitch / Player 1)
const int TRIG_1 = 9;
const int ECHO_1 = 8;

// Ultrasonic Sensor 2 (Tempo / Player 2)
const int TRIG_2 = 7;
const int ECHO_2 = 6;

// IR Obstacle Sensor (Mode Selector / Enter Key)
const int IR_PIN = 2; // Active LOW on most standard IR modules

// Audio Actuator
const int BUZZER_PIN = 10;

// Visual Indicators
const int LED_P1_GREEN = 11; // Player 1 / Low Note indicator
const int LED_P2_RED   = 12; // Player 2 / High Note indicator
const int LED_STATUS   = 5;  // Game Status / "GO" indicator

// --- CONSTANTS & SYSTEM STATES ---
enum SystemMode {
  MODE_THEREMIN,
  MODE_REFLEX_GAME
};

SystemMode currentMode = MODE_THEREMIN;

// Theremin Octave Presets
int octaveOffset = 0; // 0 = Standard, 1 = High, 2 = Low

// Function declarations
long getDistance(int trigPin, int echoPin);
void runTheremin();
void runReflexGame();
void playToneSequence(int *frequencies, int *durations, int length);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 01: Air Theremin & 2-Player Reflex Game  "));
  Serial.println(F("=================================================="));

  // Initialize Ultrasonic Pins
  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);

  // Initialize IR Sensor
  pinMode(IR_PIN, INPUT_PULLUP);

  // Initialize Outputs
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_P1_GREEN, OUTPUT);
  pinMode(LED_P2_RED, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);

  // Startup Sound & LED Test
  digitalWrite(LED_P1_GREEN, HIGH);
  digitalWrite(LED_P2_RED, HIGH);
  digitalWrite(LED_STATUS, HIGH);
  tone(BUZZER_PIN, 523, 100); delay(120);
  tone(BUZZER_PIN, 659, 100); delay(120);
  tone(BUZZER_PIN, 784, 150); delay(180);
  digitalWrite(LED_P1_GREEN, LOW);
  digitalWrite(LED_P2_RED, LOW);
  digitalWrite(LED_STATUS, LOW);

  Serial.println(F("[Ready] Mode 1: Air Theremin Active."));
  Serial.println(F("-> Wave over Left Sensor for Pitch, Right Sensor for Rhythm."));
  Serial.println(F("-> Hold IR sensor for 1.5s to start 2-Player Game!"));
}

void loop() {
  // Check for Mode Switch via IR Sensor
  // Most IR sensors output LOW when an obstacle is present
  if (digitalRead(IR_PIN) == LOW) {
    unsigned long holdStart = millis();
    while (digitalRead(IR_PIN) == LOW) {
      if (millis() - holdStart > 1500) {
        // Switch to Game Mode
        currentMode = (currentMode == MODE_THEREMIN) ? MODE_REFLEX_GAME : MODE_THEREMIN;
        
        // Mode confirmation feedback
        tone(BUZZER_PIN, 880, 100); delay(120);
        tone(BUZZER_PIN, 1320, 200); delay(250);
        
        if (currentMode == MODE_REFLEX_GAME) {
          Serial.println(F("\n>>> SWITCHED TO: 2-Player Reflex Reaction Game! <<<"));
          runReflexGame();
          // After game concludes, return to Theremin
          currentMode = MODE_THEREMIN;
          Serial.println(F("\n>>> RETURNED TO: Air Theremin Mode <<<"));
        } else {
          Serial.println(F("\n>>> SWITCHED TO: Air Theremin Mode <<<"));
        }
        
        while (digitalRead(IR_PIN) == LOW); // Wait for release
        delay(300);
        return;
      }
      delay(50);
    }
    
    // If it was just a quick tap in Theremin mode, cycle octave preset
    if (currentMode == MODE_THEREMIN) {
      octaveOffset = (octaveOffset + 1) % 3;
      tone(BUZZER_PIN, 600 + (octaveOffset * 200), 80);
      Serial.print(F("[Theremin] Octave preset changed to: "));
      Serial.println(octaveOffset);
      delay(250);
    }
  }

  // Execute active mode
  if (currentMode == MODE_THEREMIN) {
    runTheremin();
  }
}

// =====================================================================================
// MODE 1: AIR THEREMIN SYNTHESIZER
// =====================================================================================
void runTheremin() {
  long distPitch = getDistance(TRIG_1, ECHO_1);
  long distTempo = getDistance(TRIG_2, ECHO_2);

  // If a hand is detected on Sensor 1 (Pitch: 4cm to 35cm)
  if (distPitch >= 4 && distPitch <= 35) {
    // Map distance (4-35cm) to musical frequency (C4=262Hz to C6=1046Hz)
    int baseFreq = map(distPitch, 35, 4, 260, 1050);
    
    if (octaveOffset == 1) baseFreq *= 1.5;      // High scale
    else if (octaveOffset == 2) baseFreq *= 0.75;// Low scale

    // Pulse length / tempo based on Sensor 2 (Rhythm)
    int pulseDelay = 0;
    if (distTempo >= 4 && distTempo <= 35) {
      pulseDelay = map(distTempo, 4, 35, 30, 250); // Staccato tempo
    }

    // Visual feedback
    digitalWrite(LED_P1_GREEN, distPitch < 20 ? HIGH : LOW);
    digitalWrite(LED_P2_RED, distPitch >= 20 ? HIGH : LOW);

    // Play tone
    if (pulseDelay > 0) {
      tone(BUZZER_PIN, baseFreq, pulseDelay / 2);
      digitalWrite(LED_STATUS, HIGH);
      delay(pulseDelay / 2);
      digitalWrite(LED_STATUS, LOW);
      delay(pulseDelay / 2);
    } else {
      tone(BUZZER_PIN, baseFreq);
      digitalWrite(LED_STATUS, HIGH);
      delay(30);
    }
  } else {
    // No hand detected: silence
    noTone(BUZZER_PIN);
    digitalWrite(LED_P1_GREEN, LOW);
    digitalWrite(LED_P2_RED, LOW);
    digitalWrite(LED_STATUS, LOW);
    delay(40);
  }
}

// =====================================================================================
// MODE 2: 2-PLAYER REFLEX REACTION GAME
// =====================================================================================
void runReflexGame() {
  Serial.println(F("--------------------------------------------------"));
  Serial.println(F(" Get ready! Place hands 20cm away from sensors... "));
  Serial.println(F(" Wait for the 'GO!' signal. DON'T FALSE-START!    "));
  Serial.println(F("--------------------------------------------------"));

  // 3-2-1 Countdown
  for (int i = 3; i >= 1; i--) {
    Serial.print(F("Countdown: ")); Serial.println(i);
    tone(BUZZER_PIN, 440, 150);
    digitalWrite(LED_STATUS, HIGH);
    delay(150);
    digitalWrite(LED_STATUS, LOW);
    delay(850);
  }

  // Random Delay between 1.5 and 4.5 seconds
  unsigned long waitTime = random(1500, 4500);
  unsigned long startWait = millis();

  // Check for false starts during the wait
  while (millis() - startWait < waitTime) {
    long p1Dist = getDistance(TRIG_1, ECHO_1);
    long p2Dist = getDistance(TRIG_2, ECHO_2);

    if (p1Dist > 0 && p1Dist < 15) {
      Serial.println(F("[FOUL!] Player 1 false-started! Player 2 wins by DQ!"));
      tone(BUZZER_PIN, 150, 600);
      digitalWrite(LED_P2_RED, HIGH);
      delay(1500);
      digitalWrite(LED_P2_RED, LOW);
      return;
    }
    if (p2Dist > 0 && p2Dist < 15) {
      Serial.println(F("[FOUL!] Player 2 false-started! Player 1 wins by DQ!"));
      tone(BUZZER_PIN, 150, 600);
      digitalWrite(LED_P1_GREEN, HIGH);
      delay(1500);
      digitalWrite(LED_P1_GREEN, LOW);
      return;
    }
    delay(20);
  }

  // === GO SIGNAL! ===
  Serial.println(F("\n>>> GO! GO! GO! <<<"));
  digitalWrite(LED_STATUS, HIGH);
  tone(BUZZER_PIN, 1200, 250);
  unsigned long goTime = millis();

  int winner = 0;
  unsigned long reactionTime = 0;

  // Wait for fastest player (timeout after 5 seconds)
  while (millis() - goTime < 5000) {
    long d1 = getDistance(TRIG_1, ECHO_1);
    long d2 = getDistance(TRIG_2, ECHO_2);

    if (d1 > 0 && d1 < 15) {
      winner = 1;
      reactionTime = millis() - goTime;
      break;
    }
    if (d2 > 0 && d2 < 15) {
      winner = 2;
      reactionTime = millis() - goTime;
      break;
    }
    delay(10);
  }

  digitalWrite(LED_STATUS, LOW);

  // Announce Winner
  if (winner == 1) {
    Serial.print(F("🏆 PLAYER 1 WINS (Green)! Reaction time: "));
    Serial.print(reactionTime);
    Serial.println(F(" ms"));

    // Victory Fanfare for Player 1
    for (int i = 0; i < 4; i++) {
      digitalWrite(LED_P1_GREEN, HIGH);
      tone(BUZZER_PIN, 800 + (i * 100), 100);
      delay(120);
      digitalWrite(LED_P1_GREEN, LOW);
      delay(60);
    }
  } else if (winner == 2) {
    Serial.print(F("🏆 PLAYER 2 WINS (Red)! Reaction time: "));
    Serial.print(reactionTime);
    Serial.println(F(" ms"));

    // Victory Fanfare for Player 2
    for (int i = 0; i < 4; i++) {
      digitalWrite(LED_P2_RED, HIGH);
      tone(BUZZER_PIN, 800 + (i * 100), 100);
      delay(120);
      digitalWrite(LED_P2_RED, LOW);
      delay(60);
    }
  } else {
    Serial.println(F("⏰ Timeout! Nobody reacted in time."));
    tone(BUZZER_PIN, 200, 500);
  }

  Serial.println(F("\nGame Over. Tap IR sensor to return or play again."));
  delay(1500);
}

// =====================================================================================
// HELPER: HIGH-PRECISION ULTRASONIC DISTANCE READING
// =====================================================================================
long getDistance(int trigPin, int echoPin) {
  // Clear trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send 10µs pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure echo pulse (with 25ms timeout ~4.3 meters max)
  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 999; // Out of range

  // Speed of sound: 343 m/s = 0.0343 cm/µs
  // Distance = (duration * 0.0343) / 2
  long distanceCm = duration * 0.0343 / 2;
  return distanceCm;
}
