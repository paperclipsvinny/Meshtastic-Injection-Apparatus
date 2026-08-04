#pragma once
#include <Arduino.h>
#include "mbedtls/aes.h"
#include <Preferences.h>

class Crypto{
public:
void begin(const uint8_t* key, size_t keyLen);
int decrypt(const uint8_t* ciphertext, size_t len, uint32_t packetId, uint32_t source, uint8_t* outPlaintext);

//wrapping key in member variable + adding getter/setter for serial/web config
void setKey(const uint8_t* newKey, size_t keyLen);
void getKey(uint8_t* outKey, size_t* outKeyLen) const;
bool hasKey() const { return keySet; }

private:
void buildNonce(uint8_t* nonce, uint32_t packetId, uint32_t source);
    Preferences prefs;
    mbedtls_aes_context aesCtx;
    uint8_t currentKey[32]; //sized for up to 256 bit PSKs
    size_t currentKeyLen = 16; //tracks which size is currently in use
    bool keySet = false;
};