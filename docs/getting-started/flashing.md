# Flashing

## Option A: Web flasher (recommended)

Visit **[themia.dev/flash](https://themia.dev/flash)** in Chrome or Edge.

1. Plug in your device via USB
2. Select your board from the dropdown
3. Click **Flash**
4. Wait ~30 seconds for the firmware and filesystem to write
5. The device reboots automatically when done

No software installation required. Chrome or Edge only — Web Serial API isn't supported in Firefox or Safari.

## Option B: PlatformIO

For building from source or flashing a custom build:

```bash
git clone https://github.com/paperclipsvinny/Meshtastic-Injection-Apparatus
cd Meshtastic-Injection-Apparatus/firmware

# install dependencies (first time only)
pio pkg install

# flash firmware
pio run -e heltec_wifi_lora_32_V3 -t upload

# flash web dashboard to device filesystem (required)
pio run -e heltec_wifi_lora_32_V3 -t uploadfs
```

Both commands are required. `-t upload` writes the firmware binary; `-t uploadfs` writes the web dashboard files to LittleFS. Missing the second step results in a blank page at `http://192.168.4.1`.

## Option C: esptool directly

If you have pre-built binaries:

```bash
esptool.py --chip esp32s3 --port /dev/ttyACM0 --baud 460800 write_flash \
  0x0      bootloader.bin \
  0x8000   partitions.bin \
  0xe000   boot_app0.bin \
  0x10000  firmware.bin
```

## Entering bootloader mode

Most ESP32-S3 boards with native USB auto-reset into download mode when esptool connects — no button press needed.

If it fails to connect, manually enter bootloader mode:
1. Hold the **BOOT** button (or bridge GPIO0 to GND on boards without a button)
2. Press and release **RESET** (or briefly bridge EN to GND)
3. Release BOOT
4. Run the flash command