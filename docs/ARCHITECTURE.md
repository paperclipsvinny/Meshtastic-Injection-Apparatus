# Firmware Architecture

The firmware is divided into independent subsystems, each owning a single concern.

## Radio
- SPI/LoRa hardware initialization via RadioLib (SX1262/E22)
- Runtime-configurable parameters: frequency, bandwidth, spread factor, coding rate, TX power
- RX interrupt handling and packet reception
- Signal reporting: RSSI, SNR, IRQ flags
- NVS-backed persistence — settings survive reboot
- Getters/setters exposed to SerialApi and WebApi

## Crypto
- AES-CTR decryption using ESP32 hardware AES (mbedTLS)
- Supports AES-128 (16-byte) and AES-256 (32-byte) PSKs
- Per-packet nonce construction from packet ID + source address (Meshtastic CTR scheme)
- Runtime PSK update via setKey() — no reflash required
- NVS-backed persistence for active PSK

## PacketParser
- Parses the 14-byte Meshtastic packet header (dest, source, packetId, flags, channel)
- Extracts text messages from decrypted protobuf payloads (text app message format)
- Deduplication handled in main.cpp via packetId + source tracking

## HID
- DuckyScript command parsing and execution
- USB HID keyboard initialization and keystroke injection
- Handles modifier keys (CTRL, ALT, SHIFT, GUI) with press/release pairs
- Retransmission deduplication prevents duplicate command execution

## Logger
- Centralized serial output (raw, info, formatted, hex byte)
- Replaces all direct Serial.print calls throughout the firmware

## StatusLED
- Visual feedback via onboard LED
- 3x fast blink pattern on packet received
- Conditionally compiled out on boards with no LED (MIA_PCB)

## SerialApi
- JSON-over-serial command interface (115200 baud, newline-terminated)
- Commands: get_config, set_config, get_psk, set_psk, get_wifi, set_wifi,
  save_defaults, reset_defaults
- All changes applied live and persisted to NVS immediately

## WebApi
- HTTP server over ESP32 WiFi AP (ESPAsyncWebServer)
- Mirrors SerialApi's command surface over REST endpoints
- Serves web dashboard (index.html) from LittleFS
- WPA2-secured AP; POST endpoints require Content-Type: application/json

## WifiConfig
- Holds AP SSID, password, and enabled state
- NVS-backed persistence via Preferences
- Read/written by both SerialApi (set_wifi) and WebApi

## Multi-board support
Firmware supports multiple hardware targets via PlatformIO environments:
- `heltec_wifi_lora_32_V3` — Heltec WiFi LoRa 32 V3 (reference/tested platform)
- `mia_pcb` — MIA v1 custom PCB (ESP32-S3-WROOM-1 + E22-900M30S, pending hardware fix)

Board-specific differences (LED, VEXT_ENABLE) are handled via `#ifdef MIA_PCB` guards.

## Tools
- `tools/mia-cli.py` — interactive serial CLI (Python, `cmd` module)
  Commands: get/set/gen/defaults/monitor
  Connects over USB serial, no firmware changes required