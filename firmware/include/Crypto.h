#pragma once
#include <Arduino.h>

class Crypto{
public:
    static void begin(const uint8_t* key);
    static int decrypt(const uint8_t* ciphertext, size_t len, uint32_t packetId, uint32_t source, uint8_t* outPlaintext);
private:
    static void buildNonce(uint8_t* nonce, uint32_t packetId, uint32_t source);
};
