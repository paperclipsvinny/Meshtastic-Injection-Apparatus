#pragma once
#include <Arduino.h>

struct PacketHeader {
    uint32_t dest;
    uint32_t source;
    uint32_t packetId;
    uint8_t flags;
    uint8_t channel;
};

class PacketParser {
public:
    // parses the 14-byte Meshtastic header from the front of a raw packet
    static PacketHeader parseHeader(const uint8_t* buffer);

    // extracts text from a decrypted protobuf payload, if it's a text message.
    // returns true if a text message was found and written into outText.
    static bool extractTextMessage(const uint8_t* decrypted, size_t payloadLen, char* outText, size_t outTextSize, uint8_t* outTextLen);
};