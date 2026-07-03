# UDP Quick Reference

## Configuration

Set protocol mode in `src/main.cpp`:
```cpp
#define PROTOCOL_MODE "BOTH"  // "TCP", "UDP", or "BOTH"
const int UDP_PORT = 8888;
```

## Commands

| Command | Action | Response |
|---------|--------|----------|
| `FEED` | Dispense food | `{"success":true,"message":"Food dispensed"}` |
| `STATUS` | Get device info | `{"rssi":-45,"uptime":123,"freeHeap":98765,"ip":"192.168.1.100"}` |

Commands are case-insensitive.

## Testing

### Command Line (netcat)
```bash
echo "FEED" | nc -u fishfeeder.local 8888
echo "STATUS" | nc -u fishfeeder.local 8888
```

### Python Script
```bash
# Interactive mode
./test_udp.py -i 192.168.1.100

# Single command
./test_udp.py FEED -i 192.168.1.100

# Automated test
./test_udp.py --test -i 192.168.1.100
```

### From PLC/SCADA
1. Configure UDP client to target ESP32 IP:PORT
2. Send ASCII string: "FEED" or "STATUS"
3. Parse JSON response
4. Typical response time: <10ms

## Protocol Comparison

| Feature | HTTP (TCP) | UDP |
|---------|-----------|-----|
| Latency | ~50-100ms | <10ms |
| Reliability | High (retransmission) | None (fire-and-forget) |
| Overhead | Higher | Minimal |
| Browser Support | ✅ Yes | ❌ No |
| PLC/SCADA | ⚠️ Possible | ✅ Ideal |
| Connection | Stateful | Stateless |

## Serial Monitor Output

When UDP enabled:
```
✓ UDP server started
  Listening on port: 8888
  IP Address: 192.168.1.100

  UDP Commands:
    'FEED' - Dispense food
    'STATUS' - Get device status
```

When command received:
```
>>> UDP Packet from 192.168.1.50:54321 - Command: FEED
  → Activating motor (IO4 LOW for 500ms)...
  ✓ Motor activation complete
  ✓ FEED command executed
<<< UDP Request Completed
```

## Security Notes

⚠️ UDP packets are not encrypted
- Use VLANs for isolation
- Consider IP whitelist filtering
- Add authentication tokens for production
- Use DTLS for encrypted UDP if needed

## Troubleshooting

**No response:**
- Check firewall (allow UDP port 8888)
- Verify PROTOCOL_MODE is "UDP" or "BOTH"
- Ping ESP32 to confirm connectivity
- Use IP address instead of mDNS

**Commands ignored:**
- Ensure exact spelling: "FEED" or "STATUS"
- Check serial monitor for received packets
- Verify UDP_PORT matches (default: 8888)

**PLC integration issues:**
- Set timeout to 2-3 seconds
- Handle no-response gracefully
- Send STATUS before FEED to verify connectivity
- Log all UDP transactions for debugging
