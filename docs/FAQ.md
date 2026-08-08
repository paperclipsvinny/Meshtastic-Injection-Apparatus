# FAQ

**Do I need two MIA devices?**

No. You need one MIA device (the implant, plugged into the target) and any standard Meshtastic node to send commands. The operator side can be the official Meshtastic app on a phone, a stock Heltec, T-Beam, really anything that can send text messages on the same channel.

**Can I use this for "red team engagements"?**

MIA is a research tool. Use it only on systems you own or have explicit written permission to test. I'm not repsonsible for your actions! 
 

**Why does it say `Radio init failed`?**

The most common cause is a hardware issue — loose LoRa module, bad solder joint, or a pin mismatch between the firmware and the board. A good starting point is to check the [Hardware Overview](hardware/overview.md) and verify your pin definitions match the actual board wiring, as well as check physical connections with a multimeter.

**Why is the web dashboard a blank page?**
Well, assuming the MIA dashboard doesn't throw an error but rather just shows as blank, the issue is likely that you ran `pio run -t upload` but forgot `pio run -t uploadfs`. Both are required — firmware and filesystem are separate. Run `uploadfs` and try again. 

**Can MIA receive and execute commands at the same time?**

Yes. MIA is always in receive mode unless it's actively transmitting (TX then immediately returns to RX). The USB HID execution happens inline when a matching packet arrives.

**What happens if the same HID command arrives twice (commands relayed by other nodes)?**

MIA deduplicates by packet ID and source address. Meshtastic retransmits unacknowledged packets automatically — MIA detects and skips these duplicates so each command executes exactly once.

**Can I change the channel frequency?**

Both MIA and the sending node must use matching radio parameters. 
See [Meshtastic channel configuration](https://meshtastic.org/docs/configuration/radio/channels/) 
for how to set matching parameters on your sending node, and 
[Radio Settings](../firmware/serial-api.md#radio-configuration) for how to 
configure MIA's side.

**Does this work outside the US?**

The default frequency (906.875 MHz) is the US LONGFAST channel. For EU operation you'd need to change the frequency to 868.0 MHz and match Meshtastic's EU LONGFAST parameters. Check local regulations for LoRa frequency usage.

**Is the PSK stored securely?**

The PSK is stored in ESP32 NVS (flash). It's not transmitted over the air by the device, and it's not in any file that gets committed to git. Access requires either physical USB access or knowing the WiFi AP password. See [Security Considerations](security/considerations.md).

**What DuckyScript commands are supported?**

STRING, STRINGLN, ENTER, DELAY, GUI, CTRL, ALT, SHIFT, TAB, ESCAPE, BACKSPACE, and arrow keys. See [Payloads](firmware/payloads.md) for the full reference.