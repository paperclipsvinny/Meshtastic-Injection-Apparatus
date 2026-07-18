# Changelog

All notable changes to MIA will be documented in this file.

## [Unreleased]

### Added
- `Logger` class (`firmware/include/Logger.h`, `firmware/src/Logger.cpp`) — centralizes all serial output, replacing direct `Serial.print`/`println`/`printf` calls throughout `main.cpp`. No functional/output change; groundwork for future web-based logging.
- `Version.h` with firmware version constant, printed on boot.
- `HID` class (`firmware/include/HID.h`, `firmware/src/HID.cpp`) — extracts DuckyScript command parsing and USB keyboard execution from `main.cpp` into its own module, following the same pattern as `Logger`. No functional change beyond the fixes listed below.

### Fixed
- `Logger::raw(float)` was using the wrong format specifier (`%d` instead of `%f`), causing RSSI/SNR values to print as garbage (`-2147483648`) instead of correct decimal readings.
- `upload_port` in `platformio.ini` was hardcoded to a Windows-style COM port; corrected for Linux (`/dev/ttyUSB0`).
- `Keyboard.write()` was used alongside `Keyboard.press(modifier)` for CTRL/ALT/SHIFT/GUI combos, which caused the modifier key to remain logically "stuck" held after the command completed — `write()`'s internal key-translation interfered with the already-held modifier's state. Replaced `write()` with explicit `press()`+`release()` pairs for the combo'd key across all four modifier handlers, and added a `Keyboard.releaseAll()` safety-net call at the end of every command as insurance against future stuck-key regressions. Confirmed fixed via live testing (CTRL+T no longer sticks, opens a proper new tab).
- Meshtastic automatically retransmits a message when it doesn't receive delivery confirmation (common with only two nodes on the mesh) — this caused the same command to be re-executed multiple times. Added packet ID + source deduplication in the receive path to skip re-execution of retransmitted copies of an already-handled command. Confirmed fixed via live testing (single execution per logical command, duplicates correctly logged and skipped).

### Changed
- Repository restructured into monorepo layout: `firmware/`, `hardware/`, `website/`, `docs/`, `examples/`. See tag `v0.2.0-restructure`.
