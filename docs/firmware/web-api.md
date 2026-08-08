# Web API Reference

MIA exposes an HTTP REST API served over its own WiFi AP. All endpoints are available at `http://192.168.4.1` when connected to the MIA AP.

> **Important:** POST endpoints require the header `Content-Type: application/json`. Omitting this header causes ESPAsyncWebServer to skip the request body, resulting in a silent empty response.

---

## Radio configuration

### `GET /api/config`

Returns current radio parameters.

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

### `POST /api/config`

Updates radio parameters. Only include fields you want to change.

**Request:**
```json
{"spreadFactor": 9, "power": 17}
```

**Response:**
```json
{"status": "ok"}
```

**Example:**
```bash
curl -X POST http://192.168.4.1/api/config \
  -H "Content-Type: application/json" \
  -d '{"spreadFactor": 9}'
```

---

## PSK (channel key)

### `GET /api/psk`

Returns the current AES key as base64.

**Response:**
```json
{"psk": "1PG7OiApB1nwvP+rz05pAQ=="}
```

---

### `POST /api/psk`

Sets a new AES channel key.

**Request:**
```json
{"psk": "uUAKMhSSaaZpRNM0ytp6qw=="}
```

Must decode to exactly 16 bytes (AES-128) or 32 bytes (AES-256).

---

## WiFi / AP settings

### `GET /api/wifi`

Returns current AP configuration. Password is omitted.

**Response:**
```json
{"enabled": true, "ssid": "Mesh Injection Apparatus"}
```

---

### `POST /api/wifi`

Updates AP settings. Include any combination of fields.

**Request:**
```json
{"enabled": true, "ssid": "MyMIA", "password": "newpassword"}
```

SSID and password take effect after reboot.

---

## Inject

### `POST /api/inject/fire`

Executes a DuckyScript command locally via USB HID. The MIA device must be plugged into a target machine.

**Request:**
```json
{"command": "STRINGLN hello world"}
```

The `!mia:` prefix is stripped automatically if present.

**Response:**
```json
{"status": "ok"}
```

**Example:**
```bash
curl -X POST http://192.168.4.1/api/inject/fire \
  -H "Content-Type: application/json" \
  -d '{"command": "GUI r, DELAY 500, STRINGLN notepad, ENTER"}'
```

---

### `POST /api/inject/transmit`

Transmits a `!mia:` prefixed command over LoRa to other MIA devices on the same channel.

**Request:**
```json
{"command": "STRINGLN hello from MIA"}
```

**Response:**
```json
{"status": "ok"}
```

---

### `GET /api/inject/payloads`

Lists payload files stored on the device filesystem.

**Response:**
```json
[
  {"name": "open_terminal.txt", "size": 42},
  {"name": "reverse_shell.txt", "size": 128}
]
```

---

### `POST /api/inject/upload`

Uploads a DuckyScript payload file to the device. Uses multipart form upload.

**Example:**
```bash
curl -X POST http://192.168.4.1/api/inject/upload \
  -F "file=@payload.txt"
```

---

### `DELETE /api/inject/payload`

Deletes a stored payload by name.

**Request:**
```json
{"name": "open_terminal.txt"}
```

---

## Board info

### `GET /api/board`

Returns device information.

**Response:**
```json
{
  "version": "0.9.0",
  "mac": "3C:0F:02:ED:BF:8C",
  "uptime": 123456,
  "freeHeap": 245760
}
```

`uptime` is in milliseconds since last boot.

---

## Defaults

### `POST /api/defaults`

Saves or resets device configuration.

**Save current config as defaults:**
```json
{"action": "save"}
```

**Reset to saved defaults (or factory values):**
```json
{"action": "reset"}
```

---

## Reboot

### `POST /api/reboot`

Reboots the device. The connection will drop immediately after the response.

**Request:** empty body or `{}`

**Response:**
```json
{"status": "ok"}
```

---

## Static files

The web dashboard (`index.html`, `logo.png`) is served from LittleFS at the root path. Any file placed in `firmware/data/` and uploaded via `pio run -t uploadfs` will be accessible at `http://192.168.4.1/<filename>`.