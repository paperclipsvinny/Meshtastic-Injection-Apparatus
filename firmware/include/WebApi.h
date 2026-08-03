#pragma once
#include <Arduino.h>
#include "Radio.h"

class WebApi {
public:
    void begin(Radio* radio, const char* apSsid, const char* apPassword);

private:
    static Radio* radioRef;
};