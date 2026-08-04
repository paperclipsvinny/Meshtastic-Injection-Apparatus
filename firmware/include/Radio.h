#pragma once
#include <Arduino.h>
#include <RadioLib.h>
#include <Preferences.h>

// Heltec V3 LoRa Pins
#define LORA_CS 8
#define LORA_DIO1 14    
#define LORA_RST 12
#define LORA_BUSY 13
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_SCK 9

//MESHTASTIC RADIO PARAMETERS 
//Defaults, currently LONGFAST, US Frequency
#define LORA_FREQ 906.875
#define LORA_BAND 250.0
#define LORA_SF 11
#define LORA_CR 5
#define LORA_SYNC 0x2B
#define LORA_POWER 22
#define LORA_PREAM 16
#define LORA_TXCOVOLT 1.8 

class Radio{
public:
    bool begin();
    void startReceive();
    float getRSSI();
    float getSNR();
    uint16_t getIrqFlags();
    bool packetAvailable();
    int receivePacket(uint8_t* buffer, size_t bufferSize, size_t& outLen);

    //wrapping defines in member variables to be able to change them without reflashing
    //Note: defines are hardcoded in as defaults, can be changed over serial/web 
    float getFrequency() const { return freq; }
    float getBandwidth() const { return bandwidth; }
    uint8_t getSpreadFactor() const { return sf; }
    uint8_t getCodingRate() const { return cr; }
    int8_t getPower() const { return power; }  

    bool setSpreadFactor(uint8_t newSf);
    bool setPower(int8_t newPower);


private:
    static void IRAM_ATTR setRxFlag(); //must stay static: used as a raw interrupt callback
    Preferences prefs;
    float freq = LORA_FREQ;
    float bandwidth = LORA_BAND;
    uint8_t sf = LORA_SF;
    uint8_t cr = LORA_CR;
    int8_t power = LORA_POWER;
};