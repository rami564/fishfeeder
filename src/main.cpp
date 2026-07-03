#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

// ============================================
// Configuration - UPDATE THESE VALUES
// ============================================
const char* WIFI_SSID = "Linksys";        // Change to your WiFi name
const char* WIFI_PASSWORD = "p1cass00"; // Change to your WiFi password

// Protocol Selection: "TCP", "UDP", or "BOTH"
#define PROTOCOL_MODE "BOTH"  // Options: "TCP" (HTTP only), "UDP" (UDP only), "BOTH"

// UDP Configuration
const int UDP_PORT = 8888;  // UDP listening port

// OTA Configuration
const char* OTA_HOSTNAME = "fishfeeder";   // mDNS name (access via fishfeeder.local)
const char* OTA_PASSWORD = "fish2024";     // Password for OTA updates (CHANGE THIS!)

// Pin Definitions
const int LED_FEED_PIN = 2;     // GPIO2 for feed indicator LED
const int LED_STATUS_PIN = 8;   // GPIO8 for status LED (built-in on some boards)
const int LED_WIFI_PIN = 10;    // GPIO10 for WiFi indicator LED (optional)
const int MOTOR_CONTROL_PIN = 4; // GPIO4 for motor control (active low)

// LED Blink Configuration
const int FEED_BLINK_TIMES = 5;      // Number of times to blink when feeding
const int FEED_BLINK_DURATION_MS = 200; // How long each blink lasts

// ============================================
// Global Objects
// ============================================
WebServer server(80);
WiFiUDP udpServer;

// Helper macros for protocol selection
#define USE_TCP (strcmp(PROTOCOL_MODE, "TCP") == 0 || strcmp(PROTOCOL_MODE, "BOTH") == 0)
#define USE_UDP (strcmp(PROTOCOL_MODE, "UDP") == 0 || strcmp(PROTOCOL_MODE, "BOTH") == 0)

// ============================================
// Function Declarations
// ============================================
void setupWiFi();
void setupMDNS();
void setupOTA();
void setupWebServer();
void setupUDPServer();
void handleUDPPacket();
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
  
  // Initialize motor control pin (idle = HIGH)
  pinMode(MOTOR_CONTROL_PIN, OUTPUT);
  digitalWrite(MOTOR_CONTROL_PIN, HIGH);
  
  Serial.println("✓ LED pins initialized");
  Serial.println("✓ Motor control pin initialized");
  
  // Connect to WiFi
  setupWiFi();
  
  // Setup mDNS (fishfeeder.local)
  setupMDNS();
  
  // Setup OTA (Over-The-Air updates)
  setupOTA();
  
  // Start servers based on protocol mode
  Serial.print("Protocol Mode: ");
  Serial.println(PROTOCOL_MODE);
  
  if (USE_TCP) {
    setupWebServer();
  }
  
  if (USE_UDP) {
    setupUDPServer();
  }
  
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
  ArduinoOTA.handle();  // Handle OTA updates
  
  if (USE_TCP) {
    server.handleClient();
  }
  
  if (USE_UDP) {
    handleUDPPacket();
  }
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
// mDNS Setup
// ============================================
void setupMDNS() {
  if (MDNS.begin(OTA_HOSTNAME)) {
    Serial.println("✓ mDNS responder started");
    Serial.print("  Access via: http://");
    Serial.print(OTA_HOSTNAME);
    Serial.println(".local");
    
    // Add service to mDNS-SD
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("✗ Error setting up mDNS responder");
  }
}

// ============================================
// OTA Setup
// ============================================
void setupOTA() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("\n>>> OTA Update Starting (" + type + ")...");
    // Turn off WiFi LED during update
    digitalWrite(LED_WIFI_PIN, LOW);
    // Blink status LED rapidly during update
    digitalWrite(LED_STATUS_PIN, HIGH);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\n✓ OTA Update Complete!");
    digitalWrite(LED_STATUS_PIN, LOW);
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {  // Print every second
      Serial.printf("  Progress: %u%%\r", (progress / (total / 100)));
      // Blink LED during update
      digitalWrite(LED_STATUS_PIN, !digitalRead(LED_STATUS_PIN));
      lastPrint = millis();
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("✗ OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    
    // Blink error pattern (fast blinks)
    for (int i = 0; i < 5; i++) {
      blinkLED(LED_STATUS_PIN, 3, 100);
      delay(200);
    }
  });
  
  ArduinoOTA.begin();
  Serial.println("✓ OTA update service started");
  Serial.println("  Password protected for security");
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
    Serial.print(WiFi.localIP());
    Serial.print(" or http://");
    Serial.print(OTA_HOSTNAME);
    Serial.println(".local");
  }
}

// ============================================
// UDP Server Setup
// ============================================
void setupUDPServer() {
  if (udpServer.begin(UDP_PORT)) {
    Serial.println("✓ UDP server started");
    Serial.print("  Listening on port: ");
    Serial.println(UDP_PORT);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("  IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.println("\n  UDP Commands:");
      Serial.println("    'FEED' - Dispense food");
      Serial.println("    'STATUS' - Get device status");
    }
  } else {
    Serial.println("✗ Failed to start UDP server");
  }
}

// ============================================
// UDP Packet Handler
// ============================================
void handleUDPPacket() {
  int packetSize = udpServer.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udpServer.read(incomingPacket, 254);
    if (len > 0) {
      incomingPacket[len] = '\0';
    }
    
    IPAddress remoteIP = udpServer.remoteIP();
    uint16_t remotePort = udpServer.remotePort();
    
    Serial.print("\n>>> UDP Packet from ");
    Serial.print(remoteIP);
    Serial.print(":");
    Serial.print(remotePort);
    Serial.print(" - Command: ");
    Serial.println(incomingPacket);
    
    // Process command
    String command = String(incomingPacket);
    command.trim();
    command.toUpperCase();
    
    if (command == "FEED") {
      blinkLED(LED_STATUS_PIN, 1, 100);
      dispenseFoodOnce();
      
      // Send response
      String response = "{\"success\":true,\"message\":\"Food dispensed\"}";
      udpServer.beginPacket(remoteIP, remotePort);
      udpServer.print(response);
      udpServer.endPacket();
      
      Serial.println("  ✓ FEED command executed");
      
    } else if (command == "STATUS") {
      // Send status response
      String json = "{";
      json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
      json += "\"uptime\":" + String(millis() / 1000) + ",";
      json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
      json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
      json += "}";
      
      udpServer.beginPacket(remoteIP, remotePort);
      udpServer.print(json);
      udpServer.endPacket();
      
      Serial.println("  ✓ STATUS command executed");
      
    } else {
      // Unknown command
      String response = "{\"success\":false,\"error\":\"Unknown command\"}";
      udpServer.beginPacket(remoteIP, remotePort);
      udpServer.print(response);
      udpServer.endPacket();
      
      Serial.print("  ✗ Unknown command: ");
      Serial.println(incomingPacket);
    }
    
    Serial.println("<<< UDP Request Completed\n");
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
  Serial.println("  → Activating motor (IO4 LOW for 500ms)...");
  
  // Pull IO4 to ground to activate motor
  digitalWrite(MOTOR_CONTROL_PIN, LOW);
  delay(500);
  digitalWrite(MOTOR_CONTROL_PIN, HIGH);
  
  Serial.println("  ✓ Motor activation complete");
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