# Meshtastic Integration

MIA is built on top of the Meshtastic protocol. It doesn't modify Meshtastic — it implements a compatible receive/decrypt path so it can listen to a standard mesh and act on specific messages.

## How it fits in

```
[Any Meshtastic node]
        |
        | LoRa (encrypted, AES-128 or AES-256)
        |
[MIA device]
        |
        | USB HID
        |
[Target machine]
```

The operator can be anything that sends Meshtastic messages: the official app on Android/iOS, a stock Heltec, a T-Beam, a T-Deck, another MIA device. MIA only cares about receiving.

## Channels and PSKs

Meshtastic organizes communication into channels, each with its own PSK (pre-shared key). MIA listens on whichever channel its PSK is configured for.

The default PSK (`1PG7OiApB1nwvP+rz05pAQ==`) matches Meshtastic's public **LONGFAST** channel — the default channel all Meshtastic devices use out of the box.

To operate on a private channel:
1. Create a private channel in the Meshtastic app
2. Copy the channel's PSK (shown as base64 in channel settings)
3. Set it on MIA via `set psk <base64>` in the serial CLI or **Settings → Key** in the web dashboard

## The `!mia:` prefix

MIA filters messages by the `!mia:` prefix. Any text message on the configured channel that starts with `!mia:` is treated as a command. Everything after the prefix is passed to the DuckyScript parser.

This means:
- Regular mesh traffic is ignored
- Commands are explicit and intentional
- Multiple MIA devices on the same channel all respond to the same commands (broadcast by design — useful for multi-implant scenarios)

## Retransmission and deduplication

Meshtastic automatically retransmits messages when it doesn't receive delivery confirmation. This is normal behavior but would cause MIA to execute the same command multiple times.

MIA deduplicates by tracking the last executed packet ID and source address. If the same packet arrives again (same ID + same source), it's logged and skipped.

## Radio parameters

MIA ships configured for LONGFAST:

| Parameter | Value |
|---|---|
| Frequency | 906.875 MHz (US) |
| Bandwidth | 250 kHz |
| Spread Factor | 11 |
| Coding Rate | 5 |
| Sync Word | 0x2B |

These can be changed at runtime via the serial CLI or web dashboard. Both MIA and the sending node must use matching parameters to communicate.