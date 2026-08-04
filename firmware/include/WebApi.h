#pragma once
#include <Arduino.h>
#include "Radio.h"
#include "Crypto.h"

class WebApi {
public:
    void begin(Radio* radio, Crypto* crypto, const char* apSsid, const char* apPassword);

private:
    static Radio* radioRef;
    static Crypto* cryptoRef; 
};