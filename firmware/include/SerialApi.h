#pragma once
#include <Arduino.h>
#include "Radio.h"
#include "WifiConfig.h"

class SerialApi {
public:
    void begin(Radio* radio, WifiConfig* wifi);
    void handle();  // call every loop() iteration

private:
    Radio* radioRef;
    WifiConfig* wifiRef;
    String inputBuffer;
    void processCommand(const String& line);
};