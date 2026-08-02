# MIA Documentation

## Serial API

Connect to the device over USB serial at **115200 baud**. Commands are
single-line JSON objects, terminated with `\n`.

> **Note:** wait a few seconds after opening the port for the boot banner
> to finish printing before sending commands — sending too early can mix
> your command with boot log output and cause a parse error.

### `get_config`

Returns the current radio configuration.

**Send:**
```json
{"cmd":"get_config"}
```

**Response:**
```json
{"frequency":906.875,"bandwidth":250,"spreadFactor":11,"codingRate":5,"power":22}
```

### `set_config`

Updates one or more radio settings. Only include the fields you want to
change.

**Send:**
```json
{"cmd":"set_config","spreadFactor":9,"power":17}
```

**Response:**
```json
{"status":"ok"}
```
Returns `{"status":"error"}` if the radio rejected a value (e.g. an
unsupported spread factor).

### Errors

| Response | Meaning |
|---|---|
| `{"error":"invalid json"}` | The line sent wasn't valid JSON |
| `{"error":"unknown cmd"}` | `cmd` field wasn't recognized |

### Notes

- Changes take effect immediately but are **not persisted** — they reset to
  firmware defaults on reboot. Persistence is planned for a future release.
- Config commands only affect `Radio` settings currently. Crypto/PSK is not
  yet exposed over serial.