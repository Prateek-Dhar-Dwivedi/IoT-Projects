#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

/*
 * =====================================================================================
 * PROJECT 06: Ergonomic Posture & Screen Distance Monitor (IoT Web Dashboard)
 * Microcontroller: ESP32 Dev Module
 * Hardware: 2x Ultrasonic (HC-SR04) + 1x IR Sensor + 1x Buzzer + 3x LEDs
 * =====================================================================================
 * 
 * 🌐 IOT FEATURE (WI-FI ACCESS POINT):
 * - The ESP32 creates its own Wi-Fi network! No router or password needed.
 * - Wi-Fi Name (SSID) : Smart-Posture-Monitor
 * - Wi-Fi Password    : 12345678
 * - Web Dashboard URL : http://192.168.4.1
 * Connect your phone/laptop to this Wi-Fi and open the URL to view live posture stats!
 * 
 * 🪑 ERGONOMIC MONITORING:
 * 1. Screen Distance (Sensor 1): Warns if eyes/face < 40 cm to screen (Eye Strain).
 * 2. Slouching (Sensor 2)     : Detects if your torso moves too far/slouches.
 * 3. 20-20-20 Rest Timer      : Reminds you every 20 mins to rest your eyes.
 * 4. IR Sensor Snooze         : Wave hand over IR to dismiss/snooze reminder.
 * =====================================================================================
 */

// --- PIN DEFINITIONS (ESP32 GPIOs) ---
const int TRIG_SCREEN = 18; // Sensor 1: Screen-to-face distance
const int ECHO_SCREEN = 19;
const int TRIG_TORSO  = 21; // Sensor 2: Torso posture distance
const int ECHO_TORSO  = 22;
const int IR_SNOOZE   = 23; // IR: Touchless snooze button

const int BUZZER_PIN     = 25; // Audio reminder
const int LED_GOOD_GREEN = 26; // Good posture
const int LED_WARN_RED   = 27; // Slouching / Too close
const int LED_REST_YEL   = 33; // 20-20-20 Break alert

// --- ERGONOMIC THRESHOLDS (cm) ---
const int MIN_SCREEN_DIST_CM = 38; // Closer than 38 cm = Eye strain warning
const int MAX_SCREEN_DIST_CM = 75; // Normal desk distance range
const int SLOUCH_THRESHOLD_CM = 15; // Deviation from calibrated posture

// --- SYSTEM STATE ---
long currentScreenDist = 50;
long currentTorsoDist  = 40;
long calibratedTorso   = 40;
bool isCalibrated      = false;
String postureStatus   = "Initializing...";
bool tooCloseWarning   = false;
bool slouchingWarning  = false;
bool breakNeeded       = false;

// 20-20-20 Rule Timer (20 mins = 1200 seconds. For demo, set to 120s or 1200s)
unsigned long timerStart = 0;
const unsigned long BREAK_INTERVAL_MS = 60000; // 60s demo (set to 1200000 for 20m)
unsigned long lastBuzz = 0;

WebServer server(80);

long getDistance(int trigPin, int echoPin);
void handleRoot();
void handleData();
void handleCalibrate();

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

  digitalWrite(LED_GOOD_GREEN, HIGH);
  digitalWrite(LED_WARN_RED, HIGH);
  digitalWrite(LED_REST_YEL, HIGH);
  delay(300);
  digitalWrite(LED_GOOD_GREEN, LOW);
  digitalWrite(LED_WARN_RED, LOW);
  digitalWrite(LED_REST_YEL, LOW);

  // Start Wi-Fi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Smart-Posture-Monitor", "12345678");
  IPAddress IP = WiFi.softAPIP();

  Serial.println(F("🌐 [IoT] Wi-Fi Access Point Started!"));
  Serial.print(F("-> SSID     : Smart-Posture-Monitor\n-> Password : 12345678\n"));
  Serial.print(F("-> Dashboard: http://")); Serial.println(IP);

  // Setup Web Server Endpoints
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/calibrate", handleCalibrate);
  server.begin();
  Serial.println(F("-> Web server active on port 80.\n"));

  timerStart = millis();
}

void loop() {
  server.handleClient(); // Handle incoming web browser requests

  // 1. Measure Distances
  currentScreenDist = getDistance(TRIG_SCREEN, ECHO_SCREEN);
  currentTorsoDist  = getDistance(TRIG_TORSO, ECHO_TORSO);

  // Auto-calibrate upright posture on first good reading
  if (!isCalibrated && currentTorsoDist >= 20 && currentTorsoDist <= 80) {
    calibratedTorso = currentTorsoDist;
    isCalibrated = true;
    Serial.print(F("🎯 [Calibrated] Upright torso baseline set to: "));
    Serial.print(calibratedTorso); Serial.println(F(" cm"));
  }

  // 2. Evaluate Screen Distance (Eye strain)
  tooCloseWarning = (currentScreenDist > 0 && currentScreenDist < MIN_SCREEN_DIST_CM);

  // 3. Evaluate Slouching (Deviation from baseline)
  slouchingWarning = false;
  if (isCalibrated && currentTorsoDist > 0 && abs(currentTorsoDist - calibratedTorso) > SLOUCH_THRESHOLD_CM) {
    slouchingWarning = true;
  }

  // 4. 20-20-20 Break Timer Check
  if (millis() - timerStart > BREAK_INTERVAL_MS) {
    breakNeeded = true;
    digitalWrite(LED_REST_YEL, HIGH);
  }

  // 5. Touchless Snooze / Calibration via IR Sensor
  if (digitalRead(IR_SNOOZE) == LOW) {
    // Reset break timer & recalibrate upright posture
    timerStart = millis();
    breakNeeded = false;
    calibratedTorso = currentTorsoDist;
    digitalWrite(LED_REST_YEL, LOW);
    
    // Friendly chirp confirmation
    tone(BUZZER_PIN, 1200, 100); delay(120);
    tone(BUZZER_PIN, 1600, 150);
    Serial.println(F("✋ [Touchless Snooze] Break dismissed & posture re-calibrated!"));
    while (digitalRead(IR_SNOOZE) == LOW);
    delay(300);
  }

  // 6. Update Status & Physical Indicators
  if (tooCloseWarning) {
    postureStatus = "TOO CLOSE TO SCREEN! LEAN BACK";
    digitalWrite(LED_GOOD_GREEN, LOW);
    digitalWrite(LED_WARN_RED, HIGH);
    if (millis() - lastBuzz > 1000) {
      tone(BUZZER_PIN, 1500, 80);
      lastBuzz = millis();
    }
  } 
  else if (slouchingWarning) {
    postureStatus = "SLOUCHING DETECTED! SIT UPRIGHT";
    digitalWrite(LED_GOOD_GREEN, LOW);
    digitalWrite(LED_WARN_RED, HIGH);
    if (millis() - lastBuzz > 1200) {
      tone(BUZZER_PIN, 800, 100);
      lastBuzz = millis();
    }
  } 
  else {
    postureStatus = "EXCELLENT POSTURE";
    digitalWrite(LED_GOOD_GREEN, HIGH);
    digitalWrite(LED_WARN_RED, LOW);
    noTone(BUZZER_PIN);
  }

  delay(40);
}

// =====================================================================================
// WEB SERVER HANDLERS (HTML5 DARK-MODE IOT DASHBOARD)
// =====================================================================================

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Posture IoT Monitor</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #0f172a; color: #f8fafc; text-align: center; margin: 0; padding: 20px; }
    h1 { color: #38bdf8; margin-bottom: 5px; }
    p.sub { color: #94a3b8; font-size: 14px; margin-top: 0; }
    .grid { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; max-width: 800px; margin: 20px auto; }
    .card { background: #1e293b; border-radius: 16px; padding: 20px; width: 220px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); border: 1px solid #334155; }
    .card h3 { color: #94a3b8; font-size: 13px; text-transform: uppercase; margin: 0; }
    .val { font-size: 42px; font-weight: bold; margin: 10px 0; color: #38bdf8; }
    .unit { font-size: 16px; color: #64748b; }
    .badge { display: inline-block; padding: 12px 24px; border-radius: 30px; font-size: 18px; font-weight: bold; margin-top: 10px; width: 80%; max-width: 400px; }
    .good { background: #065f46; color: #34d399; border: 1px solid #059669; }
    .warn { background: #7f1d1d; color: #f87171; border: 1px solid #dc2626; }
    button { background: #0284c7; color: white; border: none; padding: 12px 24px; border-radius: 8px; font-size: 16px; cursor: pointer; margin-top: 25px; }
    button:hover { background: #0369a1; }
  </style>
</head>
<body>
  <h1>🪑 Smart Posture Monitor</h1>
  <p class="sub">IoT Real-Time Ergonomic Health Dashboard (ESP32)</p>

  <div id="statusBadge" class="badge good">EXCELLENT POSTURE</div>

  <div class="grid">
    <div class="card">
      <h3>Screen Distance</h3>
      <div id="screenVal" class="val">--<span class="unit"> cm</span></div>
      <p style="color:#64748b;font-size:12px;">Target: > 40 cm</p>
    </div>
    <div class="card">
      <h3>Torso Distance</h3>
      <div id="torsoVal" class="val">--<span class="unit"> cm</span></div>
      <p style="color:#64748b;font-size:12px;">Baseline: <span id="baselineVal">--</span> cm</p>
    </div>
    <div class="card">
      <h3>Eye Break Timer</h3>
      <div id="timerVal" class="val">--<span class="unit"> s</span></div>
      <p style="color:#64748b;font-size:12px;">20-20-20 Rule</p>
    </div>
  </div>

  <button onclick="fetch('/calibrate')">🎯 Calibrate Upright Sitting Posture</button>

  <script>
    function updateData() {
      fetch('/data')
        .then(res => res.json())
        .then(data => {
          document.getElementById('screenVal').innerHTML = data.screen + '<span class="unit"> cm</span>';
          document.getElementById('torsoVal').innerHTML = data.torso + '<span class="unit"> cm</span>';
          document.getElementById('baselineVal').innerText = data.baseline;
          document.getElementById('timerVal').innerHTML = data.timer + '<span class="unit"> s</span>';
          
          let badge = document.getElementById('statusBadge');
          badge.innerText = data.status;
          if (data.status.includes('EXCELLENT')) {
            badge.className = 'badge good';
          } else {
            badge.className = 'badge warn';
          }
        });
    }
    setInterval(updateData, 1000);
    updateData();
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleData() {
  long secondsRemaining = 0;
  if (millis() - timerStart < BREAK_INTERVAL_MS) {
    secondsRemaining = (BREAK_INTERVAL_MS - (millis() - timerStart)) / 1000;
  }

  String json = "{";
  json += "\"screen\":" + String(currentScreenDist) + ",";
  json += "\"torso\":" + String(currentTorsoDist) + ",";
  json += "\"baseline\":" + String(calibratedTorso) + ",";
  json += "\"status\":\"" + postureStatus + "\",";
  json += "\"timer\":" + String(secondsRemaining);
  json += "}";
  server.send(200, "application/json", json);
}

void handleCalibrate() {
  calibratedTorso = currentTorsoDist;
  isCalibrated = true;
  timerStart = millis();
  breakNeeded = false;
  server.send(200, "text/plain", "Calibrated!");
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
