# Hardware Overview

MIA firmware runs on ESP32-S3-based LoRa boards. The reference platform is the **Heltec WiFi LoRa 32 V3**.

## Supported boards

| Board | LoRa Module | Status |
|---|---|---|
| Heltec WiFi LoRa 32 V3 | SX1262 | ✅ Tested |
| MIA PCB v1 | E22-900M30S (SX1262-compatible) | 🔧 v2 in development |

## Pin mapping (Heltec V3 / MIA PCB v1)

| Signal | GPIO |
|---|---|
| LORA_CS | 8 |
| LORA_DIO1 | 14 |
| LORA_RST | 12 |
| LORA_BUSY | 13 |
| LORA_MISO | 11 |
| LORA_MOSI | 10 |
| LORA_SCK | 9 |

These are defined in `Radio.h` and can be changed per board via `#ifdef` guards in `platformio.ini`.

## Adding a new board

1. Add a new `[env:your_board]` in `platformio.ini`
2. Add `-DYOUR_BOARD` to `build_flags`
3. Add an `#ifdef YOUR_BOARD` pin block in `Radio.h`
4. Add `#ifndef YOUR_BOARD` guards around any board-specific peripherals in `main.cpp` (LED, VEXT, etc.)

See [Building](../firmware/building.md) for the full environment setup.