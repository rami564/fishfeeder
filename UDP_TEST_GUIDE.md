# UDP Server Test Guide

## Overview
Your Fish Feeder now supports UDP commands in addition to HTTP. You can choose which protocol(s) to use.

## Configuration

In `src/main.cpp`, set the `PROTOCOL_MODE`:

```cpp
#define PROTOCOL_MODE "BOTH"  // Options: "TCP", "UDP", or "BOTH"
```

- **"TCP"** - HTTP web server only (original functionality)
- **"UDP"** - UDP server only
- **"BOTH"** - Both HTTP and UDP servers active simultaneously

UDP port: **8888** (configurable via `UDP_PORT`)

## UDP Commands

The UDP server accepts the following commands:

### 1. FEED Command
Triggers the food dispensing mechanism.

**Command:** `FEED`

**Response:**
```json
{"success":true,"message":"Food dispensed"}
```

### 2. STATUS Command
Returns device status information.

**Command:** `STATUS`

**Response:**
```json
{
  "rssi":-45,
  "uptime":1234,
  "freeHeap":98765,
  "ip":"192.168.1.100"
}
```

## Testing Methods

### Method 1: Using netcat (nc)

#### Send FEED command:
```bash
echo "FEED" | nc -u -w1 fishfeeder.local 8888
# or with IP address
echo "FEED" | nc -u -w1 192.168.1.100 8888
```

#### Get STATUS:
```bash
echo "STATUS" | nc -u -w1 fishfeeder.local 8888
```

### Method 2: Using Python

Create a test script `udp_test.py`:

```python
#!/usr/bin/env python3
import socket
import sys

UDP_IP = "192.168.1.100"  # Change to your ESP32 IP
UDP_PORT = 8888

def send_command(command):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2)
    
    try:
        # Send command
        sock.sendto(command.encode(), (UDP_IP, UDP_PORT))
        print(f"Sent: {command}")
        
        # Receive response
        data, addr = sock.recvfrom(1024)
        print(f"Response: {data.decode()}")
        
    except socket.timeout:
        print("No response (timeout)")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        command = sys.argv[1]
    else:
        command = "STATUS"
    
    send_command(command)
```

**Usage:**
```bash
python3 udp_test.py FEED
python3 udp_test.py STATUS
```

### Method 3: Using socat

```bash
# Interactive mode
socat - UDP:fishfeeder.local:8888

# One-shot command
echo "FEED" | socat - UDP:fishfeeder.local:8888
```

### Method 4: Using Node.js

```javascript
const dgram = require('dgram');

const client = dgram.createSocket('udp4');
const message = Buffer.from('FEED');

client.send(message, 8888, '192.168.1.100', (err) => {
    if (err) {
        console.error(err);
        client.close();
    } else {
        console.log('Command sent');
    }
});

client.on('message', (msg, rinfo) => {
    console.log(`Response: ${msg}`);
    client.close();
});

setTimeout(() => {
    client.close();
}, 2000);
```

## Integration Example: PLC Control

For PLC systems that support UDP communication:

1. Configure PLC UDP client to target ESP32 IP and port 8888
2. Send ASCII string "FEED" when feeding is triggered
3. Parse JSON response for confirmation
4. Send "STATUS" periodically for health monitoring

## Troubleshooting

### No response from UDP server
1. Check firewall rules on your computer
2. Verify ESP32 IP address: `ping fishfeeder.local`
3. Check Serial Monitor for UDP server startup message
4. Ensure `PROTOCOL_MODE` is set to "UDP" or "BOTH"

### Commands not working
1. Commands are case-insensitive but must be exact: "FEED" or "STATUS"
2. Check Serial Monitor for incoming packet logs
3. Verify UDP port (default: 8888)

### Can't find device
1. Use IP address instead of mDNS name
2. Check WiFi connection status on ESP32
3. Ensure ESP32 and client are on same network/VLAN

## Performance Notes

- UDP has minimal overhead compared to HTTP
- Suitable for real-time control applications
- No connection establishment delay
- Ideal for PLC/SCADA integration
- Can handle multiple clients simultaneously
- Typical response time: <10ms on local network

## Security Considerations

⚠️ **Warning:** UDP packets are not encrypted by default.

For production use:
1. Implement authentication tokens in commands
2. Use VLANs to isolate control network
3. Add IP whitelist filtering in code
4. Consider implementing basic checksum/validation
5. Use TLS/DTLS for encrypted UDP if needed
