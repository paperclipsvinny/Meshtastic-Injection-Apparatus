#include "Crypto.h"
#include "mbedtls/aes.h" //esp32 hardware AES
#include <string.h>

//AES context:
mbedtls_aes_context aesCtx; //creates AES "context", holds state

void Crypto::begin(const uint8_t* key){
    //AES init
    mbedtls_aes_init(&aesCtx);
    mbedtls_aes_setkey_enc(&aesCtx, key, 128);    
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

