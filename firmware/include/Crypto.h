#pragma once
#include <Arduino.h>
#include "mbedtls/aes.h"

class Crypto{
public:
void begin(const uint8_t* key);
int decrypt(const uint8_t* ciphertext, size_t len, uint32_t packetId, uint32_t source, uint8_t* outPlaintext);

    //wrapping key in member variable + adding getter/setter for serial/web config
void setKey(const uint8_t* newKey);
void getKey(uint8_t* outKey) const;
bool hasKey() const { return keySet; }

private:
void buildNonce(uint8_t* nonce, uint32_t packetId, uint32_t source);

    mbedtls_aes_context aesCtx;
    uint8_t currentKey[16];
    bool keySet = false;
};