# Heltec WiFi LoRa 32 V3

The reference platform for MIA development. If you're just getting started, use this board.

## Specs

- **MCU:** ESP32-S3, 240MHz dual-core
- **Flash:** 8MB
- **LoRa:** SX1262, up to 22dBm TX power
- **USB:** Native USB-C (no separate UART bridge chip)
- **Display:** 0.96" OLED (unused by MIA firmware)
- **Buttons:** BOOT + RESET (makes flashing easy)

## Notes

- The Heltec V3 has a TCXO (temperature-compensated crystal oscillator) on the LoRa module at 1.8V — this is why `LORA_TXCOVOLT 1.8` is set in `Radio.h`. Don't remove this or the radio won't initialize.
- Native USB means the board enumerates as `/dev/ttyACM0` (not `/dev/ttyUSB0`) on Linux when using USB CDC mode. The `platformio.ini` `upload_port` should match whichever your system assigns.
- `ARDUINO_USB_CDC_ON_BOOT=0` is set for this board — serial output is available but the CDC interface doesn't start until after `Serial.begin()` in setup.
- `VEXT_ENABLE` (GPIO36, active LOW) controls power to peripherals including the display. MIA pulls this LOW on boot to enable the LoRa module's power path.