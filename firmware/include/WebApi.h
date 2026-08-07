#pragma once
#include <Arduino.h>
#include "Radio.h"
#include "Crypto.h"
#include "WifiConfig.h" 

class WebApi {
public:
    void begin(Radio* radio, Crypto* crypto, WifiConfig* wifi, const char* apSsid, const char* apPassword);
    
private:
    static Radio* radioRef;
    static Crypto* cryptoRef; 
    static WifiConfig* wifiConfigRef;
};