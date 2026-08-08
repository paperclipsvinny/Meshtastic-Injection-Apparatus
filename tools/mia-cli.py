#!/usr/bin/env python3
"""
MIA Serial CLI
Mesh Injection Apparatus — interactive serial command interface
Usage: python3 mia-cli.py <port> [--baud 115200]
"""

import cmd
import serial
import json
import time
import sys
import base64
import os


BANNER = """
===============================
  M.esh I.njection A.pparatus
     Serial Command Interface
===============================
Type 'help' for available commands.
Type 'help <command>' for usage.
"""


class MiaCli(cmd.Cmd):
    intro = BANNER
    prompt = "mia> "

    def __init__(self, port, baud=115200):
        super().__init__()
        self.port = port
        self.baud = baud
        self.ser = None

    # Serial Connection 

    def connect(self):
        try:
            self.ser = serial.Serial(self.port, self.baud, timeout=2)
            print(f"[+] Connected to {self.port} at {self.baud} baud")
            print("[*] Waiting for device to boot...")
            time.sleep(5)
            self.ser.reset_input_buffer()
            print("[+] Ready.\n")
        except Exception as e:
            print(f"[-] Failed to connect: {e}")
            sys.exit(1)

    def send(self, cmd_dict):
        """Send a JSON command to the device and return the parsed response."""
        if not self.ser:
            print("[-] Not connected.")
            return None
        line = json.dumps(cmd_dict) + "\n"
        self.ser.write(line.encode())
        time.sleep(0.5)
        raw = self.ser.read(self.ser.in_waiting or 1)
        if not raw:
            print("[-] No response from device.")
            return None
        # pull the JSON line out, ignoring RSSI heartbeats and other output
        for ln in raw.decode(errors="replace").splitlines():
            ln = ln.strip()
            if ln.startswith("{"):
                try:
                    return json.loads(ln)
                except json.JSONDecodeError:
                    pass
        print(f"[-] Unexpected response: {raw.decode(errors='replace').strip()}")
        return None

    # get command blocks

    def do_get(self, arg):
        """
Get a value from the device.

  get config    radio parameters (frequency, SF, power, etc.)
  get psk       current PSK as base64
  get wifi      AP SSID and enabled state
  get status    all of the above at once
        """
        sub = arg.strip().lower()

        if sub == "config":
            resp = self.send({"cmd": "get_config"})
            if resp:
                print(f"  Frequency:     {resp.get('frequency')} MHz")
                print(f"  Bandwidth:     {resp.get('bandwidth')} kHz")
                print(f"  Spread Factor: {resp.get('spreadFactor')}")
                print(f"  Coding Rate:   {resp.get('codingRate')}")
                print(f"  Power:         {resp.get('power')} dBm")

        elif sub == "psk":
            resp = self.send({"cmd": "get_psk"})
            if resp:
                psk = resp.get("psk", "")
                try:
                    bits = len(base64.b64decode(psk)) * 8
                except Exception:
                    bits = "?"
                print(f"  PSK ({bits}-bit): {psk}")

        elif sub == "wifi":
            resp = self.send({"cmd": "get_wifi"})
            if resp:
                enabled = resp.get('enabled')
                print(f"  AP:   {'ON' if enabled else 'OFF'}")
                print(f"  SSID: {resp.get('ssid')}")

        elif sub == "status":
            print("\n[ Radio Config ]")
            self.do_get("config")
            print("\n[ WiFi ]")
            self.do_get("wifi")
            print("\n[ PSK ]")
            self.do_get("psk")
            print()

        else:
            print("[-] Usage: get <config|psk|wifi|status>")

    # set command blocks

    def do_set(self, arg):
        """
Set a value on the device. Changes persist across reboots.

  set sf <7-12>          spread factor (lower = faster, shorter range)
  set power <dBm>        TX power in dBm (-9 to 22)
  set psk <base64>       AES key as base64-encoded 16 or 32 bytes
  set ssid <name>        WiFi AP network name
  set password <pass>    WiFi AP password
  set wifi on            enable the WiFi AP
  set wifi off           disable the WiFi AP
        """
        parts = arg.strip().split(None, 1)
        if not parts:
            print("[-] Usage: set <sf|power|psk|ssid|password|wifi> <value>")
            return

        sub = parts[0].lower()
        value = parts[1].strip() if len(parts) > 1 else ""

        if sub == "sf":
            try:
                sf = int(value)
                if not 7 <= sf <= 12:
                    print("[-] Spread factor must be between 7 and 12.")
                    return
                resp = self.send({"cmd": "set_config", "spreadFactor": sf})
                if resp:
                    print(f"[+] Spread factor set to {sf} — {resp.get('status')}")
            except ValueError:
                print("[-] Usage: set sf <7-12>")

        elif sub == "power":
            try:
                power = int(value)
                resp = self.send({"cmd": "set_config", "power": power})
                if resp:
                    print(f"[+] Power set to {power} dBm — {resp.get('status')}")
            except ValueError:
                print("[-] Usage: set power <dBm>")

        elif sub == "psk":
            if not value:
                print("[-] Usage: set psk <base64>")
                return
            try:
                decoded = base64.b64decode(value)
                if len(decoded) not in (16, 32):
                    print(f"[-] Key must decode to 16 or 32 bytes, got {len(decoded)}.")
                    return
            except Exception:
                print("[-] Invalid base64 string.")
                return
            resp = self.send({"cmd": "set_psk", "psk": value})
            if resp:
                bits = len(decoded) * 8
                print(f"[+] {bits}-bit PSK applied — {resp.get('status')}")

        elif sub == "ssid":
            if not value:
                print("[-] Usage: set ssid <name>")
                return
            resp = self.send({"cmd": "set_wifi", "ssid": value})
            if resp:
                print(f"[+] SSID set to '{value}' — {resp.get('status')}")

        elif sub == "password":
            if not value:
                print("[-] Usage: set password <pass>")
                return
            resp = self.send({"cmd": "set_wifi", "password": value})
            if resp:
                print(f"[+] Password updated — {resp.get('status')}")
        
        elif sub == "freq":
            try:
                freq = float(value)
                resp = self.send({"cmd": "set_config", "frequency": freq})
                if resp:
                    print(f"[+] Frequency set to {freq} MHz — {resp.get('status')}")
            except ValueError:
                print("[-] Usage: set freq <MHz>")

        elif sub == "bw":
            try:
                bw = float(value)
                resp = self.send({"cmd": "set_config", "bandwidth": bw})
                if resp:
                    print(f"[+] Bandwidth set to {bw} kHz — {resp.get('status')}")
            except ValueError:
                print("[-] Usage: set bw <kHz>")

        elif sub == "cr":
            try:
                cr = int(value)
                resp = self.send({"cmd": "set_config", "codingRate": cr})
                if resp:
                    print(f"[+] Coding rate set to {cr} — {resp.get('status')}")
            except ValueError:
                print("[-] Usage: set cr <5-8>")

        elif sub == "wifi":
            if value.lower() == "on":
                resp = self.send({"cmd": "set_wifi", "enabled": True})
                if resp:
                    print(f"[+] WiFi AP enabled — {resp.get('status')}")
                    print("[!] Reboot required if the AP wasn't already running.")
            elif value.lower() == "off":
                resp = self.send({"cmd": "set_wifi", "enabled": False})
                if resp:
                    print(f"[+] WiFi AP disabled — {resp.get('status')}")
            else:
                print("[-] Usage: set wifi <on|off>")

        else:
            print(f"[-] Unknown setting: '{sub}'")
            print("    Options: sf, power, psk, ssid, password, wifi")

    # gen command (useful if you can't find an option for channels in meshtastic.)

    def do_gen(self, arg):
        """
Generate a random value and apply it to the device.

  gen psk 128    random 128-bit PSK (16 bytes)
  gen psk 256    random 256-bit PSK (32 bytes)
        """
        parts = arg.strip().split()
        if len(parts) < 2:
            print("[-] Usage: gen psk <128|256>")
            return

        sub = parts[0].lower()
        value = parts[1]

        if sub == "psk":
            if value not in ("128", "256"):
                print("[-] Usage: gen psk <128|256>")
                return
            key_bytes = 16 if value == "128" else 32
            key = os.urandom(key_bytes)
            b64 = base64.b64encode(key).decode()
            print(f"[+] Generated {value}-bit PSK: {b64}")
            resp = self.send({"cmd": "set_psk", "psk": b64})
            if resp:
                print(f"[+] Applied to device — {resp.get('status')}")
        else:
            print(f"[-] Unknown: gen {sub}")

    # monitor command (live serial, allows RSSI heartbeat to resume)

    def do_monitor(self, arg):
        """
Stream all device serial output live. Press Ctrl+C to stop.

  monitor
        """
        print("[*] Streaming output. Press Ctrl+C to stop.\n")
        try:
            while True:
                data = self.ser.read(self.ser.in_waiting or 1)
                if data:
                    print(data.decode(errors="replace"), end="", flush=True)
        except KeyboardInterrupt:
            print("\n\n[*] Monitor stopped.")

    # exit

    def do_exit(self, arg):
        """Exit the MIA CLI."""
        print("Disconnecting...")
        if self.ser:
            self.ser.close()
        return True

    do_quit = do_exit
    do_EOF = do_exit

    def do_help(self, arg):
        """Show help for a command, or list all commands."""
        if arg:
            try:
                func = getattr(self, 'do_' + arg)
                print(func.__doc__ or f"No help available for '{arg}'")
            except AttributeError:
                print(f"[-] Unknown command: '{arg}'")
            return

        print("""
Commands
────────────────────────────────────────
  get <config|psk|wifi|status>     read values from the device
  set <setting> <value>            write a value to the device
  gen <psk> <128|256>              generate and apply a random value
  defaults <save|reset>            manage saved default config
  monitor                          stream live serial output
  exit                             disconnect and quit

Settings (for 'set'):
────────────────────────────────────────
  sf <7-12>          spread factor
  power <dBm>        TX power (-9 to 22)
  freq <MHz>         frequency (e.g. 906.875)
  bw <kHz>           bandwidth (e.g. 250.0)
  cr <5-8>           coding rate
  psk <base64>       AES channel key
  ssid <name>        WiFi AP name
  password <pass>    WiFi AP password
  wifi <on|off>      enable or disable the WiFi AP

Type 'help <command>' for detailed usage.
────────────────────────────────────────""")

    def default(self, line):
        print(f"[-] Unknown command: '{line}'. Type 'help' for available commands.")

    def do_defaults(self, arg):
        """
Manage default configuration values.

  defaults save     save current config as the restore point for 'defaults reset'
  defaults reset    restore previously saved defaults (or factory values if none saved)
        """
        sub = arg.strip().lower()

        if sub == "save":
            resp = self.send({"cmd": "save_defaults"})
            if resp:
                print(f"[+] {resp.get('msg', resp)}")
                print("[*] Current config is now your restore point.")

        elif sub == "reset":
            confirm = input("[!] This will revert all settings. Type 'yes' to confirm: ")
            if confirm.strip().lower() != "yes":
                print("[*] Cancelled.")
                return
            resp = self.send({"cmd": "reset_defaults"})
            if resp:
                print(f"[+] {resp.get('msg', resp)}")
                print("[*] Reboot the device to apply.")
                

        else:
            print("[-] Usage: defaults <save|reset>")

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="MIA Serial CLI — Mesh Injection Apparatus"
    )
    parser.add_argument(
        "port",
        nargs="?",
        default="/dev/ttyUSB0",
        help="Serial port (default: /dev/ttyUSB0)"
    )
    parser.add_argument(
        "--baud",
        default=115200,
        type=int,
        help="Baud rate (default: 115200)"
    )
    args = parser.parse_args()

    cli = MiaCli(args.port, args.baud)
    cli.connect()
    try:
        cli.cmdloop()
    except KeyboardInterrupt:
        print("\nInterrupted.")
        if cli.ser:
            cli.ser.close()