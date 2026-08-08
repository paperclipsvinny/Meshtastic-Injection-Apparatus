# Security Considerations

## PSK handling

MIA stores its channel PSK in NVS (ESP32 non-volatile storage). This is flash memory on the device itself — not transmitted anywhere, not stored in plaintext in any file that gets committed to git.

The PSK is exposed over:
- **Serial CLI** via `get psk` — requires physical USB access to the device
- **Web dashboard** via `GET /api/psk` — requires connecting to the MIA WiFi AP

The WiFi AP is WPA2-protected. Anyone who doesn't know the AP password cannot reach the PSK endpoint. The PSK is transmitted in transit as base64 JSON over HTTP — which is plaintext at the HTTP layer, but already encrypted at the WiFi (WPA2) layer. This is an acceptable trust boundary for a local AP.

## AP security model

The MIA AP is WPA2-secured with a configurable password. The default password (`Mesh-Inject-7f3K9pQ2`) should be changed before operational deployment:

```
mia> set password <your-strong-password>
```

Anyone who connects to the AP has full access to the web dashboard and all API endpoints, including the ability to change the PSK, fire HID commands, and reboot the device. Treat AP access as equivalent to physical access.

## Channel security

The `!mia:` prefix is not authenticated beyond the shared channel PSK. Anyone who knows the channel key and is on the same Meshtastic frequency can send commands to a MIA device. This is intentional for the multi-operator use case but means the channel PSK is your primary access control mechanism.

Use a private channel with a strong, randomly generated PSK for any non-public deployment:

```
mia> gen psk 256
```

## Responsible use

MIA is a security research and education tool. Use it only on systems and networks you own or have explicit written permission to test. Unauthorized HID injection is illegal in most jurisdictions.

The author is not responsible for misuse.