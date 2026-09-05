/*
 * =====================================================================================
 * PROJECT 01: Contactless "Air Theremin" & 2-Player Reflex Reaction Game
 * Hardware: Arduino Uno + 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * PIN ASSIGNMENTS:
 * - Ultrasonic 1 (Pitch / Player 1) : TRIG = Pin 9, ECHO = Pin 8
 * - Ultrasonic 2 (Tempo / Player 2) : TRIG = Pin 7, ECHO = Pin 6
 * - IR Sensor (Mode / Octave)       : OUT  = Pin 2
 * - Piezo Buzzer                    : (+)  = Pin 10
 * - Green LED (Close / Player 1)    : (+)  = Pin 11
 * - Red LED   (Far / Player 2)      : (+)  = Pin 12
 * - Yellow LED (Mid / Status / GO)  : (+)  = Pin 13
 * 
 * 🎮 HOW TO USE:
 * 1. Air Theremin Mode (Default):
 *    - Close  ( 4cm - 15cm) : GREEN LED ON (Low Pitch)
 *    - Middle (15cm - 25cm) : YELLOW LED ON & BLINKS (Mid Pitch)
 *    - Far    (25cm - 38cm) : RED LED ON (High Pitch)
 *    - Sensor 2 (Right)     : Wave hand to add rhythmic pulse/tempo
 *    - Tap IR Sensor        : Switches octave preset
 * 
 * 2. 2-Player Reflex Game:
 *    - Hold hand on IR Sensor for 1.5s to start game!
 *    - Yellow LED blinks for 3-2-1 countdown... then random pause.
 *    - "GO!" beep sounds & Yellow LED turns ON solid.
 *    - Race to hover hand (< 15cm) over your sensor!
 *    - Winner's LED flashes & reaction time (ms) prints to Serial Monitor.
 * =====================================================================================
 */

// --- PIN DEFINITIONS ---
const int TRIG_1 = 9;
const int ECHO_1 = 8;
const int TRIG_2 = 7;
const int ECHO_2 = 6;
const int IR_PIN = 2;
const int BUZZER_PIN = 10;

const int LED_P1_GREEN = 11; // Pin 11: Green LED
const int LED_P2_RED   = 12; // Pin 12: Red LED
const int LED_STATUS   = 13; // Pin 13: Yellow LED

// --- SYSTEM STATES ---
enum SystemMode { MODE_THEREMIN, MODE_REFLEX_GAME };
SystemMode currentMode = MODE_THEREMIN;
int octaveOffset = 0;

long getDistance(int trigPin, int echoPin);
void runTheremin();
void runReflexGame();

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=================================================="));
  Serial.println(F(" Project 01: Air Theremin & 2-Player Reflex Game  "));
  Serial.println(F("=================================================="));

  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);
  pinMode(IR_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_P1_GREEN, OUTPUT);
  pinMode(LED_P2_RED, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);

  // Power-on startup animation: Green -> Red -> Yellow, then power chord!
  digitalWrite(LED_P1_GREEN, HIGH); delay(150); digitalWrite(LED_P1_GREEN, LOW);
  digitalWrite(LED_STATUS, HIGH);   delay(150); digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_P2_RED, HIGH);   delay(150); digitalWrite(LED_P2_RED, LOW);

  digitalWrite(LED_P1_GREEN, HIGH);
  digitalWrite(LED_STATUS, HIGH);
  digitalWrite(LED_P2_RED, HIGH);
  tone(BUZZER_PIN, 523, 100); delay(120);
  tone(BUZZER_PIN, 659, 100); delay(120);
  tone(BUZZER_PIN, 784, 150); delay(180);
  digitalWrite(LED_P1_GREEN, LOW);
  digitalWrite(LED_STATUS, LOW);
  digitalWrite(LED_P2_RED, LOW);

  Serial.println(F("[Ready] Mode 1: Air Theremin Active."));
  Serial.println(F("-> Move hand over Sensor 1 (Green -> Yellow -> Red)"));
  Serial.println(F("-> Hold IR Sensor 1.5s for 2-Player Reflex Game"));
}

void loop() {
  // Check for Mode Switch via IR Sensor (Active LOW)
  if (digitalRead(IR_PIN) == LOW) {
    unsigned long holdStart = millis();
    while (digitalRead(IR_PIN) == LOW) {
      if (millis() - holdStart > 1500) {
        currentMode = (currentMode == MODE_THEREMIN) ? MODE_REFLEX_GAME : MODE_THEREMIN;
        tone(BUZZER_PIN, 880, 100); delay(120);
        tone(BUZZER_PIN, 1320, 200); delay(250);
        
        if (currentMode == MODE_REFLEX_GAME) {
          Serial.println(F("\n>>> STARTING: 2-Player Reflex Reaction Game! <<<"));
          runReflexGame();
          currentMode = MODE_THEREMIN;
          Serial.println(F("\n>>> RETURNED TO: Air Theremin Mode <<<"));
        }
        while (digitalRead(IR_PIN) == LOW);
        delay(300);
        return;
      }
      delay(50);
    }
    
    // Quick tap switches octave preset
    if (currentMode == MODE_THEREMIN) {
      octaveOffset = (octaveOffset + 1) % 3;
      tone(BUZZER_PIN, 600 + (octaveOffset * 200), 80);
      Serial.print(F("[Theremin] Octave preset changed to: "));
      Serial.println(octaveOffset);
      delay(250);
    }
  }

  // Run Air Theremin
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

  // Active sensing zone: 4 cm to 38 cm
  if (distPitch >= 4 && distPitch <= 38) {
    int baseFreq = map(distPitch, 38, 4, 260, 1050);
    if (octaveOffset == 1) baseFreq *= 1.4;
    else if (octaveOffset == 2) baseFreq *= 0.75;

    // --- 3-ZONE LED METER ---
    if (distPitch < 15) {
      // Close (4-15 cm) -> Green LED
      digitalWrite(LED_P1_GREEN, HIGH);
      digitalWrite(LED_STATUS, LOW);
      digitalWrite(LED_P2_RED, LOW);
    } 
    else if (distPitch >= 15 && distPitch <= 25) {
      // Middle (15-25 cm) -> Yellow LED (Pin 13) Pulses!
      digitalWrite(LED_P1_GREEN, LOW);
      digitalWrite(LED_STATUS, (millis() % 200 < 100) ? HIGH : LOW);
      digitalWrite(LED_P2_RED, LOW);
    } 
    else {
      // Far (25-38 cm) -> Red LED
      digitalWrite(LED_P1_GREEN, LOW);
      digitalWrite(LED_STATUS, LOW);
      digitalWrite(LED_P2_RED, HIGH);
    }

    // Tempo modulation from Sensor 2
    int pulseDelay = 0;
    if (distTempo >= 4 && distTempo <= 35) {
      pulseDelay = map(distTempo, 4, 35, 40, 220);
    }

    if (pulseDelay > 0) {
      tone(BUZZER_PIN, baseFreq, pulseDelay / 2);
      delay(pulseDelay / 2);
      noTone(BUZZER_PIN);
      delay(pulseDelay / 2);
    } else {
      tone(BUZZER_PIN, baseFreq);
      delay(25);
    }
  } else {
    // Silence when hands are away
    noTone(BUZZER_PIN);
    digitalWrite(LED_P1_GREEN, LOW);
    digitalWrite(LED_STATUS, LOW);
    digitalWrite(LED_P2_RED, LOW);
    delay(40);
  }
}

// =====================================================================================
// MODE 2: 2-PLAYER REFLEX REACTION GAME
// =====================================================================================
void runReflexGame() {
  Serial.println(F("--------------------------------------------------"));
  Serial.println(F(" Get ready! Hands 20cm away from sensors...       "));
  Serial.println(F(" Watch Yellow LED (Pin 13) for countdown!         "));
  Serial.println(F("--------------------------------------------------"));

  // 3-2-1 Countdown on Yellow LED
  for (int i = 3; i >= 1; i--) {
    Serial.print(F("Countdown: ")); Serial.println(i);
    tone(BUZZER_PIN, 440, 150);
    digitalWrite(LED_STATUS, HIGH);
    delay(150);
    digitalWrite(LED_STATUS, LOW);
    delay(850);
  }

  // Random Delay (1.5s to 4.5s)
  unsigned long waitTime = random(1500, 4500);
  unsigned long startWait = millis();

  // False-start check
  while (millis() - startWait < waitTime) {
    if (getDistance(TRIG_1, ECHO_1) < 15) {
      Serial.println(F("[FOUL!] Player 1 moved early! Player 2 Wins!"));
      tone(BUZZER_PIN, 150, 600);
      digitalWrite(LED_P2_RED, HIGH);
      delay(1500);
      digitalWrite(LED_P2_RED, LOW);
      return;
    }
    if (getDistance(TRIG_2, ECHO_2) < 15) {
      Serial.println(F("[FOUL!] Player 2 moved early! Player 1 Wins!"));
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
  digitalWrite(LED_STATUS, HIGH); // Yellow LED ON for GO!
  tone(BUZZER_PIN, 1200, 250);
  unsigned long goTime = millis();

  int winner = 0;
  unsigned long reactionTime = 0;

  while (millis() - goTime < 5000) {
    if (getDistance(TRIG_1, ECHO_1) < 15) {
      winner = 1;
      reactionTime = millis() - goTime;
      break;
    }
    if (getDistance(TRIG_2, ECHO_2) < 15) {
      winner = 2;
      reactionTime = millis() - goTime;
      break;
    }
    delay(10);
  }

  digitalWrite(LED_STATUS, LOW);

  if (winner == 1) {
    Serial.print(F("🏆 PLAYER 1 WINS (Green)! Time: "));
    Serial.print(reactionTime);
    Serial.println(F(" ms"));
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_P1_GREEN, HIGH);
      tone(BUZZER_PIN, 800 + (i * 100), 100);
      delay(120);
      digitalWrite(LED_P1_GREEN, LOW);
      delay(60);
    }
  } else if (winner == 2) {
    Serial.print(F("🏆 PLAYER 2 WINS (Red)! Time: "));
    Serial.print(reactionTime);
    Serial.println(F(" ms"));
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_P2_RED, HIGH);
      tone(BUZZER_PIN, 800 + (i * 100), 100);
      delay(120);
      digitalWrite(LED_P2_RED, LOW);
      delay(60);
    }
  } else {
    Serial.println(F("⏰ Timeout! Nobody reacted."));
    tone(BUZZER_PIN, 200, 500);
  }

  delay(1500);
}

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return 999;
  return duration * 0.0343 / 2;
}
