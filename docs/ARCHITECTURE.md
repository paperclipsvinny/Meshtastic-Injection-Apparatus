# Firmware Architecture
The firmware is divided into independent subsystems.

## Radio
Responsible for:
- SPI/LoRa hardware initialization (SX1262 config: frequency, bandwidth, SF, CR, sync word)
- RX interrupt handling (packetAvailable)
- Reading raw packet bytes off the radio (receivePacket)
- Reporting signal status (RSSI, SNR, IRQ flags)

## Crypto
Responsible for:
- Initializing the AES engine with a 128-bit key
- Constructing the per-packet nonce from packet ID + source address (CTR mode)
- Decrypting a packet's payload into plaintext bytes

## HID
Responsible for:
- parsing DuckyScript commands
- executing keyboard actions
- USB initialization

## Logger
Responsible for:
- serial output
- debug messages
- error reporting

## Future modules
Parser/     — protobuf parsing (extract text messages from decrypted payload)
Config/     — runtime radio/channel configuration (NVS-backed, serial commands)
Payload/    —
Storage/    —
CLI/        — serial command handling (get/set/apply/save)
