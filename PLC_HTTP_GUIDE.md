# PLC HTTP Communication Guide

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

## Status Endpoint (Optional)

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

## Quick Reference

### Minimum Required HTTP Request
```
POST /feed HTTP/1.1
Host: 10.10.1.139

```
(Note: Double newline at end)

### Minimum Success Check
```
IF response.contains("200") THEN success = TRUE
```

### IP Address (Update if Changed)
Current IP: `10.10.1.139`

To find current IP:
```bash
ping fishfeeder.local
```

Or check ESP32 serial monitor on boot.
