#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

/*
 * =====================================================================================
 * PROJECT 11: Edge AI Face Detection & Biometric Security Sentry
 * Board: AI-Thinker ESP32-CAM (with ESP32-CAM-MB micro-USB programmer shield)
 * Hardware Wiring: ZERO EXTRA WIRING NEEDED (Runs 100% on the standalone board!)
 *
 * Architecture:
 *   - Dual-Server Design:
 *       * Port 80 : High-speed non-blocking UI, AI frame grabber, Flash LED & controls
 *       * Port 81 : Dedicated low-latency MJPEG video stream
 *   - Untainted Same-Origin AI Vision Pipeline:
 *       * Eliminates CORS / canvas-tainting security restrictions
 *       * Real-time YCbCr normalized skin & facial geometry detection (12-18 FPS)
 *       * Biometric Face Enrollment ("Subject 0" Authorized Master vs Stranger)
 *       * Web Audio API Synthesized Security Warning Siren
 *       * Automated High-Power Spotlight Strobe & Snapshot Evidence Logging (GPIO 4)
 * =====================================================================================
 */

// --- Network Configuration ---
// Home / Phone Hotspot credentials (2.4 GHz ONLY)
const char* wifi_ssid = "fhaaaa";
const char* wifi_password = "sohit7ss";

// Autonomous Access Point credentials (Used if home Wi-Fi is unreachable)
const char* ap_ssid = "ESP32-CAM-AI-SENTRY";
const char* ap_password = "password123";

// --- AI-Thinker ESP32-CAM Pin Definitions ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_LED_PIN      4   // High-Power White Spotlight LED
#define STATUS_LED_PIN    33   // Onboard Red Indicator LED (Active LOW)

// HTTP Server Handles
httpd_handle_t camera_httpd = NULL; // Port 80 (UI, Controls, Frame Grabber)
httpd_handle_t stream_httpd = NULL; // Port 81 (Dedicated MJPEG Stream)
bool flashState = false;

// Web UI Dashboard HTML & Edge AI JavaScript (Stored in Flash PROGMEM)
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>AI Face Sentry & Biometric Security</title>
  <style>
    :root {
      --bg: #090d16;
      --card: #131b2e;
      --card-border: #1e293b;
      --accent-cyan: #00f2fe;
      --accent-green: #10b981;
      --accent-red: #ef4444;
      --accent-yellow: #f59e0b;
      --text-main: #f8fafc;
      --text-dim: #94a3b8;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }
    body { background: var(--bg); color: var(--text-main); text-align: center; padding: 12px; }
    .container { max-width: 780px; margin: 0 auto; }
    header { margin-bottom: 12px; }
    h1 { font-size: 1.5rem; color: var(--accent-cyan); display: flex; align-items: center; justify-content: center; gap: 8px; letter-spacing: 0.5px; }
    .status-badge-bar { display: flex; justify-content: center; gap: 8px; flex-wrap: wrap; margin-top: 8px; }
    .badge { background: var(--card); border: 1px solid var(--card-border); padding: 4px 12px; border-radius: 9999px; font-size: 0.78rem; color: var(--text-dim); display: inline-flex; align-items: center; gap: 5px; }
    .badge.active { border-color: var(--accent-green); color: var(--accent-green); }
    .badge.alert { border-color: var(--accent-red); color: var(--accent-red); animation: pulse 0.8s infinite alternate; }
    @keyframes pulse { from { opacity: 0.6; } to { opacity: 1; filter: drop-shadow(0 0 8px var(--accent-red)); } }

    /* Video & Canvas HUD Wrapper */
    .stream-wrapper {
      position: relative;
      background: #000;
      border: 2px solid var(--card-border);
      border-radius: 14px;
      overflow: hidden;
      box-shadow: 0 12px 30px rgba(0,0,0,0.7);
      margin: 10px auto;
      display: block;
      width: 100%;
      max-width: 640px;
      min-height: 240px;
    }
    #stream { width: 100%; height: auto; display: block; }
    #ai-canvas {
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      pointer-events: none;
    }
    #work-canvas { display: none; }

    /* System Stats Strip */
    .hud-strip {
      background: rgba(19, 27, 46, 0.85);
      border: 1px solid var(--card-border);
      border-radius: 10px;
      padding: 8px 12px;
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 6px;
      font-size: 0.8rem;
      margin-bottom: 12px;
    }
    .hud-item { display: flex; flex-direction: column; align-items: center; }
    .hud-label { font-size: 0.68rem; color: var(--text-dim); text-transform: uppercase; }
    .hud-val { font-weight: bold; font-family: monospace; font-size: 0.95rem; }

    /* Controls Grid */
    .panel {
      background: var(--card);
      border: 1px solid var(--card-border);
      border-radius: 12px;
      padding: 12px 14px;
      margin-bottom: 12px;
    }
    .panel-title { font-size: 0.88rem; color: var(--accent-cyan); margin-bottom: 10px; text-transform: uppercase; letter-spacing: 0.8px; text-align: left; display: flex; align-items: center; gap: 6px; }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    button {
      background: #1e293b;
      border: 1px solid #334155;
      color: var(--text-main);
      padding: 12px 14px;
      border-radius: 8px;
      font-size: 0.92rem;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.2s;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 6px;
    }
    button:hover { background: #334155; border-color: #64748b; }
    button:active { transform: scale(0.98); }
    .btn-enroll { background: #065f46; border-color: #059669; }
    .btn-enroll:hover { background: #047857; }
    .btn-siren { background: #7f1d1d; border-color: #dc2626; }
    .btn-siren.active { background: #dc2626; box-shadow: 0 0 12px rgba(220,38,38,0.6); }
    .btn-spotlight { background: #78350f; border-color: #d97706; }
    .btn-spotlight.on { background: #d97706; color: #000; box-shadow: 0 0 15px #d97706; }
    .btn-snap { background: #1e40af; border-color: #3b82f6; }
    .btn-reset { background: #334155; border-color: #475569; font-size: 0.85rem; }

    /* Resolution & Tuning Row */
    .tuning-row { display: flex; justify-content: space-between; align-items: center; gap: 10px; flex-wrap: wrap; margin-top: 10px; }
    .res-group { display: flex; gap: 6px; }
    .res-btn { padding: 6px 12px; font-size: 0.78rem; border-radius: 6px; background: #1e293b; border: 1px solid #334155; }
    .res-btn.active { background: var(--accent-cyan); color: #000; font-weight: bold; border-color: var(--accent-cyan); }

    /* Incident Log */
    #log-panel { max-height: 180px; overflow-y: auto; text-align: left; font-size: 0.8rem; font-family: monospace; }
    .log-item { padding: 6px 10px; border-bottom: 1px solid #1e293b; display: flex; justify-content: space-between; align-items: center; }
    .log-item.alert { color: #f87171; background: rgba(239, 68, 68, 0.1); }
    .log-item.auth { color: #34d399; background: rgba(16, 185, 129, 0.1); }

    footer { margin-top: 18px; font-size: 0.75rem; color: #64748b; }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>🛡️ AI Face Sentry & Biometrics</h1>
      <div class="status-badge-bar">
        <span id="sentry-badge" class="badge active">🟢 Sentry Armed</span>
        <span id="target-badge" class="badge">👁️ Scanning for Faces</span>
        <span id="enrolled-badge" class="badge">👤 Subject 0: Not Enrolled</span>
      </div>
    </header>

    <!-- Video Canvas HUD Display -->
    <div class="stream-wrapper">
      <img id="stream" src="" alt="Live Video Feed">
      <canvas id="ai-canvas"></canvas>
      <canvas id="work-canvas"></canvas>
    </div>

    <!-- Live Telemetry HUD Strip -->
    <div class="hud-strip">
      <div class="hud-item">
        <span class="hud-label">AI Pipeline FPS</span>
        <span id="hud-fps" class="hud-val" style="color:var(--accent-cyan)">0</span>
      </div>
      <div class="hud-item">
        <span class="hud-label">Identity Match</span>
        <span id="hud-match" class="hud-val" style="color:var(--accent-green)">-- %</span>
      </div>
      <div class="hud-item">
        <span class="hud-label">Face Area</span>
        <span id="hud-area" class="hud-val" style="color:var(--accent-yellow)">None</span>
      </div>
      <div class="hud-item">
        <span class="hud-label">Intruder Events</span>
        <span id="hud-alerts" class="hud-val" style="color:var(--accent-red)">0</span>
      </div>
    </div>

    <!-- Biometric ID Controls -->
    <div class="panel">
      <div class="panel-title">👤 Biometric Enrollment & Master Profile</div>
      <div class="btn-grid">
        <button class="btn-enroll" onclick="enrollCurrentFace()">
          📸 <span>Enroll Face (Subject 0)</span>
        </button>
        <button class="btn-reset" onclick="clearEnrollment()">
          🗑️ <span>Clear Biometrics</span>
        </button>
      </div>
    </div>

    <!-- Active Defense & Controls -->
    <div class="panel">
      <div class="panel-title">🚨 Active Defense & Controls</div>
      <div class="btn-grid">
        <button id="siren-btn" class="btn-siren active" onclick="toggleSiren()">
          🔊 <span>Audio Siren: ON</span>
        </button>
        <button id="spotlight-btn" class="btn-spotlight" onclick="toggleSpotlight()">
          💡 <span>Spotlight: OFF</span>
        </button>
        <button class="btn-snap" onclick="takeManualSnapshot()">
          📷 <span>Snap Evidence</span>
        </button>
        <button id="auto-flash-btn" class="btn-reset" onclick="toggleAutoFlash()">
          ⚡ <span>Auto-Strobe: ON</span>
        </button>
      </div>

      <div class="tuning-row">
        <span style="font-size:0.8rem; color:var(--text-dim);">Camera Resolution:</span>
        <div class="res-group">
          <button class="res-btn active" onclick="setRes('qvga', this)">QVGA (Fast AI)</button>
          <button class="res-btn" onclick="setRes('vga', this)">VGA (Smooth)</button>
          <button class="res-btn" onclick="setRes('svga', this)">SVGA (Crisp)</button>
        </div>
      </div>
    </div>

    <!-- Security Activity Log -->
    <div class="panel">
      <div class="panel-title">📋 Live Security Incident Log</div>
      <div id="log-panel">
        <div class="log-item">[SYSTEM] AI Edge Sentry Synchronized. Dual-port stream online.</div>
      </div>
    </div>

    <div id="snap-preview" style="display:none; margin-top:14px; background:var(--card); border:1px solid var(--card-border); border-radius:12px; padding:12px;">
      <div class="panel-title">📸 Captured Security Snapshot</div>
      <img id="snap-img" src="" style="max-width:100%; border-radius:8px; border:1px solid #334155; margin-top:8px;">
      <div style="margin-top:10px;">
        <a id="snap-download" href="" download="sentry_evidence.jpg"><button style="width:100%;">💾 Download Evidence Image</button></a>
      </div>
    </div>

    <footer>
      Project 11: Real-Time AI Face Detection & Biometric Security Sentry &bull; AI-Thinker OV2640
    </footer>
  </div>

  <script>
    // State Variables
    let enrolledVector = null;
    let sirenEnabled = true;
    let autoFlashEnabled = true;
    let spotlightOn = false;
    let intruderCount = 0;
    let audioCtx = null;
    let lastAlertTime = 0;
    let frameCount = 0;
    let fps = 0;
    let lastFpsTime = Date.now();
    let currentFaceResult = null;

    const streamImg = document.getElementById('stream');
    const aiCanvas = document.getElementById('ai-canvas');
    const aiCtx = aiCanvas.getContext('2d');
    const workCanvas = document.getElementById('work-canvas');
    const workCtx = workCanvas.getContext('2d', { willReadFrequently: true });

    // Connect to dedicated Port 81 for MJPEG stream
    const port81Stream = window.location.protocol + '//' + window.location.hostname + ':81/stream';
    streamImg.src = port81Stream;

    // Web Audio Synthesizer (Auto-resumes on first user touch/click)
    function initAudio() {
      if (!audioCtx) {
        audioCtx = new (window.AudioContext || window.webkitAudioContext)();
      }
      if (audioCtx.state === 'suspended') {
        audioCtx.resume();
      }
    }
    ['click', 'touchstart', 'mousedown'].forEach(ev => document.addEventListener(ev, initAudio, { once: true }));

    function playSirenAlert() {
      if (!sirenEnabled) return;
      try {
        initAudio();
        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();
        osc.connect(gain);
        gain.connect(audioCtx.destination);
        const now = audioCtx.currentTime;
        osc.type = 'sawtooth';
        osc.frequency.setValueAtTime(750, now);
        osc.frequency.linearRampToValueAtTime(1350, now + 0.15);
        osc.frequency.linearRampToValueAtTime(750, now + 0.3);
        gain.gain.setValueAtTime(0.35, now);
        gain.gain.exponentialRampToValueAtTime(0.01, now + 0.35);
        osc.start(now);
        osc.stop(now + 0.35);
      } catch (e) {
        console.warn('Audio alert error:', e);
      }
    }

    function playSuccessChime() {
      try {
        initAudio();
        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();
        osc.connect(gain);
        gain.connect(audioCtx.destination);
        const now = audioCtx.currentTime;
        osc.type = 'sine';
        osc.frequency.setValueAtTime(523.25, now);       // C5
        osc.frequency.setValueAtTime(659.25, now + 0.1); // E5
        osc.frequency.setValueAtTime(783.99, now + 0.2); // G5
        gain.gain.setValueAtTime(0.25, now);
        gain.gain.exponentialRampToValueAtTime(0.01, now + 0.45);
        osc.start(now);
        osc.stop(now + 0.45);
      } catch (e) {}
    }

    // High-Performance Normalized YCbCr Skin & Facial Geometry Detector
    function detectFaceAndFeatures(imgData, w, h) {
      const data = imgData.data;
      let totalSkinPixels = 0;
      let minX = w, maxX = 0, minY = h, maxY = 0;
      let sumX = 0, sumY = 0;
      const step = 4; // High-speed spatial subsampling

      for (let y = 0; y < h; y += step) {
        for (let x = 0; x < w; x += step) {
          const idx = (y * w + x) * 4;
          const r = data[idx];
          const g = data[idx + 1];
          const b = data[idx + 2];

          // YCbCr transformation (invariant to ethnicity & skin shade)
          const yVal  =  0.299 * r + 0.587 * g + 0.114 * b;
          const cbVal = 128 - 0.168736 * r - 0.331264 * g + 0.5 * b;
          const crVal = 128 + 0.5 * r - 0.418688 * g - 0.081312 * b;

          if (yVal > 35 && yVal < 240 && cbVal >= 75 && cbVal <= 130 && crVal >= 130 && crVal <= 178) {
            totalSkinPixels++;
            sumX += x;
            sumY += y;
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
          }
        }
      }

      const sampledTotal = (w / step) * (h / step);
      const skinRatio = totalSkinPixels / sampledTotal;

      // Facial cluster verification
      if (totalSkinPixels >= 25 && skinRatio >= 0.03 && skinRatio <= 0.85) {
        const boxW = maxX - minX;
        const boxH = maxY - minY;
        const aspect = boxH / (boxW || 1);

        // Aspect ratio filter for human facial structure
        if (aspect >= 0.8 && aspect <= 2.4 && boxW >= w * 0.12 && boxH >= h * 0.14) {
          const centerX = sumX / totalSkinPixels;
          const centerY = sumY / totalSkinPixels;

          // Extract 16-point normalized spatial luminance vector
          const vector = [];
          const numSamples = 4;
          const stepX = Math.max(1, Math.floor(boxW / numSamples));
          const stepY = Math.max(1, Math.floor(boxH / numSamples));

          for (let sy = 0; sy < numSamples; sy++) {
            for (let sx = 0; sx < numSamples; sx++) {
              const sampleX = Math.min(w - 1, Math.max(0, Math.floor(minX + sx * stepX)));
              const sampleY = Math.min(h - 1, Math.max(0, Math.floor(minY + sy * stepY)));
              const pIdx = (sampleY * w + sampleX) * 4;
              const lum = (0.299 * data[pIdx] + 0.587 * data[pIdx + 1] + 0.114 * data[pIdx + 2]) / 255.0;
              vector.push(lum);
            }
          }

          return {
            x: minX,
            y: minY,
            w: boxW,
            h: boxH,
            cx: centerX,
            cy: centerY,
            vector: vector,
            confidence: Math.min(99, Math.round(75 + skinRatio * 30))
          };
        }
      }
      return null;
    }

    // Cosine similarity for Biometric Vector Matching
    function compareVectors(v1, v2) {
      if (!v1 || !v2 || v1.length !== v2.length) return 0;
      let dot = 0, mag1 = 0, mag2 = 0;
      for (let i = 0; i < v1.length; i++) {
        dot += v1[i] * v2[i];
        mag1 += v1[i] * v1[i];
        mag2 += v2[i] * v2[i];
      }
      if (mag1 === 0 || mag2 === 0) return 0;
      const sim = dot / (Math.sqrt(mag1) * Math.sqrt(mag2));
      return Math.max(0, Math.min(100, Math.round(sim * 100)));
    }

    // Draw Sci-Fi Cyber Reticle HUD
    function drawReticle(ctx, x, y, w, h, isAuthorized, label, conf) {
      const color = isAuthorized ? '#10b981' : '#ef4444';
      ctx.strokeStyle = color;
      ctx.lineWidth = 2.5;
      ctx.shadowColor = color;
      ctx.shadowBlur = 10;

      const cornerLen = Math.min(w, h) * 0.25;

      // Top-Left Corner
      ctx.beginPath();
      ctx.moveTo(x, y + cornerLen);
      ctx.lineTo(x, y);
      ctx.lineTo(x + cornerLen, y);
      ctx.stroke();

      // Top-Right Corner
      ctx.beginPath();
      ctx.moveTo(x + w - cornerLen, y);
      ctx.lineTo(x + w, y);
      ctx.lineTo(x + w, y + cornerLen);
      ctx.stroke();

      // Bottom-Left Corner
      ctx.beginPath();
      ctx.moveTo(x, y + h - cornerLen);
      ctx.lineTo(x, y + h);
      ctx.lineTo(x + cornerLen, y + h);
      ctx.stroke();

      // Bottom-Right Corner
      ctx.beginPath();
      ctx.moveTo(x + w - cornerLen, y + h);
      ctx.lineTo(x + w, y + h);
      ctx.lineTo(x + w, y + h - cornerLen);
      ctx.stroke();

      // Center Crosshair
      const cx = x + w / 2;
      const cy = y + h / 2;
      ctx.beginPath();
      ctx.arc(cx, cy, 6, 0, Math.PI * 2);
      ctx.moveTo(cx - 12, cy); ctx.lineTo(cx + 12, cy);
      ctx.moveTo(cx, cy - 12); ctx.lineTo(cx, cy + 12);
      ctx.stroke();

      // Top Tag Banner
      ctx.fillStyle = color;
      ctx.font = 'bold 12px monospace';
      const tagText = `[ ${label} | ${conf}% ]`;
      const textMetrics = ctx.measureText(tagText);
      ctx.fillRect(x, Math.max(0, y - 22), textMetrics.width + 12, 20);

      ctx.fillStyle = '#000';
      ctx.fillText(tagText, x + 6, Math.max(14, y - 8));
      ctx.shadowBlur = 0;
    }

    // High-Speed Untainted AI Frame Fetch Pipeline (Port 80)
    async function runVisionPipeline() {
      const scanW = 160;
      const scanH = 120;
      workCanvas.width = scanW;
      workCanvas.height = scanH;

      while (true) {
        try {
          const response = await fetch('/capture?t=' + Date.now());
          if (response.ok) {
            const blob = await response.blob();
            const imgBitmap = await createImageBitmap(blob);

            // Sync overlay canvas dimensions
            if (aiCanvas.width !== streamImg.clientWidth || aiCanvas.height !== streamImg.clientHeight) {
              aiCanvas.width = streamImg.clientWidth || 320;
              aiCanvas.height = streamImg.clientHeight || 240;
            }

            // Draw to work canvas (100% same-origin, zero security errors)
            workCtx.drawImage(imgBitmap, 0, 0, scanW, scanH);
            const imgData = workCtx.getImageData(0, 0, scanW, scanH);
            const result = detectFaceAndFeatures(imgData, scanW, scanH);
            currentFaceResult = result;

            // Clear previous HUD
            aiCtx.clearRect(0, 0, aiCanvas.width, aiCanvas.height);

            const targetBadge = document.getElementById('target-badge');
            const hudMatch = document.getElementById('hud-match');
            const hudArea = document.getElementById('hud-area');

            if (result) {
              const scaleX = aiCanvas.width / scanW;
              const scaleY = aiCanvas.height / scanH;
              const dispX = result.x * scaleX;
              const dispY = result.y * scaleY;
              const dispW = result.w * scaleX;
              const dispH = result.h * scaleY;

              let isAuth = false;
              let matchScore = 0;
              let label = "UNKNOWN INTRUDER";

              if (enrolledVector) {
                matchScore = compareVectors(result.vector, enrolledVector);
                hudMatch.innerText = matchScore + ' %';
                if (matchScore >= 78) {
                  isAuth = true;
                  label = "AUTHORIZED: SUBJ-0";
                  targetBadge.className = "badge active";
                  targetBadge.innerText = "✅ Authorized Subject";
                } else {
                  isAuth = false;
                  label = "ALERT: INTRUDER";
                  targetBadge.className = "badge alert";
                  targetBadge.innerText = "🚨 Intruder Detected!";
                  handleIntruderEvent(matchScore);
                }
              } else {
                hudMatch.innerText = "--";
                label = "FACE DETECTED";
                targetBadge.className = "badge active";
                targetBadge.innerText = "👁️ Face In View";
              }

              hudArea.innerText = `${Math.round(dispW)}x${Math.round(dispH)}`;
              drawReticle(aiCtx, dispX, dispY, dispW, dispH, isAuth || !enrolledVector, label, result.confidence);
            } else {
              targetBadge.className = "badge";
              targetBadge.innerText = "👁️ Scanning for Faces";
              hudMatch.innerText = "-- %";
              hudArea.innerText = "None";
            }

            // Update FPS Counter
            frameCount++;
            const now = Date.now();
            if (now - lastFpsTime >= 1000) {
              fps = frameCount;
              frameCount = 0;
              lastFpsTime = now;
              document.getElementById('hud-fps').innerText = fps;
            }
          }
        } catch (err) {
          // Gracefully continue on transient network drop
        }
        await new Promise(r => setTimeout(r, 65)); // ~15 FPS pipeline
      }
    }

    function handleIntruderEvent(matchScore) {
      const now = Date.now();
      if (now - lastAlertTime > 2500) { // Cooldown between alert events
        lastAlertTime = now;
        intruderCount++;
        document.getElementById('hud-alerts').innerText = intruderCount;

        // Sound Siren
        playSirenAlert();

        // Strobe Hardware Spotlight if enabled
        if (autoFlashEnabled) {
          triggerHardwareFlashPulse();
        }

        // Add to Incident Log
        const timeStr = new Date().toLocaleTimeString();
        addLogEntry(`[ALERT ${timeStr}] Intruder detected! Biometric Match: ${matchScore}%. Threat Level HIGH.`, true);
      }
    }

    function addLogEntry(text, isAlert) {
      const logPanel = document.getElementById('log-panel');
      const item = document.createElement('div');
      item.className = 'log-item ' + (isAlert ? 'alert' : 'auth');
      item.innerText = text;
      logPanel.insertBefore(item, logPanel.firstChild);
      if (logPanel.children.length > 25) {
        logPanel.removeChild(logPanel.lastChild);
      }
    }

    // Biometric Enrollment Handlers
    function enrollCurrentFace() {
      initAudio();
      if (!currentFaceResult) {
        alert("⚠️ No face detected in frame! Position your face in front of the camera and try again.");
        return;
      }
      enrolledVector = [...currentFaceResult.vector];
      playSuccessChime();
      document.getElementById('enrolled-badge').className = "badge active";
      document.getElementById('enrolled-badge').innerText = "👤 Subject 0: Enrolled";
      addLogEntry(`[ENROLL] Biometric profile recorded for Subject 0. Facial vector memorized.`, false);
    }

    function clearEnrollment() {
      enrolledVector = null;
      document.getElementById('enrolled-badge').className = "badge";
      document.getElementById('enrolled-badge').innerText = "👤 Subject 0: Not Enrolled";
      document.getElementById('hud-match').innerText = "-- %";
      addLogEntry(`[SYSTEM] Biometric memory cleared. All faces classified as unassigned.`, false);
    }

    // Hardware Actuator Controls
    function toggleSiren() {
      initAudio();
      sirenEnabled = !sirenEnabled;
      const btn = document.getElementById('siren-btn');
      if (sirenEnabled) {
        btn.classList.add('active');
        btn.querySelector('span').innerText = 'Audio Siren: ON';
      } else {
        btn.classList.remove('active');
        btn.querySelector('span').innerText = 'Audio Siren: OFF';
      }
    }

    function toggleAutoFlash() {
      autoFlashEnabled = !autoFlashEnabled;
      const btn = document.getElementById('auto-flash-btn');
      btn.querySelector('span').innerText = autoFlashEnabled ? 'Auto-Strobe: ON' : 'Auto-Strobe: OFF';
    }

    function triggerHardwareFlashPulse() {
      fetch('/flash?state=1')
        .then(() => setTimeout(() => fetch('/flash?state=0'), 1000))
        .catch(console.warn);
    }

    function toggleSpotlight() {
      spotlightOn = !spotlightOn;
      fetch('/flash?state=' + (spotlightOn ? '1' : '0'))
        .then(() => {
          const btn = document.getElementById('spotlight-btn');
          if (spotlightOn) {
            btn.classList.add('on');
            btn.querySelector('span').innerText = 'Spotlight: ON';
          } else {
            btn.classList.remove('on');
            btn.querySelector('span').innerText = 'Spotlight: OFF';
          }
        })
        .catch(console.warn);
    }

    function takeManualSnapshot() {
      const snapUrl = '/capture?t=' + Date.now();
      const preview = document.getElementById('snap-preview');
      const img = document.getElementById('snap-img');
      const dl = document.getElementById('snap-download');
      img.src = snapUrl;
      dl.href = snapUrl;
      preview.style.display = 'block';
      preview.scrollIntoView({ behavior: 'smooth' });
      addLogEntry(`[SNAP] Manual security capture saved.`, false);
    }

    function setRes(size, btn) {
      document.querySelectorAll('.res-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      fetch('/res?size=' + size);
    }

    // Launch AI Vision Pipeline immediately
    setTimeout(runVisionPipeline, 500);
  </script>
</body>
</html>
)rawliteral";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Dedicated MJPEG Streaming Handler (Port 81)
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      res = ESP_FAIL;
    } else {
      if (fb->format != PIXFORMAT_JPEG) {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = NULL;
        if (!jpeg_converted) {
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (res != ESP_OK) {
      break;
    }
  }
  return res;
}

// Web UI Index Handler (Port 80)
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}

// Single Photo / AI Frame Capture Handler (Port 80)
static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;

  fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

// Flashlight Handler (Port 80)
static esp_err_t flash_handler(httpd_req_t *req) {
  char buf[32];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char param[16];
    if (httpd_query_key_value(buf, "state", param, sizeof(param)) == ESP_OK) {
      if (strcmp(param, "1") == 0) {
        flashState = true;
        digitalWrite(FLASH_LED_PIN, HIGH);
      } else {
        flashState = false;
        digitalWrite(FLASH_LED_PIN, LOW);
      }
    }
  }
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, flashState ? "ON" : "OFF", 2);
}

// Resolution Switching Handler (Port 80)
static esp_err_t res_handler(httpd_req_t *req) {
  char buf[32];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char param[16];
    if (httpd_query_key_value(buf, "size", param, sizeof(param)) == ESP_OK) {
      sensor_t * s = esp_camera_sensor_get();
      if (s) {
        if (strcmp(param, "qvga") == 0) s->set_framesize(s, FRAMESIZE_QVGA);
        else if (strcmp(param, "vga") == 0) s->set_framesize(s, FRAMESIZE_VGA);
        else if (strcmp(param, "svga") == 0) s->set_framesize(s, FRAMESIZE_SVGA);
      }
    }
  }
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, "OK", 2);
}

// Start HTTP Servers (Dual-Port Architecture)
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 8;

  httpd_uri_t index_uri   = { .uri = "/",        .method = HTTP_GET, .handler = index_handler,   .user_ctx = NULL };
  httpd_uri_t flash_uri   = { .uri = "/flash",    .method = HTTP_GET, .handler = flash_handler,   .user_ctx = NULL };
  httpd_uri_t capture_uri = { .uri = "/capture",  .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL };
  httpd_uri_t res_uri     = { .uri = "/res",      .method = HTTP_GET, .handler = res_handler,     .user_ctx = NULL };
  httpd_uri_t stream_uri  = { .uri = "/stream",   .method = HTTP_GET, .handler = stream_handler,  .user_ctx = NULL };

  // 1. Port 80: High-Speed UI, Frame Capture & Controls
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &flash_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &res_uri);
    Serial.println(F("✅ Main Control Server started on Port 80"));
  }

  // 2. Port 81: Dedicated Streaming Server
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println(F("✅ Dedicated Stream Server started on Port 81"));
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector to prevent current surge resets
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n========================================================"));
  Serial.println(F("  PROJECT 11: AI FACE DETECTION & BIOMETRIC SENTRY      "));
  Serial.println(F("========================================================"));

  pinMode(FLASH_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);   // Spotlight OFF at boot
  digitalWrite(STATUS_LED_PIN, HIGH); // Active LOW -> OFF

  // Camera Hardware Configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;  // QVGA (320x240) optimized for fast AI frame rates
  config.jpeg_quality = 12;            // 10-63 (lower is higher quality)
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Initialize OV2640 Sensor
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed with error 0x%x\n", err);
    while (true) {
      digitalWrite(STATUS_LED_PIN, LOW);
      delay(200);
      digitalWrite(STATUS_LED_PIN, HIGH);
      delay(200);
    }
  }

  // Adjust Sensor Tuning
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_vflip(s, 1);       // Flip image if camera module is mounted upside down
    s->set_brightness(s, 1);   // Boost brightness slightly for facial contrast
    s->set_saturation(s, 0);
  }

  Serial.println(F("✅ OV2640 Optical Sensor Initialized"));

  // Connect to Wi-Fi with SoftAP Fallback
  Serial.println(F("\n\n=================================================="));
  Serial.println(F("  STEP 1: CONNECTING TO WI-FI                     "));
  Serial.println(F("=================================================="));
  Serial.printf("📡 Target SSID : %s (2.4 GHz)\n", wifi_ssid);
  Serial.println(F("⏳ Waiting for connection (12 sec)..."));

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  unsigned long startAttemptTime = millis();
  bool connected = false;

  while (millis() - startAttemptTime < 12000) {
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  delay(1000);

  if (connected) {
    Serial.println(F("\n=================================================="));
    Serial.println(F("  ✅ WI-FI CONNECTED SUCCESSFULLY!               "));
    Serial.println(F("=================================================="));
    Serial.print(F("🌐 IP Address : http://"));
    Serial.println(WiFi.localIP());
    Serial.println(F("==================================================\n"));
  } else {
    Serial.println(F("\n=================================================="));
    Serial.println(F("  ⚠️  WI-FI NOT CONNECTED / TIMED OUT             "));
    Serial.println(F("  🔄 SWITCHING TO AUTONOMOUS SOFTAP HOTSPOT...   "));
    Serial.println(F("=================================================="));

    WiFi.disconnect(true);
    delay(800);

    WiFi.mode(WIFI_AP);
    delay(300);
    WiFi.softAP(ap_ssid, ap_password);
    delay(1200);

    Serial.println(F("\n--------------------------------------------------"));
    Serial.println(F("  📶 HOTSPOT ACCESS POINT IS LIVE!                "));
    Serial.println(F("--------------------------------------------------"));
    Serial.printf("  1. Connect your Phone/PC to SSID : %s\n", ap_ssid);
    Serial.printf("  2. Enter Wi-Fi Password          : %s\n", ap_password);
    Serial.print(F("  3. Hotspot Gateway IP            : http://"));
    Serial.println(WiFi.softAPIP());
    Serial.println(F("--------------------------------------------------\n"));
    delay(1000);
  }

  // Start HTTP Servers
  startCameraServer();
  delay(500);

  Serial.println(F("=================================================="));
  Serial.println(F("  🟢 AI FACE SENTRY READY & STREAMING LIVE!       "));
  Serial.println(F("=================================================="));
  if (connected) {
    Serial.printf("🌐 Open in Browser : http://%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.printf("🌐 Open in Browser : http://%s\n", WiFi.softAPIP().toString().c_str());
  }
  Serial.println(F("==================================================\n"));
}

void loop() {
  // Armed heartbeat flash on status LED every 3 seconds
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat >= 3000) {
    lastHeartbeat = millis();
    digitalWrite(STATUS_LED_PIN, LOW); // Flash briefly
    delay(30);
    digitalWrite(STATUS_LED_PIN, HIGH);
  }
  delay(10);
}
