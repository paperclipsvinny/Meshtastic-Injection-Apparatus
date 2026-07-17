# Changelog

All notable changes to MIA will be documented in this file.

## [Unreleased]

### Added
- `Logger` class (`firmware/include/Logger.h`, `firmware/src/Logger.cpp`) — centralizes all serial output, replacing direct `Serial.print`/`println`/`printf` calls throughout `main.cpp`. No functional/output change; groundwork for future web-based logging.
- `Version.h` with firmware version constant, printed on boot.
- Packet ID + source deduplication in the Meshtastic receive path — prevents Meshtastic's automatic retransmissions (when a node doesn't receive delivery confirmation) from being re-executed as duplicate commands.

### Fixed
- `Logger::raw(float)` was using the wrong format specifier (`%d` instead of `%f`), causing RSSI/SNR values to print as garbage (`-2147483648`) instead of correct decimal readings.
- `upload_port` in `platformio.ini` was hardcoded to a Windows-style COM port; corrected for Linux (`/dev/ttyUSB0`).

### Changed
- Repository restructured into monorepo layout: `firmware/`, `hardware/`, `website/`, `docs/`, `examples/`. See tag `v0.2.0-restructure`.
