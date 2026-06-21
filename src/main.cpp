#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ============================================
// Configuration - UPDATE THESE VALUES
// ============================================
const char* WIFI_SSID = "Linksys";        // Change to your WiFi name
const char* WIFI_PASSWORD = "p1cass00"; // Change to your WiFi password

// Pin Definitions
const int LED_FEED_PIN = 2;     // GPIO2 for feed indicator LED
const int LED_STATUS_PIN = 8;   // GPIO8 for status LED (built-in on some boards)
const int LED_WIFI_PIN = 10;    // GPIO10 for WiFi indicator LED (optional)

// LED Blink Configuration
const int FEED_BLINK_TIMES = 5;      // Number of times to blink when feeding
const int FEED_BLINK_DURATION_MS = 200; // How long each blink lasts

// ============================================
// Global Objects
// ============================================
WebServer server(80);

// ============================================
// Function Declarations
// ============================================
void setupWiFi();
void setupWebServer();
void handleRoot();
void handleFeed();
void handleStatus();
void handleNotFound();
void dispenseFoodOnce();
void blinkLED(int pin, int times, int delayMs);

// ============================================
// Setup Function
// ============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("Fish Feeder Starting...");
  Serial.println("=================================");
  
  // Initialize LED pins
  pinMode(LED_FEED_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(LED_WIFI_PIN, OUTPUT);
  digitalWrite(LED_FEED_PIN, LOW);
  digitalWrite(LED_STATUS_PIN, LOW);
  digitalWrite(LED_WIFI_PIN, LOW);
  
  Serial.println("✓ LED pins initialized");
  
  // Connect to WiFi
  setupWiFi();
  
  // Start web server
  setupWebServer();
  
  // Indicate ready
  blinkLED(LED_STATUS_PIN, 3, 200);
  Serial.println("=================================");
  Serial.println("Fish Feeder Ready!");
  Serial.println("=================================\n");
}

// ============================================
// Main Loop
// ============================================
void loop() {
  server.handleClient();
}

// ============================================
// WiFi Setup
// ============================================
void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_WIFI_PIN, !digitalRead(LED_WIFI_PIN)); // Blink while connecting
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_WIFI_PIN, HIGH); // Solid when connected
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    digitalWrite(LED_WIFI_PIN, LOW);
    Serial.println("\n✗ WiFi Connection Failed!");
    Serial.println("  Please check SSID and password in the code");
  }
}

// ============================================
// Web Server Setup
// ============================================
void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/feed", HTTP_POST, handleFeed);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("✓ Web server started");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("  Access at: http://");
    Serial.println(WiFi.localIP());
  }
}

// ============================================
// Web Server Handlers
// ============================================
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Fish Feeder Control</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    .container {
      background: white;
      border-radius: 20px;
      padding: 40px;
      box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
      max-width: 400px;
      width: 100%;
      text-align: center;
    }
    h1 {
      color: #333;
      margin-bottom: 10px;
      font-size: 2em;
    }
    .emoji {
      font-size: 4em;
      margin: 20px 0;
    }
    .status {
      background: #f0f0f0;
      padding: 15px;
      border-radius: 10px;
      margin: 20px 0;
    }
    .status-item {
      display: flex;
      justify-content: space-between;
      margin: 8px 0;
      font-size: 0.95em;
    }
    .status-label {
      color: #666;
    }
    .status-value {
      color: #333;
      font-weight: 600;
    }
    .btn {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border: none;
      padding: 18px 40px;
      font-size: 1.2em;
      border-radius: 50px;
      cursor: pointer;
      width: 100%;
      margin-top: 20px;
      transition: transform 0.2s, box-shadow 0.2s;
      font-weight: 600;
    }
    .btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 10px 20px rgba(0, 0, 0, 0.2);
    }
    .btn:active {
      transform: translateY(0);
    }
    .btn:disabled {
      opacity: 0.6;
      cursor: not-allowed;
    }
    .message {
      margin-top: 15px;
      padding: 12px;
      border-radius: 8px;
      font-size: 0.9em;
      display: none;
    }
    .message.success {
      background: #d4edda;
      color: #155724;
      border: 1px solid #c3e6cb;
    }
    .message.error {
      background: #f8d7da;
      color: #721c24;
      border: 1px solid #f5c6cb;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="emoji">🐠</div>
    <h1>Fish Feeder</h1>
    <p style="color: #666; margin-bottom: 20px;">ESP32-C3 Controller</p>
    
    <div class="status">
      <div class="status-item">
        <span class="status-label">WiFi Signal:</span>
        <span class="status-value" id="wifi">--</span>
      </div>
      <div class="status-item">
        <span class="status-label">Uptime:</span>
        <span class="status-value" id="uptime">--</span>
      </div>
      <div class="status-item">
        <span class="status-label">Free Memory:</span>
        <span class="status-value" id="memory">--</span>
      </div>
    </div>
    
    <button class="btn" onclick="feedFish()" id="feedBtn">
      Feed Fish Now 🐟
    </button>
    
    <div class="message" id="message"></div>
  </div>

  <script>
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          document.getElementById('wifi').textContent = data.rssi + ' dBm';
          document.getElementById('uptime').textContent = formatUptime(data.uptime);
          document.getElementById('memory').textContent = (data.freeHeap / 1024).toFixed(1) + ' KB';
        })
        .catch(err => console.error('Status update failed:', err));
    }
    
    function formatUptime(seconds) {
      const hours = Math.floor(seconds / 3600);
      const minutes = Math.floor((seconds % 3600) / 60);
      const secs = seconds % 60;
      return `${hours}h ${minutes}m ${secs}s`;
    }
    
    function feedFish() {
      const btn = document.getElementById('feedBtn');
      const msg = document.getElementById('message');
      
      btn.disabled = true;
      btn.textContent = 'Feeding... 🐟';
      msg.style.display = 'none';
      
      fetch('/feed', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
          msg.className = 'message success';
          msg.textContent = '✓ ' + data.message;
          msg.style.display = 'block';
          btn.textContent = 'Feed Fish Now 🐟';
          btn.disabled = false;
        })
        .catch(err => {
          msg.className = 'message error';
          msg.textContent = '✗ Failed to feed fish';
          msg.style.display = 'block';
          btn.textContent = 'Feed Fish Now 🐟';
          btn.disabled = false;
        });
    }
    
    // Update status every 5 seconds
    updateStatus();
    setInterval(updateStatus, 5000);
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleFeed() {
  Serial.println("\n>>> Feed Request Received");
  
  blinkLED(LED_STATUS_PIN, 1, 100);
  dispenseFoodOnce();
  
  String response = "{\"success\": true, \"message\": \"Food dispensed successfully!\"}";
  server.send(200, "application/json", response);
  
  Serial.println("<<< Feed Request Completed\n");
}

void handleStatus() {
  String json = "{";
  json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap());
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

// ============================================
// Food Dispensing Function
// ============================================
void dispenseFoodOnce() {
  Serial.println("  → Blinking LED to simulate feeding...");
  
  // Blink the feed LED multiple times
  for (int i = 0; i < FEED_BLINK_TIMES; i++) {
    digitalWrite(LED_FEED_PIN, HIGH);
    digitalWrite(LED_STATUS_PIN, HIGH);
    delay(FEED_BLINK_DURATION_MS);
    
    digitalWrite(LED_FEED_PIN, LOW);
    digitalWrite(LED_STATUS_PIN, LOW);
    delay(FEED_BLINK_DURATION_MS);
  }
  
  Serial.println("  ✓ Feed simulation complete");
}

// ============================================
// LED Helper Function
// ============================================
void blinkLED(int pin, int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(delayMs);
    digitalWrite(pin, LOW);
    delay(delayMs);
  }
}