# Requirements

## Hardware

You need one of these:

| Board | Notes |
|---|---|
| **Heltec WiFi LoRa 32 V3** | Reference platform. Tested and confirmed working. Has built-in display (unused by MIA), OLED, USB-C. |
| **MIA PCB v1** | Custom ESP32-S3 + E22-900M30S board. v2 in development. |

Any ESP32-S3 board with an SX1262-compatible LoRa module should work with minor pin changes — see [Building](../firmware/building.md).

You also need:
- A **USB data cable** (not charge-only — MIA enumerates as a USB keyboard)
- A **Meshtastic node** to send commands from — any standard device works (phone app, stock Heltec, T-Beam, etc.)
- An **antenna** for the LoRa module (900 MHz for US, 868 MHz for EU)

## Software

| Tool | Required for |
|---|---|
| Chrome or Edge | Web flasher at themia.dev |
| Python 3 + pyserial | Serial CLI (`tools/mia-cli.py`) |
| PlatformIO | Building from source |
| Meshtastic app | Sending commands over the mesh |

## OS support

MIA has been tested on **Linux** (Fedora, Ubuntu). It should work on macOS and Windows with the standard PlatformIO toolchain — serial port naming will differ (`/dev/cu.usbmodem*` on macOS, `COM*` on Windows).