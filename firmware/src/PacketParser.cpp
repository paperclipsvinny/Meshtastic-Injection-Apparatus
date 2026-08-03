#include "PacketParser.h"

//header parsing functionality
PacketHeader PacketParser::parseHeader(const uint8_t* buffer) {
    PacketHeader header;
    header.dest = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
    header.source = buffer[4] | buffer[5] << 8 | buffer[6] << 16 | buffer[7] << 24;
    header.packetId = buffer[8] | buffer[9] << 8 | buffer[10] << 16 | buffer[11] << 24;
    header.flags = buffer[12];
    header.channel = buffer[13];
    return header;
}

bool PacketParser::extractTextMessage(const uint8_t* decrypted, size_t payloadLen, char* outText, size_t outTextSize, uint8_t* outTextLen) {
    //protobuf reading logic (only works for text message app messages)
    if (payloadLen < 2 || decrypted[0] != 0x08 || decrypted[1] != 0x01) {
        return false;
    }
    //Field 2 (payload) starts at byte 2, 12 = field 2, wire type 2 (length delimited)
    if (payloadLen < 4 || decrypted[2] != 0x12) {
        return false;
    }

    //extract text into buffer
    uint8_t textLen = decrypted[3];
    int len = 0;
    for (int i = 0; i < textLen && (4 + i) < (int)payloadLen && i < (int)outTextSize - 1; i++) {
        outText[i] = decrypted[4 + i];
        len++;
    }
    outText[len] = '\0'; //null terminate  
    *outTextLen = textLen;
    return true;
}