# Changelog

All notable changes to MIA will be documented in this file.

## [v0.5.0]
- Added `SerialApi` module (`SerialApi.h` / `SerialApi.cpp`) — JSON-based command
  interface over USB serial for reading and modifying radio config at runtime.


## [v0.4.0]
-Moving towards changing radio variables over Serial (and eventually web).
- Convert Radio class from static to instance-based methods, enabling runtime-editable radio config (spread factor, power)
- Add getters/setters for radio frequency, bandwidth, spread factor, coding rate, and power
- Rename internal SX1262 driver object to objectSX1262 to avoid naming collision with the new Radio instance 


## [v0.3.3]
-Introduced `StatusLED` module (`Status.LED` class), which is responsible for the status LED blink.
-Notes: Currently hardcoded to blink three times on recieve.

## [v0.3.2]
-Introduced `Crypto` module (`Crypto.h` / `Crypto.cpp`) as the logic responsible for AES-CTR decryption. 
**Crypto is now responsible for:**
- Initializing the AES engine with a 128-bit key (`Crypto::begin(key)`)
- Constructing the per-packet nonce from packet ID + source address, per Meshtastic's
  CTR-mode scheme (`Crypto::buildNonce`, private)
- Decrypting a packet's payload given its ciphertext, length, packet ID, and source
  address (`Crypto::decrypt`)

Note that the key value is still stored in Main. 

## [v0.3.1]
### Added
- Introduced `Radio` module (`Radio.h` / `Radio.cpp`) as the dedicated interface for SX1262 radio communication.
- Added a public hardware abstraction layer for:
  - Radio initialization
  - Packet reception
  - Receive interrupt handling
  - RSSI retrieval
  - SNR retrieval
  - IRQ flag access

### Changed
- Moved all LoRa hardware configuration, SPI initialization, and RadioLib setup out of `main.cpp`.
- Replaced direct `radio` object access throughout the application with calls to the `Radio` interface.
- Centralized LoRa configuration constants inside the radio module.

### Internal
- Reduced responsibility of `main.cpp`; it now coordinates application flow instead of managing radio hardware directly.
- Continued migration toward a modular firmware architecture in preparation for additional subsystem extraction.

##[v0.2.0]
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
