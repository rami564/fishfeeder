# 🐠 ESP32-C3 Fish Feeder

A WiFi-controlled automatic fish feeder using ESP32-C3 microcontroller with web interface.

## Features

- 🌐 **WiFi Control** - Control via web browser from any device on your network
- 🎯 **Simple Web Interface** - Clean, mobile-friendly control panel
- 🔄 **Servo Motor Control** - Precise food dispensing mechanism
- 💡 **LED Indicators** - Visual feedback for status and WiFi connectivity
- 📊 **Real-time Status** - Monitor WiFi signal, uptime, and memory usage

## Hardware Requirements

### Components
- **ESP32-C3 Super Mini** (or compatible ESP32-C3 board)
- **Servo Motor** (SG90 or similar 0-180° servo)
- **2x LEDs** (optional, for status indicators)
- **2x 220Ω Resistors** (for LEDs)
- **Power Supply** (5V, sufficient current for servo)
- **Breadboard and jumper wires**

### Wiring Diagram

```
ESP32-C3 Pin Connections:
┌─────────────────────────────────────┐
│ ESP32-C3 Super Mini                  │
│                                     │
│  GPIO2  ───────────> Servo Signal  │
│  5V     ───────────> Servo VCC     │
│  GND    ───────────> Servo GND     │
│                                     │
│  GPIO8  ───[220Ω]──> LED1 (Status) │
│  GPIO10 ───[220Ω]──> LED2 (WiFi)   │
│  GND    ───────────> LEDs (cathode)│
└─────────────────────────────────────┘
```

**Important Notes:**
- Connect servo to **external 5V power** if it draws > 500mA
- Ensure common ground between ESP32 and servo power supply
- GPIO8 may be the built-in LED on some boards (test first)

## Software Setup

### 1. Install PlatformIO

If you haven't already:
```bash
# Via VS Code Extension
# Install "PlatformIO IDE" extension from VS Code marketplace
```

### 2. Configure WiFi Credentials

Edit `src/main.cpp` and update these lines (around line 9-10):
```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Your WiFi network name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

### 3. Adjust Pin Configuration (if needed)

If your wiring is different, update these constants in `src/main.cpp` (around line 13-15):
```cpp
const int SERVO_PIN = 2;        // Change if using different GPIO
const int LED_STATUS_PIN = 8;   // Change if using different GPIO
const int LED_WIFI_PIN = 10;    // Change if using different GPIO
```

### 4. Build and Upload

```bash
# Option 1: Using PlatformIO CLI
pio run --target upload

# Option 2: Using VS Code
# Click the "Upload" button (→) in the PlatformIO toolbar
```

### 5. Monitor Serial Output

```bash
# View serial monitor to get IP address
pio device monitor

# Or use VS Code PlatformIO serial monitor
```

## Usage

### First Time Setup

1. **Upload the code** to your ESP32-C3
2. **Open Serial Monitor** (115200 baud)
3. **Wait for WiFi connection** - You'll see the IP address printed
4. **Open a web browser** and navigate to `http://YOUR_ESP32_IP`
5. **Test the feeder** by clicking "Feed Fish Now"

### Web Interface

The web interface provides:
- **Feed Button** - Dispenses food immediately
- **WiFi Signal** - Current WiFi signal strength (dBm)
- **Uptime** - How long the device has been running
- **Free Memory** - Available heap memory

### Accessing from Different Devices

- **Same Network**: Any device on your WiFi can access `http://YOUR_ESP32_IP`
- **Bookmarking**: Save the IP address as a bookmark on your phone for easy access
- **Static IP** (optional): Configure your router to assign a static IP to the ESP32

## Customization

### Adjust Servo Angles

Modify these values in `src/main.cpp` (around line 18-20) based on your feeder mechanism:
```cpp
const int SERVO_REST_POSITION = 0;      // Starting position
const int SERVO_DISPENSE_POSITION = 90; // Dispensing position
const int DISPENSE_DURATION_MS = 2000;  // How long to stay in dispense position
```

### Change Dispense Duration

Increase or decrease the time the servo holds the dispense position:
```cpp
const int DISPENSE_DURATION_MS = 3000;  // 3 seconds instead of 2
```

## Troubleshooting

### WiFi Won't Connect
- Verify SSID and password are correct
- Ensure ESP32 is within WiFi range
- Check if your network is 2.4GHz (ESP32 doesn't support 5GHz)
- Look for "✗ WiFi Connection Failed!" in serial monitor

### Servo Not Moving
- Check wiring connections (GPIO2, 5V, GND)
- Verify servo has sufficient power supply
- Try adjusting `SERVO_DISPENSE_POSITION` angle
- Test servo separately with sweep example

### Can't Access Web Interface
- Confirm ESP32 is connected to WiFi (check serial monitor)
- Verify you're on the same WiFi network as ESP32
- Try accessing `http://IP_ADDRESS` (not `https://`)
- Check firewall settings on your device

### Compilation Errors
- Ensure PlatformIO libraries are installed (check `platformio.ini`)
- Try: `pio lib install` to reinstall dependencies
- Clean build folder: `pio run --target clean`

## Future Enhancements

Ideas for extending this project:
- [ ] Schedule automatic feeding times
- [ ] Add feeding history/log
- [ ] Implement food level sensor
- [ ] MQTT integration for smart home
- [ ] Mobile app (instead of web interface)
- [ ] Multiple dispense amounts
- [ ] Water temperature sensor
- [ ] Camera for monitoring

## Technical Details

### Libraries Used
- **ESP32Servo** (v1.2.0) - Servo motor control for ESP32
- **ArduinoJson** (v7.0.4) - JSON parsing for API responses
- **WiFi** (built-in) - WiFi connectivity
- **WebServer** (built-in) - HTTP server

### Memory Usage
- Typical flash usage: ~250KB
- Typical RAM usage: ~50KB free (of ~400KB total on ESP32-C3)

### Power Consumption
- Active (WiFi connected): ~80-120mA
- Servo moving: +500-1000mA (depending on servo and load)
- Deep sleep (not implemented): ~10µA

## License

This project is open source. Feel free to modify and share!

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review serial monitor output for error messages
3. Verify hardware connections match the wiring diagram

---

**Happy Fish Feeding! 🐟**
