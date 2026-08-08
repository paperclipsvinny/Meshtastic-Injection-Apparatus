# Bill of Materials — MIA PCB v1

Custom USB-A form-factor board designed to plug directly into a target machine.

| Ref | Component | Value | Notes |
|---|---|---|---|
| U1 | ESP32-S3-WROOM-1 | N16R8 | 16MB flash, 8MB PSRAM |
| U2 | E22-900M30S | — | SX1262-based, 30dBm, 900MHz |
| U3 | AMS1117-3.3 | 3.3V LDO | Powers ESP32 + E22 + SD |
| J1 | Hirose DM3AT | MicroSD slot | SPI mode |
| J2 | USB-A connector | PCB edge | Gold-plated castellated contacts |
| C1 | 10µF | VBUS bypass | |
| C2 | 10µF | 3.3V output bypass | |
| C3 | 22µF | 3.3V bulk | |
| C4 | 100nF | ESP32 decoupling | |
| C5 | 100nF | EN pin bypass | |
| C6 | 100nF | E22 decoupling | |
| C7 | 100nF | SD card decoupling | |
| R1 | 10kΩ | EN pull-up | |

## Known issues (v1)

- MOSI and MISO pads on U2 (E22) are shorted on the PCB — radio does not initialize. v2 corrects this routing error.
- No BOOT or RESET buttons — bootloader entry requires bridging GPIO0 to GND manually.

## v2 planned fixes

- Correct MOSI/MISO routing
- Add BOOT + RESET tactile buttons
- Verify all E22 control pin connections end-to-end before fab