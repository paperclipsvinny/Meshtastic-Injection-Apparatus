#pragma once
#include <Arduino.h>
#include <RadioLib.h>

class Radio{
public:
    static bool begin();
    static void startReceive();
    static float getRSSI();
    static float getSNR();
    static uint16_t getIrqFlags();
    static bool packetAvailable();
    static int receivePacket(uint8_t* buffer, size_t bufferSize, size_t& outLen);

private:
    static void IRAM_ATTR setRxFlag();
};