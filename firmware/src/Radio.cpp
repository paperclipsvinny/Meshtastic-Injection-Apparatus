#include "Radio.h"
#include "Logger.h"
#include <RadioLib.h>

// Heltec V3 LoRa Pins
#define LORA_CS 8
#define LORA_DIO1 14    
#define LORA_RST 12
#define LORA_BUSY 13
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_SCK 9

//MESHTASTIC RADIO PARAMETERS 
//currently LONGFAST, US Frequency
#define LORA_FREQ 906.875
#define LORA_BAND 250.0
#define LORA_SF 11
#define LORA_CR 5
#define LORA_SYNC 0x2B
#define LORA_POWER 22
#define LORA_PREAM 16
#define LORA_TXCOVOLT 1.8 

//New Radio Instance
SPIClass spi(HSPI);
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, spi);

//rx interrupt flag, set by radio, cleared by loop.
volatile bool rxFlag = false;

//prevents collisions 
void IRAM_ATTR Radio::setRxFlag(){
    rxFlag = true;
}

bool Radio::begin() {
    //initialize spi for LORA
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

    //lora init
    Logger::raw("Initializing LoRa...");
    //following goes: freq, BW, SF, CR, Sync word, power, preamble len, txcoVoltages, use LDO
    int state = radio.begin(LORA_FREQ, LORA_BAND, LORA_SF, LORA_CR, LORA_SYNC, LORA_POWER, LORA_PREAM, LORA_TXCOVOLT, false);
    radio.setCRC(RADIOLIB_SX126X_LORA_CRC_ON); //appends a checksum to packets
    radio.setCurrentLimit(140.0); //how much current the radio can draw during tx. 
    radio.setRxBoostedGainMode(true); //boosted gain for better sensitivity
    radio.setDio2AsRfSwitch(true); //connect antenna to recieve path
    radio.setDio1Action(setRxFlag);

    if (state == RADIOLIB_ERR_NONE) {
        Logger::rawln("OK!");
        //restarts listening
        radio.startReceive();
        return true;
    } else{
        Logger::rawln("FAILED!");
        return false;
    }

}

void Radio::startReceive(){
    radio.startReceive();
} 

float Radio::getRSSI(){
    return radio.getRSSI();
}
    
float Radio::getSNR(){
    return radio.getSNR();
}

bool Radio::packetAvailable(){
    if (!rxFlag) return false;
    rxFlag = false; //clear it first
    return true;
}

int Radio::receivePacket(uint8_t* buffer, size_t bufferSize, size_t& outLen){
    outLen = radio.getPacketLength();
    if (outLen == 0 || outLen > bufferSize){
        return RADIOLIB_ERR_LORA_HEADER_DAMAGED;
    }
    return radio.readData(buffer, outLen);
}

uint16_t Radio::getIrqFlags(){
    return radio.getIrqFlags();
}