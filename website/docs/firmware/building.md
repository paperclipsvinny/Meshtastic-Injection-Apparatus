# Building from Source

## Prerequisites

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) or the PlatformIO IDE extension for VS Code
- Python 3 (for the serial CLI tool)
- Git

## Clone and build

```bash
git clone https://github.com/paperclipsvinny/Meshtastic-Injection-Apparatus
cd Meshtastic-Injection-Apparatus/firmware

# build (compiles all environments)
pio run

# build a specific environment
pio run -e heltec_wifi_lora_32_V3
pio run -e mia_pcb
```

## Flash

```bash
# flash firmware to Heltec V3
pio run -e heltec_wifi_lora_32_V3 -t upload

# flash web dashboard to device filesystem (required separately)
pio run -e heltec_wifi_lora_32_V3 -t uploadfs
```

Always run both commands when setting up a new device. Firmware and filesystem are separate flash operations.

## Environments

| Environment | Board | Upload port |
|---|---|---|
| `heltec_wifi_lora_32_V3` | Heltec WiFi LoRa 32 V3 | `/dev/ttyUSB0` |
| `mia_pcb` | MIA PCB v1 (ESP32-S3-WROOM-1) | `/dev/ttyACM0` |

Ports are set in `platformio.ini` — update `upload_port` to match your system if needed.

## Key build flags

| Flag | Environment | Effect |
|---|---|---|
| `-DMIA_PCB` | `mia_pcb` | Disables LED, VEXT_ENABLE; selects MIA PCB pin defs |
| `-DARDUINO_USB_MODE=1` | both | Enables native USB mode |
| `-DARDUINO_USB_CDC_ON_BOOT=0` | Heltec | CDC serial starts after `Serial.begin()` |
| `-DARDUINO_USB_CDC_ON_BOOT=1` | MIA PCB | CDC serial available from boot |

## Dependencies

All managed by PlatformIO automatically:

- `jgromes/RadioLib` — LoRa radio abstraction
- `bblanchon/ArduinoJson@^7.0.0` — JSON parsing for serial/web API
- `esphome/ESPAsyncWebServer-esphome@^3.0.0` — async HTTP server

## Filesystem

Web dashboard files live in `firmware/data/`. Upload them to the device:

```bash
pio run -e heltec_wifi_lora_32_V3 -t uploadfs
```

Note: This is separate from the firmware flash and uses LittleFS. Both must be done for the web dashboard to work.