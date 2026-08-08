# Serial API Reference

MIA accepts JSON commands over USB serial at **115,200 baud**. Each command is a single line of JSON terminated with a newline (`\n`).

The easiest way to interact with the serial API is the included CLI tool:

```bash
python3 tools/mia-cli.py /dev/ttyUSB0
```

Or raw serial (useful for scripting):

```bash
python3 -c "
import serial, time
s = serial.Serial('/dev/ttyUSB0', 115200, timeout=2)
time.sleep(5)          # wait for boot
s.reset_input_buffer()
s.write(b'{\"cmd\":\"get_config\"}\n')
time.sleep(0.5)
print(s.read(s.in_waiting or 1).decode())
"
```

> **Note:** Wait a few seconds after opening the port for the device to finish booting before sending commands. Sending too early can mix your command with boot output and cause a parse error.

---

## Radio configuration

### `get_config`

Returns current radio parameters.

**Request:**
```json
{"cmd": "get_config"}
```

**Response:**
```json
{
  "frequency": 906.875,
  "bandwidth": 250,
  "spreadFactor": 11,
  "codingRate": 5,
  "power": 22
}
```

---

### `set_config`

Updates one or more radio parameters. Only include the fields you want to change. Changes take effect immediately and persist across reboots.
- `frequency`: frequency in MHz (e.g. 906.875)
- `bandwidth`: bandwidth in kHz (e.g. 250.0)  
- `codingRate`: coding rate (5–8)
Example:
**Request:**
```json
{"cmd": "set_config", "spreadFactor": 9, "power": 17}
```

**Response:**
```json
{"status": "ok"}
```


**Valid ranges:**
- `spreadFactor`: 7–12 (RadioLib validates this; invalid values return `{"status":"error"}`)
- `power`: −9 to 22 dBm

---

## PSK (channel key)

### `get_psk`

Returns the current AES channel key as base64.

**Request:**
```json
{"cmd": "get_psk"}
```

**Response:**
```json
{"psk": "1PG7OiApB1nwvP+rz05pAQ=="}
```

The response includes the raw base64 key. Decode it to get the actual bytes: 24 characters = 16 bytes (AES-128), 44 characters = 32 bytes (AES-256).

---

### `set_psk`

Sets a new AES channel key. Must be a valid base64 string that decodes to exactly 16 bytes (AES-128) or 32 bytes (AES-256).

**Request:**
```json
{"cmd": "set_psk", "psk": "uUAKMhSSaaZpRNM0ytp6qw=="}
```

**Response:**
```json
{"status": "ok"}
```

To match a Meshtastic private channel, find the PSK in the Meshtastic app's channel settings (shown as base64) and pass it directly here.

---

## WiFi / AP settings

### `get_wifi`

Returns current WiFi AP configuration. The password is intentionally omitted from the response.

**Request:**
```json
{"cmd": "get_wifi"}
```

**Response:**
```json
{"enabled": true, "ssid": "Mesh Injection Apparatus"}
```

---

### `set_wifi`

Updates WiFi AP settings. Any combination of fields can be included. SSID and password changes require a reboot to take effect; the `enabled` toggle applies on the next boot.

**Request:**
```json
{"cmd": "set_wifi", "ssid": "MyMIA", "password": "newpassword", "enabled": true}
```

**Response:**
```json
{"status": "ok"}
```

---

## Defaults

### `save_defaults`

Saves the current configuration (radio, PSK, WiFi) as a named restore point. A subsequent `reset_defaults` will return to these values rather than the compiled-in factory values.

**Request:**
```json
{"cmd": "save_defaults"}
```

**Response:**
```json
{"status": "ok", "msg": "current config saved as defaults"}
```

---

### `reset_defaults`

Restores previously saved defaults. If no defaults have been saved, wipes NVS entirely and reverts to compiled-in factory values on next reboot.

**Request:**
```json
{"cmd": "reset_defaults"}
```

**Response:**
```json
{"status": "ok", "msg": "defaults restored, reboot to apply"}
```

or, if no saved defaults exist:

```json
{"status": "ok", "msg": "factory reset, reboot to apply"}
```

---

## Error responses

All commands return a consistent error format for invalid input:

| Response | Meaning |
|---|---|
| `{"error":"invalid json"}` | The line sent wasn't valid JSON |
| `{"error":"unknown cmd"}` | The `cmd` field wasn't recognized |
| `{"error":"psk must decode to 16 or 32 bytes"}` | The PSK base64 string decoded to an invalid length |
| `{"status":"error"}` | The radio rejected a parameter (e.g. out-of-range spread factor) |

---

## Serial CLI quick reference

```
mia> get config              read radio parameters
mia> get psk                 read current PSK
mia> get wifi                read AP settings
mia> get status              read everything at once

mia> set sf <7-12>           set spread factor
mia> set power <dBm>         set TX power
mia> set psk <base64>        set channel key
mia> set ssid <name>         set AP network name
mia> set password <pass>     set AP password
mia> set wifi <on|off>       enable or disable the AP

mia> set freq <MHz>          set frequency
mia> set bw <kHz>            set bandwidth  
mia> set cr <5-8>            set coding rate

mia> gen psk 128             generate and apply a random 128-bit key
mia> gen psk 256             generate and apply a random 256-bit key

mia> defaults save           save current config as restore point
mia> defaults reset          restore saved defaults (or factory reset)

mia> monitor                 stream live serial output
mia> exit                    disconnect
```