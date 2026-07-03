#!/usr/bin/env python3
"""
Fish Feeder UDP Test Script
Simple utility to test UDP commands with your ESP32 Fish Feeder
"""

import socket
import sys
import json
import time

# Configuration
DEFAULT_IP = "192.168.1.100"  # Change to your ESP32 IP address
DEFAULT_PORT = 8888
TIMEOUT = 2  # seconds

def send_udp_command(ip, port, command):
    """
    Send a UDP command to the fish feeder and wait for response
    
    Args:
        ip: IP address of the ESP32
        port: UDP port number
        command: Command string (e.g., "FEED", "STATUS")
    
    Returns:
        Response string from device, or None if timeout
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(TIMEOUT)
    
    try:
        # Send command
        print(f"\n→ Sending '{command}' to {ip}:{port}")
        sock.sendto(command.encode(), (ip, port))
        
        # Wait for response
        data, addr = sock.recvfrom(1024)
        response = data.decode()
        
        print(f"← Response from {addr[0]}:{addr[1]}")
        
        # Try to parse as JSON for pretty printing
        try:
            json_response = json.loads(response)
            print(json.dumps(json_response, indent=2))
        except json.JSONDecodeError:
            print(response)
        
        return response
        
    except socket.timeout:
        print("✗ No response (timeout)")
        print("  Check: Device powered? Correct IP? UDP enabled?")
        return None
        
    except Exception as e:
        print(f"✗ Error: {e}")
        return None
        
    finally:
        sock.close()

def interactive_mode(ip, port):
    """Interactive mode - send multiple commands"""
    print("\n" + "="*50)
    print("UDP Fish Feeder - Interactive Mode")
    print("="*50)
    print(f"Target: {ip}:{port}")
    print("Commands: FEED, STATUS, or type 'quit' to exit")
    print("="*50 + "\n")
    
    while True:
        try:
            command = input("Enter command: ").strip()
            
            if command.lower() in ['quit', 'exit', 'q']:
                print("Goodbye!")
                break
            
            if not command:
                continue
            
            send_udp_command(ip, port, command)
            
        except KeyboardInterrupt:
            print("\n\nInterrupted. Goodbye!")
            break
        except EOFError:
            break

def test_sequence(ip, port):
    """Run a test sequence of commands"""
    print("\n" + "="*50)
    print("UDP Fish Feeder - Test Sequence")
    print("="*50)
    
    tests = [
        ("STATUS", "Get device status"),
        ("FEED", "Trigger food dispensing"),
        ("INVALID", "Test error handling"),
        ("STATUS", "Verify device still responding"),
    ]
    
    passed = 0
    failed = 0
    
    for command, description in tests:
        print(f"\n[Test] {description}")
        result = send_udp_command(ip, port, command)
        
        if result:
            passed += 1
        else:
            failed += 1
        
        time.sleep(1)  # Brief delay between tests
    
    print("\n" + "="*50)
    print(f"Test Results: {passed} passed, {failed} failed")
    print("="*50 + "\n")

def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='ESP32 Fish Feeder UDP Test Utility',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                          # Interactive mode with default IP
  %(prog)s -i 192.168.1.50          # Interactive with custom IP
  %(prog)s FEED                     # Send FEED command
  %(prog)s STATUS -i 192.168.1.50   # Get status from custom IP
  %(prog)s --test                   # Run automated test sequence
        """
    )
    
    parser.add_argument(
        'command',
        nargs='?',
        default=None,
        help='UDP command to send (FEED, STATUS, etc.) - omit for interactive mode'
    )
    
    parser.add_argument(
        '-i', '--ip',
        default=DEFAULT_IP,
        help=f'ESP32 IP address (default: {DEFAULT_IP})'
    )
    
    parser.add_argument(
        '-p', '--port',
        type=int,
        default=DEFAULT_PORT,
        help=f'UDP port number (default: {DEFAULT_PORT})'
    )
    
    parser.add_argument(
        '--test',
        action='store_true',
        help='Run automated test sequence'
    )
    
    args = parser.parse_args()
    
    # Run test sequence
    if args.test:
        test_sequence(args.ip, args.port)
        return
    
    # Send single command
    if args.command:
        send_udp_command(args.ip, args.port, args.command)
        return
    
    # Interactive mode (default)
    interactive_mode(args.ip, args.port)

if __name__ == "__main__":
    main()
