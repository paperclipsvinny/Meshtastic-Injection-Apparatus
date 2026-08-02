#pragma once
#include <Arduino.h>
#include "Radio.h"

class SerialApi {
public:
    void begin(Radio* radio);
    void handle();  // call every loop() iteration

private:
    Radio* radioRef;
    String inputBuffer;
    void processCommand(const String& line);
};