# PLC Communication Guide

This guide covers both **HTTP (TCP)** and **UDP** communication methods for controlling the Fish Feeder from PLC systems.

---

## Protocol Comparison

| Feature | HTTP (TCP) | UDP |
|---------|-----------|-----|
| **Complexity** | Higher | Lower |
| **Connection** | Stateful (3-way handshake) | Stateless |
| **Overhead** | ~200 bytes | ~10 bytes |
| **Response Time** | 50-100ms | <10ms |
| **Reliability** | Guaranteed delivery | No guarantees |
| **Parsing** | Complex (headers + JSON) | Simple (direct JSON) |
| **Best For** | Reliable commands | Fast, real-time control |

**Recommendation:** Use **UDP** for PLC systems - it's simpler, faster, and more suitable for industrial control.

---

# UDP Communication (Recommended for PLC)

## UDP Configuration

### Device Configuration
Set protocol mode in ESP32 firmware:
```cpp
#define PROTOCOL_MODE "BOTH"  // or "UDP" for UDP only
const int UDP_PORT = 8888;
```

### PLC Socket Configuration
```
Socket Type: UDP Client
Target IP: 10.10.1.139
Target Port: 8888
Protocol: UDP/IP
```

## UDP Commands

### Command Format
Send plain ASCII text commands (case-insensitive):

| Command | Purpose | Response |
|---------|---------|----------|
| `FEED` | Dispense food | `{"success":true,"message":"Food dispensed"}` |
| `STATUS` | Get device status | `{"rssi":-45,"uptime":123,"freeHeap":98765,"ip":"10.10.1.139"}` |

### String to Send
```
"FEED"
```
That's it! Just 4 bytes + newline (if needed).

### Expected Response (FEED)
```json
{"success":true,"message":"Food dispensed"}
```

### Expected Response (STATUS)
```json
{"rssi":-45,"uptime":3600,"freeHeap":98765,"ip":"10.10.1.139"}
```

## V350 Unitronics PLC - UDP Implementation

### Simple UDP Ladder Logic
```
1. Create UDP socket (non-blocking)
2. Set target: IP=10.10.1.139, Port=8888
3. Send string: "FEED"
4. Wait for response (timeout: 2 seconds)
5. Store response in buffer
6. Parse JSON response
7. Set feedback bit based on result
8. Close socket
```

### Buffer Requirements
- Send Buffer: 10 bytes (for "FEED" command)
- Receive Buffer: 128 bytes (for JSON response)

### Minimal Parsing (Easiest)
```
IF response contains '"success":true' THEN
    Feed_Success = TRUE
ELSE
    Feed_Success = FALSE
END IF
```

### Response Timeout
- **Recommended:** 2 seconds
- **Maximum:** 3 seconds
- **Retry:** Up to 3 attempts with 1 second delay

## UDP Testing Before PLC Implementation

### Using netcat (Linux/Mac)
```bash
# Send FEED command
echo "FEED" | nc -u 10.10.1.139 8888

# Send STATUS command
echo "STATUS" | nc -u 10.10.1.139 8888
```

### Using Python Script
```bash
# Test connectivity
./test_udp.py STATUS -i 10.10.1.139

# Test feed command
./test_udp.py FEED -i 10.10.1.139
```

### Using socat (Alternative)
```bash
echo "FEED" | socat - UDP:10.10.1.139:8888
```

## UDP Error Handling

### No Response (Timeout)
```
Possible causes:
- ESP32 offline
- Wrong IP address
- Wrong UDP port (should be 8888)
- Firewall blocking UDP
- Protocol mode not set to "UDP" or "BOTH"

Action:
- Send STATUS command first to verify connectivity
- Check network connectivity (ping ESP32)
- Verify UDP_PORT in firmware
```

### Invalid Response
```
If response doesn't contain "success":
- Device received command but failed to execute
- Motor malfunction
- Internal error

Action:
- Log response for debugging
- Retry after 5 seconds
- Alert operator if multiple failures
```

## Complete UDP Example (Structured Text)

```
PROGRAM FishFeeder_UDP
VAR
    udpSocket : SOCKET_UDP;
    targetIP : STRING := '10.10.1.139';
    targetPort : INT := 8888;
    sendBuffer : STRING := 'FEED';
    recvBuffer : STRING[128];
    feedSuccess : BOOL := FALSE;
    timeout : TIME := T#2s;
END_VAR

(* Open UDP socket *)
udpSocket.Open();

(* Send command *)
udpSocket.SendTo(targetIP, targetPort, sendBuffer);

(* Wait for response with timeout *)
IF udpSocket.ReceiveFrom(recvBuffer, timeout) THEN
    (* Parse response *)
    IF FIND(recvBuffer, '"success":true') > 0 THEN
        feedSuccess := TRUE;
    ELSE
        feedSuccess := FALSE;
    END_IF;
ELSE
    (* Timeout - no response *)
    feedSuccess := FALSE;
END_IF;

(* Close socket *)
udpSocket.Close();
END_PROGRAM
```

---

# HTTP Communication (Alternative Method)

## HTTP Request to Send

### Raw HTTP Message Format
```http
POST /feed HTTP/1.1
Host: 10.10.1.139
Content-Type: application/json
Content-Length: 0
Connection: close

```

**Important Notes:**
- Two CRLF (carriage return + line feed) at the end: `\r\n\r\n`
- Empty body (Content-Length: 0)
- Use actual IP address, not `fishfeeder.local`
- Port: 80 (default HTTP)

### Alternative with Keep-Alive
```http
POST /feed HTTP/1.1
Host: 10.10.1.139
Content-Type: application/json
Content-Length: 0

```

---

## Expected Response

### Success Response
```http
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 60
Connection: close

{"success": true, "message": "Food dispensed successfully!"}
```

### Response Breakdown
- **Status Line**: `HTTP/1.1 200 OK` (first line)
- **Headers**: Content-Type, Content-Length, etc.
- **Blank Line**: Separates headers from body
- **JSON Body**: `{"success": true, "message": "Food dispensed successfully!"}`

---

## Parsing the Response (PLC Logic)

### Step 1: Extract Status Code
```
Find: "HTTP/1.1 "
Extract next 3 characters: "200"
If status == "200" → Success
If status == "404" → Wrong endpoint
If status == "500" → Server error
```

### Step 2: Find JSON Body
```
1. Search for blank line ("\r\n\r\n" or "\n\n")
2. Everything after blank line = JSON body
3. Body should be: {"success": true, "message": "Food dispensed successfully!"}
```

### Step 3: Parse JSON (Simple Method)
```
Option A - Check for "success": true
  If response contains '"success": true' → Feed successful
  If response contains '"success": false' → Feed failed

Option B - Simple string parsing (no JSON parser needed)
  Find: '"success":'
  Extract next word after colon
  If word contains "true" → Success
  If word contains "false" → Failed
```

---

## V350 Unitronics PLC Implementation

### Configuration
```
Socket Type: TCP Client
Target IP: 10.10.1.139
Port: 80
Protocol: None (Raw TCP)
```

### String to Send (Complete HTTP Request)
```
"POST /feed HTTP/1.1\r\nHost: 10.10.1.139\r\nContent-Type: application/json\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
```

### Ladder Logic Pseudocode
```
1. Open TCP Socket to 10.10.1.139:80
2. Wait for connection established
3. Send HTTP request string (above)
4. Wait for response (timeout: 5 seconds)
5. Store response in buffer
6. Close socket
7. Parse response:
   - Search for "200 OK" in first line → Success
   - Or search for '"success": true' in body → Success
8. Set feedback bit based on result
```

### Response Buffer Size
- Minimum: 128 bytes
- Recommended: 256 bytes (to handle headers + JSON)

---

## Testing with curl (Before PLC Implementation)

### Test Feed Command
```bash
curl -X POST http://10.10.1.139/feed
```

**Expected Output:**
```json
{"success": true, "message": "Food dispensed successfully!"}
```

### Test with Verbose Output
```bash
curl -v -X POST http://10.10.1.139/feed
```

**Shows:**
```
* Connected to 10.10.1.139 (10.10.1.139) port 80
> POST /feed HTTP/1.1
> Host: 10.10.1.139
> User-Agent: curl/7.81.0
> Accept: */*
> 
< HTTP/1.1 200 OK
< Content-Type: application/json
< Content-Length: 60
< Connection: close
< 
{"success": true, "message": "Food dispensed successfully!"}
```

---

## Simple Acknowledgement Logic

### Minimal Parsing (Easiest for PLC)
```
IF response contains "200 OK" THEN
    Feed_Success = TRUE
ELSE
    Feed_Success = FALSE
END IF
```

### More Robust Parsing
```
1. Extract first line of response
2. Split by space: ["HTTP/1.1", "200", "OK"]
3. If second element == "200":
      Find "success"
      If next non-space chars are ": true":
          Feed_Success = TRUE
      Else:
          Feed_Success = FALSE
   Else:
      Feed_Success = FALSE
```

---

## Error Handling

### Possible Errors

| Status Code | Meaning | Action |
|------------|---------|--------|
| 200 | Success | Feed completed |
| 404 | Not Found | Check endpoint (/feed) |
| 500 | Server Error | ESP32 crashed, reboot needed |
| Timeout | No response | Check network/power |
| Connection refused | ESP32 offline | Check WiFi connection |

### Timeout Recommendations
- Connection timeout: 3 seconds
- Response timeout: 5 seconds
- Retry attempts: 3 times with 2 second delay

---

## HTTP Status Endpoint (Optional)

### Get Device Status
```bash
curl http://10.10.1.139/status
```

**Response:**
```json
{"rssi":-45,"uptime":1234,"freeHeap":234567}
```

### Use Cases
- Health check before feeding
- Monitor WiFi signal strength (rssi)
- Track device uptime
- Verify device is responsive

---

# Protocol Selection Guide

## When to Use UDP (Recommended)

✅ **Best for:**
- PLC/SCADA systems
- Real-time control applications
- Embedded controllers with limited resources
- Fast response requirements (<10ms)
- Simple string parsing capabilities
- Fire-and-forget commands

**Advantages:**
- Simpler implementation (4 bytes vs 200 bytes)
- Lower latency
- Less PLC memory usage
- No connection management
- Easier to debug

**Example PLC brands that work well with UDP:**
- Unitronics (V350, V430, V560, etc.)
- Allen-Bradley MicroLogix/CompactLogix
- Siemens S7-1200/1500
- Modicon M221/M241
- IDEC MicroSmart

## When to Use HTTP

✅ **Best for:**
- Web-based HMI systems
- SCADA with built-in HTTP clients
- Cloud integration
- Systems requiring guaranteed delivery
- Audit trail requirements
- When UDP is blocked by firewall

**Advantages:**
- Guaranteed delivery
- Status codes for error handling
- Works through most firewalls
- Better for unreliable networks
- Standard web protocols

## Side-by-Side Comparison

### UDP Example
```
Send: "FEED" (4 bytes)
Receive: {"success":true,"message":"Food dispensed"} (~50 bytes)
Time: ~5-10ms
```

### HTTP Example
```
Send: "POST /feed HTTP/1.1\r\nHost: 10.10.1.139\r\n..." (~200 bytes)
Receive: HTTP/1.1 200 OK\r\n... {"success":true,...} (~150 bytes)
Time: ~50-100ms
```

**UDP is 10x faster and 50x simpler!**

---

## Mixed Protocol Strategy

You can enable both protocols simultaneously:
```cpp
#define PROTOCOL_MODE "BOTH"
```

**Use case:**
- UDP for time-critical PLC commands (feed operation)
- HTTP for web HMI monitoring (status dashboard)
- HTTP for manual override from tablet/phone
- UDP for automated scheduled feeds

---

## Quick Reference Card

### UDP Quick Reference
```
Target: 10.10.1.139:8888
Command: "FEED" or "STATUS"
Timeout: 2 seconds
Buffer: 128 bytes
Success: Contains '"success":true'
```

### HTTP Quick Reference
```
Target: 10.10.1.139:80
Request: POST /feed HTTP/1.1\r\nHost: 10.10.1.139\r\n\r\n
Timeout: 5 seconds
Buffer: 256 bytes
Success: Contains "200 OK"
```

---

## Troubleshooting Both Protocols

### Device Not Responding

**Check list:**
1. ✓ Ping ESP32: `ping 10.10.1.139`
2. ✓ Check protocol mode in firmware (UDP/TCP/BOTH)
3. ✓ Verify port numbers (UDP=8888, HTTP=80)
4. ✓ Check firewall rules
5. ✓ Verify ESP32 WiFi connection (check Serial Monitor)
6. ✓ Test with command line tools first

### UDP-Specific Issues
- **No response:** Check UDP_PORT is 8888
- **Wrong response:** Verify command is "FEED" or "STATUS" (case-insensitive)
- **Firewall:** UDP may be blocked, try HTTP

### HTTP-Specific Issues
- **404 Error:** Check endpoint is "/feed" not "/FEED"
- **Timeout:** HTTP takes longer, increase timeout to 5s
- **Connection refused:** Port 80 might be in use

---

## IP Address (Update if Changed)
Current IP: `10.10.1.139`

To find current IP:
```bash
ping fishfeeder.local
# or check ESP32 serial monitor on boot
```

---

## Additional Resources

- **UDP Testing Guide:** See `UDP_TEST_GUIDE.md`
- **UDP Quick Reference:** See `UDP_QUICK_REF.md`
- **Main README:** See `README.md`
- **OTA Updates:** See `OTA_GUIDE.md`

---

## Support Matrix

| PLC/SCADA System | UDP Support | HTTP Support | Recommended |
|-----------------|-------------|--------------|-------------|
| Unitronics V-Series | ✅ Excellent | ✅ Good | UDP |
| Siemens S7 | ✅ Excellent | ✅ Good | UDP |
| Allen-Bradley | ✅ Good | ✅ Excellent | Either |
| Modicon M2xx | ✅ Good | ✅ Good | UDP |
| Ignition SCADA | ✅ Excellent | ✅ Excellent | Either |
| FactoryTalk | ⚠️ Limited | ✅ Excellent | HTTP |
| WinCC | ✅ Good | ✅ Good | UDP |

---

**Last Updated:** 2026-07-03
