# 🚀 OTA & mDNS Quick Reference

## What You Just Got

### ✅ mDNS (Network Name)
- Access your fish feeder via **`http://fishfeeder.local`**
- No need to remember IP addresses!
- Works on most devices (Mac, iPhone, iPad, most Android)

### ✅ OTA (Over-The-Air Updates)
- Update firmware **wirelessly** - no USB cable needed
- Password protected for security
- Visual feedback via LED during updates

---

## First-Time Setup (USB Required)

1. **Update the code** with your credentials:
   ```cpp
   const char* WIFI_SSID = "YOUR_WIFI";
   const char* WIFI_PASSWORD = "YOUR_PASSWORD";
   const char* OTA_PASSWORD = "YOUR_SECRET_PASSWORD"; // CHANGE THIS!
   ```

2. **Upload via USB** (first time only):
   ```bash
   pio run --target upload
   ```

3. **Check serial monitor** for confirmation:
   ```
   ✓ WiFi Connected!
     IP Address: 192.168.1.XXX
   ✓ mDNS responder started
     Access via: http://fishfeeder.local
   ✓ OTA update service started
     Password protected for security
   ✓ Web server started
     Access at: http://192.168.1.XXX or http://fishfeeder.local
   ```

---

## Daily Usage

### Access Web Interface
Choose whichever works for you:
- **Option 1**: `http://fishfeeder.local` ⭐ (easiest)
- **Option 2**: `http://192.168.1.XXX` (if mDNS doesn't work)

### Update Firmware (No Cable!)
```bash
# Make your code changes, then:
pio run --target upload --upload-port fishfeeder.local

# Or use IP if mDNS doesn't work:
pio run --target upload --upload-port 192.168.1.XXX
```

**During OTA Update:**
- Status LED will blink
- Takes about 10-30 seconds
- Device automatically reboots
- Don't power off during update!

---

## For Unitronics V350 PLC Integration

### Use IP Address (Not mDNS)
The V350 PLC likely doesn't support mDNS, so:
- Use the **IP address** in your PLC program
- Set **DHCP reservation** in your router for the ESP32
- This ensures the IP never changes

### Example V350 Configuration
```
Target IP: 192.168.1.100  (your ESP32's IP)
Port: 80
Endpoint: /feed
Method: POST
```

---

## Troubleshooting

### mDNS Not Working?
1. **Windows**: Install Bonjour (comes with iTunes)
2. **Android**: Some devices don't support it - use IP address
3. **Test**: Run `ping fishfeeder.local` in terminal
4. **Fallback**: Always use IP address if mDNS fails

### OTA Upload Fails?
1. Check ESP32 is powered on and connected to WiFi
2. Verify password matches in code
3. Try using IP address instead of `.local`
4. Check firewall isn't blocking port 3232
5. Only one device can upload at a time

### Can't Connect After OTA Update?
1. Wait 30 seconds for device to fully reboot
2. Check serial monitor for error messages
3. If stuck, upload via USB to recover

---

## Security Notes

### Default Password
⚠️ **Change the default OTA password!**
```cpp
const char* OTA_PASSWORD = "fish2024";  // CHANGE THIS to something unique!
```

### Network Security
- ESP32 and V350 should be on **isolated VLAN** (optional but recommended)
- Only devices on same network can access the feeder
- OTA requires password - prevents unauthorized updates

### Best Practices
- Use strong WiFi password
- Use unique OTA password
- Consider MAC address filtering on router
- Keep firmware updated

---

## Advanced: PlatformIO.ini OTA Configuration

To make OTA the default upload method, add to `platformio.ini`:

```ini
[env:esp32-c3-super-mini]
upload_protocol = espota
upload_port = fishfeeder.local
upload_flags = 
    --auth=fish2024
```

Then you can just run:
```bash
pio run --target upload  # Automatically uses OTA!
```

---

**Happy Wireless Updating! 📡**
