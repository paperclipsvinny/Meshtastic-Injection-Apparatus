# Meshtastic Resources

The MIA builds on top of the Meshtastic protocol and radio parameters (LONGFAST
preset, channel PSKs, packet structure). For anything not specific to MIA
itself, refer to the official docs:

- **Getting started / device setup**: https://meshtastic.org/docs/getting-started/
- **Full documentation**: https://meshtastic.org/docs/
- **Protocol / packet structure**: https://meshtastic.org/docs/overview/mesh-algo/
- **Channels & PSKs**: https://meshtastic.org/docs/configuration/radio/channels/
- **Python API** (useful for testing/tooling against MIA): https://python.meshtastic.org/

MIA intentionally does not modify or extend the Meshtastic protocol — it implements a
compatible receive/decrypt path so it can interoperate with a standard mesh.  