# MIA PCB v1

A custom USB-A form-factor board designed to plug directly into a target machine and disappear — it looks like a USB drive.

## Design goals

- USB-A plug built into the PCB edge (gold-plated contacts) — plugs directly into any USB-A port
- ESP32-S3-WROOM-1 (N16R8) for processing and native USB HID
- E22-900M30S for long-range LoRa comms
- MicroSD slot for payload storage
- Compact enough to leave in a machine without drawing attention

## Current status

**v1 has a hardware bug** — A routing error on v1 boards prevents the E22 LoRa module from
initializing. v2 corrects the routing error and adds BOOT/RESET buttons + an LED.

## Pin mapping

| Signal | ESP32 GPIO |
|---|---|
| LORA_CS (NSS) | 8 |
| LORA_RST | 12 |
| LORA_BUSY | 13 |
| LORA_DIO1 | 14 |
| LORA_SCK | 9 |
| LORA_MOSI | 10 |
| LORA_MISO | 11 |
| SD_CS | 4 |
| SD_DETECT | 5 (optional) |
| USB_D- | 19 (native) |
| USB_D+ | 20 (native) |

## Flashing v1 boards

v1 has no BOOT/RESET buttons. To enter bootloader mode:
1. Bridge GPIO0 (physical pad 10 on the WROOM-1) to GND while powering on
2. Release after the board enumerates
3. Run `pio run -e mia_pcb -t upload`

Native USB auto-reset works once firmware is installed.