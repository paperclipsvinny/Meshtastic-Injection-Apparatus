# Quickstart

Five (ish) minutes from unboxing to your first injection.

## 1. Flash

Visit **[themia.dev/flash](https://themia.dev/flash)**, plug in your device, click Flash.

## 2. Connect to the dashboard (optional)

After booting, connect to the MIA WiFi AP if you want to control it over the ESP32's WiFi portal:

- **SSID:** `Mesh Injection Apparatus`
- **Password:** `Mesh-Inject-7f3K9pQ2`

Open **http://192.168.4.1** in your connected device's browser. From here you can change radio and crypto settings as well as upload payloads, and more. 

This is completely optional, (but useful if you don't have a second Meshtastic node handy to send commands from) and it can be disabled via settings (see firmware for more details). 

## 3. Plug into a target

Plug the MIA device into the machine you want to inject keystrokes into. It shows up as a USB keyboard — no driver install needed.

## 4. Send a command

From the **Meshtastic app** on any node on the same channel, send:

```
!mia: STRINGLN hello world
```

The target machine types `hello world` and presses Enter.

That's it. Everything else — private channels, custom payloads, radio settings — is optional from here.

---

## What just happened

Your Meshtastic node transmitted that message over LoRa. MIA received it, saw the `!mia:` prefix, decrypted the payload using the shared channel key, and passed the DuckyScript command to the USB HID stack, which typed it on the target.

The whole chain is encrypted in transit using AES (128-bit by default on the public LONGFAST channel). Switch to a private channel with a custom PSK "to keep your OPSEC clean". 