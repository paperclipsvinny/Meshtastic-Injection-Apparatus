#include "Crypto.h"
#include <string.h>
#include <Preferences.h>

void Crypto::begin(const uint8_t* key, size_t keyLen){
    mbedtls_aes_init(&aesCtx); //AES init
    prefs.begin("crypto", false);
    size_t storedLen = prefs.getBytesLength("key");
    if (storedLen == 16 || storedLen == 32) {
        uint8_t stored[32];
        prefs.getBytes("key", stored, storedLen);
        setKey(stored, storedLen);
    } else {
        setKey(key, keyLen);   // no valid stored key, use the default passed in
    }
}

void Crypto::setKey(const uint8_t* newKey, size_t keyLen){
    if (keyLen != 16 && keyLen != 32) return;
    memcpy(currentKey, newKey, keyLen);
    currentKeyLen = keyLen;
    mbedtls_aes_setkey_enc(&aesCtx, newKey, keyLen * 8);
    keySet = true;
    prefs.putBytes("key", newKey, keyLen);
}

void Crypto::getKey(uint8_t* outKey, size_t* outKeyLen) const {
    memcpy(outKey, currentKey, currentKeyLen);
    *outKeyLen = currentKeyLen;
}
//nonce generation:
void Crypto::buildNonce(uint8_t* nonce, uint32_t packetId, uint32_t source){
    memset(nonce, 0, 16); //zero all 16 bytes first
        //bytes 0-3: packetid (little-endian)
    nonce[0] = packetId & 0xFF;
    nonce[1] = (packetId >> 8) & 0xFF;
    nonce[2] = (packetId >> 16) & 0xFF;
    nonce[3] = (packetId >> 24) & 0xFF;
        //bytes 4-7: zeros padding (packetid is only 32 bit)
        //bytes 8-11: source (little-endian) 
    nonce[8] = source & 0xFF;
    nonce[9] = (source >> 8) & 0xFF;
    nonce[10] = (source >> 16) & 0xFF;
    nonce[11] = (source >> 24) & 0xFF;
        //bytes 12-15 zero'd as well (matters when payload size > 16 bytes)
}

int Crypto::decrypt(const uint8_t* ciphertext, size_t len, uint32_t packetId, uint32_t source, uint8_t* outPlaintext){
uint8_t nonce[16];
buildNonce(nonce, packetId, source);

size_t nonceOffset = 0;
uint8_t streamBlock[16];
return mbedtls_aes_crypt_ctr(&aesCtx, len, &nonceOffset, nonce, streamBlock, ciphertext, outPlaintext);
}