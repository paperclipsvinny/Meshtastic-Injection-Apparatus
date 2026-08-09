# Payloads

MIA executes commands using DuckyScript syntax — the same language used by the USB Rubber Ducky and many other HID injection tools.

## Sending a command

Any message on the shared Meshtastic channel prefixed with `!mia:` is treated as a command:

```
!mia: STRINGLN hello world
```

Multiple commands are separated by commas:

```
!mia: GUI r, DELAY 500, STRINGLN notepad, ENTER
```

## DuckyScript reference

| Command | Description | Example |
|---|---|---|
| `STRING` | Type text without pressing Enter | `STRING hello` |
| `STRINGLN` | Type text and press Enter | `STRINGLN hello` |
| `ENTER` | Press Enter | `ENTER` |
| `DELAY` | Wait in milliseconds | `DELAY 500` |
| `GUI` | Windows/Super key + key | `GUI r` |
| `CTRL` | Control + key | `CTRL c` |
| `ALT` | Alt + key | `ALT F4` |
| `SHIFT` | Shift + key | `SHIFT TAB` |
| `TAB` | Tab key | `TAB` |
| `ESCAPE` | Escape key | `ESCAPE` |
| `BACKSPACE` | Backspace | `BACKSPACE` |
| `UP`, `DOWN`, `LEFT`, `RIGHT` | Arrow keys | `DOWN` |

## Example payloads

**Open a terminal (Windows):**
```
!mia: GUI r, DELAY 300, STRINGLN cmd, DELAY 300
```

**Open a terminal (Linux):**
```
!mia: CTRL ALT t
```

**Type a string:**
```
!mia: STRINGLN the quick brown fox
```

**Lock screen (Windows):**
```
!mia: GUI l
```

**Close window:**
```
!mia: ALT F4
```

## Saving payloads

Payloads can be saved in the web dashboard under **Inject → Saved Payloads**. Saved payloads are stored in browser localStorage and can be fired with one tap.

Payload files (`.txt`) can also be uploaded to the device via **Inject → Upload** — these are stored on the device's LittleFS filesystem and survive reboots.

## Timing

Some payloads need delays between commands to allow applications time to open. Use `DELAY` generously — a 300–500ms delay after opening a window is usually enough.

If commands are executing too fast, add more delays:

```
!mia: GUI r, DELAY 500, STRINGLN notepad, DELAY 300, ENTER, DELAY 500, STRINGLN injected
```