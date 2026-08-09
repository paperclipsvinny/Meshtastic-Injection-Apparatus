# Mesh Injection Apparatus (M.I.A.) 
A tool that combines HID injection with LoRa (Meshtastic) mesh networking for a red team implant that doesn't rely on WiFi or internet for C2. 
![Board both sides](/docs/images/MIA_Board.png)

## DISCLAIMER: 
**This is a covert keystroke injection tool, designed for Red Teams, Penetration Testers, and authorized testing only. Do NOT use this tool on any systems which you have not been given express, explicit permission. I, the author, am not responsible for any damages which may incur from using this tool.** 

## Where is everything?
For Firmware Architecture notes, check out [ARCHITECTURE.md](./ARCHITECTURE.md). 
Hardware, Schematics and PCB manufacturing files can be found under [`/Hardware`](./Hardware) . 

For technical Documentation, head over to [DOCUMENTATION.md](docs/DOCUMENTATION.md), also available at [TheMIA.dev](https://themia.dev)

As debuted at the [DEF CON 34 Main stage](https://defcon.org/html/defcon-34/dc-34-speakers.html#content_66655)

## LICENSE: 
This project is released under the **MIT license**. 
 Anyone is permitted to modify, distribute, and use anything from this project for both private and commercial use. 
### Conditions: 
You must provide a copy of the original copyright notice and include the full MIT license text, found in LICENSE.txt. 
### Limitations: 
The software is NOT under any warranty, and the author will not be held responsible for any damages or issues arising from this software. The original Author’s name and original logos cannot be used for marketing or promotion. 

## Credits & Inspiration
- **[Meshtastic](https://meshtastic.org)** — the mesh protocol MIA builds on
- **[RadioLib](https://github.com/jgromes/RadioLib)** — LoRa radio abstraction
- **[Loki](https://github.com/RocketGod-git/Meshtastic-Loki)** — Meshtastic fork that added HID 
  support, the direct inspiration for MIA's development. 
- **[EvilCrowRF](https://github.com/joelsernamoreno/EvilCrowRF-V2)** — web dashboard 
  design inspiration.   