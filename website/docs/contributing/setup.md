# Development Setup

## Clone the repo

```bash
git clone https://github.com/paperclipsvinny/Meshtastic-Injection-Apparatus
cd Meshtastic-Injection-Apparatus
```

## Firmware

```bash
cd firmware
pio run                          # build all environments
pio run -e heltec_wifi_lora_32_V3 -t upload     # flash
pio run -e heltec_wifi_lora_32_V3 -t uploadfs   # flash filesystem
```

## Serial CLI

```bash
pip install pyserial
python3 tools/mia-cli.py /dev/ttyUSB0
```


## Project structure
```
firmware/          PlatformIO project
  src/             C++ source files
  include/         Header files
  data/            LittleFS web dashboard files
  tools/           Python CLI tool
hardware/          KiCad schematics and PCB files
website/           Eleventy documentation site
docs/              Markdown documentation source
examples/          Example payloads and scripts
```