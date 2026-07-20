# Firmware Architecture

The firmware is divided into independent subsystems.

## Radio

Responsible for

- receiving packets
- decrypting payloads
- duplicate suppression

## HID

Responsible for

- parsing DuckyScript commands
- executing keyboard actions
- USB initialization

## Logger

Responsible for

- serial output
- debug messages
- error reporting

## Future modules

Parser/
Config/
Payload/
Storage/
CLI/