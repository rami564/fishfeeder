# Hardware Testing Guide

Before deploying your fish feeder, test each component individually to ensure everything works correctly.

## Test 1: Basic Board Test

**Purpose**: Verify ESP32-C3 is functioning and can be programmed.

**Test Code**: Already included in the main program. Upload and check serial monitor at 115200 baud.

**Expected Output**:
```
=================================
Fish Feeder Starting...
=================================
✓ Servo initialized
Connecting to WiFi........
✓ WiFi Connected!
```

**If fails**: Check USB connection, drivers, and board selection in platformio.ini

---

## Test 2: LED Test

**Purpose**: Verify LED connections and GPIO pins work.

**Test**: The LEDs should:
- **WiFi LED (GPIO10)**: Blink while connecting, solid when connected
- **Status LED (GPIO8)**: Blink 3 times on startup, flash during feeding

**Quick Test Code** (if needed):
```cpp
void loop() {
  digitalWrite(LED_STATUS_PIN, HIGH);
  delay(500);
  digitalWrite(LED_STATUS_PIN, LOW);
  delay(500);
}
```

**Troubleshooting**:
- LED doesn't light: Check polarity (long leg = anode/+)
- Too dim: Check resistor value (220Ω recommended)
- GPIO8 may be built-in LED on some boards

---

## Test 3: Servo Test

**Purpose**: Verify servo is properly connected and moves correctly.

**Test Steps**:
1. Upload the main code
2. Open serial monitor
3. Access web interface
4. Click "Feed Fish Now"
5. Watch servo move from 0° to 90° and back

**Expected Behavior**:
- Servo moves smoothly to 90°
- Holds position for 2 seconds
- Returns smoothly to 0°
- No jittering or stalling

**Troubleshooting**:
- **Servo jitters**: Insufficient power, use external 5V supply
- **Servo doesn't move**: Check signal wire (GPIO2), verify 5V and GND
- **Servo moves wrong direction**: Adjust angles in code
- **Servo overheats**: Reduce duty cycle or check for mechanical binding

**Servo Current Requirements**:
- Idle: 5-10mA
- Moving (no load): 100-250mA
- Moving (with load): 500-1000mA
- **Recommendation**: Use external power supply if > 500mA

---

## Test 4: WiFi Connection Test

**Purpose**: Verify WiFi connectivity and web server.

**Test Steps**:
1. Update WiFi credentials in code
2. Upload and open serial monitor
3. Wait for connection (up to 15 seconds)
4. Note the IP address printed
5. Open browser to `http://IP_ADDRESS`

**Expected Output** (Serial Monitor):
```
Connecting to WiFi.......
✓ WiFi Connected!
  IP Address: 192.168.1.100
  Signal Strength: -45 dBm
✓ Web server started
  Access at: http://192.168.1.100
```

**Troubleshooting**:
- **Connection timeout**: Check SSID/password spelling (case-sensitive!)
- **Wrong IP**: Verify you're on 2.4GHz network (not 5GHz)
- **Can't access webpage**: Ensure device is on same network
- **Signal weak (< -70 dBm)**: Move router closer or use WiFi extender

---

## Test 5: Web Interface Test

**Purpose**: Test all web functionality.

**Test Checklist**:
- [ ] Page loads with correct styling
- [ ] WiFi signal displays in dBm
- [ ] Uptime displays and updates
- [ ] Free memory displays
- [ ] "Feed Fish Now" button is clickable
- [ ] Button disables during feeding
- [ ] Success message appears after feeding
- [ ] Status updates every 5 seconds
- [ ] Works on mobile device
- [ ] Works on desktop browser

**API Endpoints to Test**:
- `GET /` - Main page (should return HTML)
- `GET /status` - Status JSON (should return {"rssi": -45, "uptime": 123, "freeHeap": 50000})
- `POST /feed` - Feed command (should return {"success": true, "message": "..."})

**Manual API Test** (using curl):
```bash
# Get status
curl http://YOUR_ESP32_IP/status

# Trigger feeding
curl -X POST http://YOUR_ESP32_IP/feed
```

---

## Test 6: Full Integration Test

**Purpose**: Test complete feeding cycle.

**Test Procedure**:
1. Ensure servo is attached to feeder mechanism
2. Load feeder with food (small amount for testing)
3. Open web interface on phone
4. Click "Feed Fish Now"
5. Observe entire feeding sequence

**Success Criteria**:
- [x] Web button responds immediately
- [x] Status LED lights up
- [x] Servo moves to dispense position
- [x] Food is dispensed
- [x] Servo returns to rest position
- [x] Status LED turns off
- [x] Web interface shows success message
- [x] No errors in serial monitor

**Measure**:
- Total cycle time: ~2 seconds (adjustable)
- Food amount dispensed: (adjust servo angle/duration as needed)

---

## Test 7: Reliability Test

**Purpose**: Ensure system is stable for long-term operation.

**Test Duration**: 30 minutes minimum

**Test Steps**:
1. Upload code and start device
2. Trigger feeding every 5 minutes (6 times total)
3. Monitor serial output for errors
4. Check memory usage over time
5. Verify WiFi stays connected

**What to Monitor**:
```
Initial Free Heap: ~50,000 bytes
After 30 min: Should be similar (±5%)
WiFi Disconnects: 0
Servo Failures: 0
```

**Red Flags**:
- Memory continuously decreasing (memory leak)
- WiFi disconnects frequently (signal issue)
- Servo stops responding (power or overheating)
- Web interface becomes slow/unresponsive

---

## Recommended Servo Angles (by mechanism type)

### Rotating Drum
```cpp
SERVO_REST_POSITION = 0;
SERVO_DISPENSE_POSITION = 90;  // Quarter turn
DISPENSE_DURATION_MS = 2000;
```

### Flap/Gate
```cpp
SERVO_REST_POSITION = 0;       // Closed
SERVO_DISPENSE_POSITION = 45;  // Open 45°
DISPENSE_DURATION_MS = 1500;
```

### Auger/Screw
```cpp
SERVO_REST_POSITION = 0;
SERVO_DISPENSE_POSITION = 180; // Full rotation
DISPENSE_DURATION_MS = 3000;   // Longer for complete turn
```

### Piston/Pusher
```cpp
SERVO_REST_POSITION = 90;      // Retracted
SERVO_DISPENSE_POSITION = 0;   // Extended
DISPENSE_DURATION_MS = 1000;
```

---

## Power Supply Checklist

**Minimum Requirements**:
- Voltage: 5V (±0.25V)
- Current: 1.5A (for ESP32 + servo under load)
- Connection: Stable, avoid long thin wires

**Recommended**:
- USB power adapter: 5V 2A
- USB power bank (for portability)
- Wall adapter with quality USB cable

**Power Issues Symptoms**:
- Brownouts (ESP32 resets during servo movement)
- Servo jittering
- WiFi disconnects
- Serial monitor shows "Brownout detector triggered"

**Solution**: Use thicker wires (20-22 AWG) and dedicated servo power supply.

---

## Final Deployment Checklist

Before mounting and deploying your fish feeder:

- [ ] All tests above passed
- [ ] Servo moves correctly with actual food
- [ ] WiFi signal is strong at deployment location (> -60 dBm)
- [ ] Power supply is reliable
- [ ] Feeder mechanism is smooth (no jamming)
- [ ] ESP32 is protected from water/humidity
- [ ] Wiring is secure and won't be disturbed
- [ ] You know the IP address or have it bookmarked
- [ ] You've tested feeding multiple times successfully
- [ ] Serial monitor shows no errors or warnings

**Ready to deploy! 🐠**
