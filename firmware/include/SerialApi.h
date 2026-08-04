#pragma once
#include <Arduino.h>
#include "Radio.h"
#include "WifiConfig.h"
#include "Crypto.h"
#include "WebApi.h" 


class SerialApi {
public:
    void begin(Radio* radio, WifiConfig* wifi, WebApi* webApi, Crypto* crypto);
    void handle();  // call every loop() iteration

private:
    Radio* radioRef;
    WifiConfig* wifiRef;
    Crypto* cryptoRef;
    String inputBuffer;
    WebApi* webApiRef;
    void processCommand(const String& line);
};