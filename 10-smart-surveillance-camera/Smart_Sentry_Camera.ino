#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

/*
 * =====================================================================================
 * PROJECT 10: Smart Wi-Fi Video Security Sentry & Photo Trap
 * Board: AI Thinker ESP32-CAM (with ESP32-CAM-MB USB programmer shield)
 * Features:
 *   - Autonomous Access Point: Creates "ESP32-CAM-Sentry" Wi-Fi Hotspot
 *   - Real-time MJPEG Video Streaming directly in browser
 *   - On-demand High-Power Spotlight (Flash LED on GPIO 4)
 *   - Single-click Full Resolution Photo Capture
 *   - Interactive Responsive Dark-Mode Web Dashboard
 * =====================================================================================
 */

// --- OPTIONAL: Enter your home Wi-Fi credentials below ---
// If left as-is, the camera will still broadcast its own Wi-Fi Hotspot ("ESP32-CAM-Sentry")!
const char* wifi_ssid = "";      // e.g. "YourHomeWiFi"
const char* wifi_password = "";  // e.g. "YourWiFiPassword"

// --- Access Point Credentials (Connect phone/PC directly to this!) ---
const char* ap_ssid = "ESP32-CAM-Sentry";
const char* ap_password = "password123";

// --- AI-THINKER ESP32-CAM PIN CONFIGURATION ---
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

#define FLASH_LED_PIN      4   // Bright White Flash LED
#define STATUS_LED_PIN    33   // Small Red Indicator LED (Active LOW)

// HTTP Server Handles
httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;
bool flashState = false;

// Web Dashboard HTML
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32-CAM Security Sentry</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }
    body { background: #0f172a; color: #f8fafc; text-align: center; padding: 16px; }
    .container { max-width: 720px; margin: 0 auto; }
    header { margin-bottom: 20px; }
    h1 { font-size: 1.6rem; color: #38bdf8; display: flex; align-items: center; justify-content: center; gap: 8px; }
    .badge { background: #1e293b; border: 1px solid #334155; padding: 4px 12px; border-radius: 9999px; font-size: 0.8rem; color: #94a3b8; display: inline-block; margin-top: 6px; }
    .stream-box { background: #000; border: 2px solid #334155; border-radius: 12px; overflow: hidden; position: relative; box-shadow: 0 10px 25px rgba(0,0,0,0.5); min-height: 240px; display: flex; align-items: center; justify-content: center; }
    #stream { width: 100%; height: auto; display: block; border-radius: 10px; }
    .controls { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-top: 20px; }
    button { background: #1e293b; border: 1px solid #475569; color: #f8fafc; padding: 14px 18px; border-radius: 10px; font-size: 1rem; font-weight: 600; cursor: pointer; transition: all 0.2s; display: flex; align-items: center; justify-content: center; gap: 8px; }
    button:hover { background: #334155; border-color: #64748b; }
    button:active { transform: scale(0.97); }
    .btn-flash { background: #ca8a04; border-color: #eab308; }
    .btn-flash.on { background: #eab308; color: #000; box-shadow: 0 0 15px #eab308; }
    .btn-snap { background: #2563eb; border-color: #3b82f6; }
    .btn-snap:hover { background: #1d4ed8; }
    .resolution-bar { margin-top: 16px; background: #1e293b; padding: 12px; border-radius: 10px; display: flex; justify-content: space-around; align-items: center; }
    .res-btn { padding: 6px 12px; font-size: 0.85rem; border-radius: 6px; background: #334155; border: none; }
    .res-btn.active { background: #38bdf8; color: #0f172a; font-weight: bold; }
    #snap-view { margin-top: 20px; display: none; background: #1e293b; padding: 16px; border-radius: 12px; }
    #snap-view img { max-width: 100%; border-radius: 8px; border: 1px solid #475569; margin-top: 10px; }
    footer { margin-top: 30px; font-size: 0.8rem; color: #64748b; }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>🚨 ESP32-CAM Sentry Hub</h1>
      <span class="badge">🟢 Live Feed Active &bull; Low Latency MJPEG</span>
    </header>

    <div class="stream-box">
      <img id="stream" src="/stream" alt="Live Camera Stream">
    </div>

    <div class="controls">
      <button id="flash-btn" class="btn-flash" onclick="toggleFlash()">
        💡 <span>Spotlight: OFF</span>
      </button>
      <button class="btn-snap" onclick="takeSnapshot()">
        📸 <span>Snap Photo</span>
      </button>
    </div>

    <div class="resolution-bar">
      <span style="font-size:0.85rem; color:#94a3b8;">Resolution:</span>
      <button class="res-btn" onclick="setRes('qvga', this)">QVGA (Fast)</button>
      <button class="res-btn active" onclick="setRes('vga', this)">VGA (Smooth)</button>
      <button class="res-btn" onclick="setRes('svga', this)">SVGA (Crisp)</button>
    </div>

    <div id="snap-view">
      <h3>📸 Latest Intruder Snapshot</h3>
      <img id="snap-img" src="" alt="Snapshot">
      <div style="margin-top:10px;">
        <a id="snap-download" href="" download="sentry_snap.jpg"><button style="width:100%;">💾 Download Image</button></a>
      </div>
    </div>

    <footer>
      Project 10: Multi-Zone IoT Portfolio &bull; AI-Thinker OV2640 Module
    </footer>
  </div>

  <script>
    let flash = false;
    function toggleFlash() {
      flash = !flash;
      fetch('/flash?state=' + (flash ? '1' : '0'))
        .then(res => res.text())
        .then(txt => {
          const btn = document.getElementById('flash-btn');
          if (flash) {
            btn.classList.add('on');
            btn.querySelector('span').innerText = 'Spotlight: ON';
          } else {
            btn.classList.remove('on');
            btn.querySelector('span').innerText = 'Spotlight: OFF';
          }
        });
    }

    function takeSnapshot() {
      const snapUrl = '/capture?t=' + Date.now();
      const snapView = document.getElementById('snap-view');
      const snapImg = document.getElementById('snap-img');
      const snapDl = document.getElementById('snap-download');
      
      snapImg.src = snapUrl;
      snapDl.href = snapUrl;
      snapView.style.display = 'block';
      snapView.scrollIntoView({ behavior: 'smooth' });
    }

    function setRes(resName, el) {
      document.querySelectorAll('.res-btn').forEach(b => b.classList.remove('active'));
      el.classList.add('active');
      fetch('/res?size=' + resName);
    }
  </script>
</body>
</html>
)rawliteral";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Stream Handler
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println(F("Camera capture failed"));
      res = ESP_FAIL;
    } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
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

// Web UI Index Handler
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}

// Single Photo Capture Handler
static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;

  // Flash blink for photo effect if desired
  digitalWrite(STATUS_LED_PIN, LOW); // Active LOW
  fb = esp_camera_fb_get();
  digitalWrite(STATUS_LED_PIN, HIGH);

  if (!fb) {
    Serial.println(F("Camera capture failed"));
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

// Flashlight Toggle Handler
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
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, flashState ? "ON" : "OFF", 2);
}

// Resolution Switching Handler
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
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, "OK", 2);
}

// Start HTTP Servers
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 8;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t flash_uri = {
    .uri       = "/flash",
    .method    = HTTP_GET,
    .handler   = flash_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t capture_uri = {
    .uri       = "/capture",
    .method    = HTTP_GET,
    .handler   = capture_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t res_uri = {
    .uri       = "/res",
    .method    = HTTP_GET,
    .handler   = res_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  // Main UI Server (Port 80)
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &flash_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &res_uri);
    httpd_register_uri_handler(camera_httpd, &stream_uri);
    Serial.println(F("✅ HTTP Web Server started successfully"));
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable hardware brownout detector (prevents boot crash loops)
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n========================================================"));
  Serial.println(F("  PROJECT 10: ESP32-CAM SMART VIDEO SECURITY SENTRY     "));
  Serial.println(F("========================================================"));

  pinMode(FLASH_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);   // Keep flash OFF at boot (prevents current spike)
  digitalWrite(STATUS_LED_PIN, HIGH); // Active LOW -> OFF

  Serial.println(F("⚡ Brownout detector bypassed. Initializing Camera..."));

  // Camera Configuration
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // PSRAM Check for high quality
  if (psramFound()) {
    Serial.println(F("🚀 PSRAM Detected: Enabling High Performance VGA Streaming"));
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    Serial.println(F("⚠️ No PSRAM: Defaulting to QVGA (Fast Mode)"));
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera Init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera Init Failed with error 0x%x\n", err);
    return;
  }
  Serial.println(F("📷 Camera Module (OV2640) Initialized OK!"));

  // --- WI-FI CONFIGURATION ---
  WiFi.mode(WIFI_AP_STA);

  // 1. Start Autonomous SoftAP (Hotspot)
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress apIP = WiFi.softAPIP();
  Serial.println(F("\n--------------------------------------------------------"));
  Serial.print(F("📡 [Hotspot Mode] Wi-Fi SSID : ")); Serial.println(ap_ssid);
  Serial.print(F("🔒 [Hotspot Mode] Password  : ")); Serial.println(ap_password);
  Serial.print(F("🌐 [Hotspot Mode] Web URL   : http://")); Serial.println(apIP);
  Serial.println(F("--------------------------------------------------------"));

  // 2. Connect to Home Wi-Fi (if provided)
  if (strlen(wifi_ssid) > 0) {
    Serial.print(F("Connecting to Local Wi-Fi: "));
    Serial.println(wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);
    
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("\n✅ Connected to Home Wi-Fi!"));
      Serial.print(F("🌐 [Local Network URL]: http://"));
      Serial.println(WiFi.localIP());
    } else {
      Serial.println(F("\n⚠️ Home Wi-Fi not connected (Use the Hotspot URL above!)"));
    }
  }

  // Start HTTP Streaming Server
  startCameraServer();

  Serial.println(F("\n========================================================"));
  Serial.println(F("  SENTRY SYSTEM READY!                                  "));
  Serial.println(F("  1. Connect your Phone or PC to Wi-Fi: ESP32-CAM-Sentry"));
  Serial.println(F("  2. Open your browser and go to: http://192.168.4.1    "));
  Serial.println(F("========================================================\n"));
}

void loop() {
  static unsigned long lastBanner = 0;
  if (millis() - lastBanner > 3000) {
    lastBanner = millis();
    Serial.println(F("--------------------------------------------------"));
    Serial.println(F("🟢 SENTRY READY & STREAMING LIVE!"));
    Serial.println(F("📱 Connect Wi-Fi : ESP32-CAM-Sentry (Pass: password123)"));
    Serial.println(F("🌐 Open Browser  : http://192.168.4.1"));
    Serial.println(F("--------------------------------------------------\n"));
  }
  delay(200);
}
