#include "Radio.h"
#include "Logger.h"
#include <RadioLib.h>

//New Radio Instance
SPIClass spi(HSPI);
SX1262 objectSX1262 = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, spi);

//rx interrupt flag, set by radio, cleared by loop.
volatile bool rxFlag = false;

//prevents collisions 
void IRAM_ATTR Radio::setRxFlag(){
    rxFlag = true;
}

bool Radio::begin() {
    // load persisted values, falling back to compiled-in defaults
    prefs.begin("radio", false);   // "radio" = NVS namespace name
    freq = prefs.getFloat("freq", LORA_FREQ);
    bandwidth = prefs.getFloat("bw", LORA_BAND);
    sf = prefs.getUChar("sf", LORA_SF);
    cr = prefs.getUChar("cr", LORA_CR);
    power = prefs.getChar("power", LORA_POWER);
    
    //initialize spi for LORA
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

    //lora init
    Logger::raw("Initializing LoRa...");
    //following goes: (LORA_FREQ, LORA_BAND, LORA_SF, LORA_CR, Sync word, LORA_POWER, preamble len, LORA_TXCOVOLT, use LDO);
    #ifdef MIA_PCB
    int state = objectSX1262.begin(freq, bandwidth, sf, cr, LORA_SYNC, power, LORA_PREAM);
    // no TCXO voltage, no useLDO — let RadioLib use its defaults
#else
    int state = objectSX1262.begin(freq, bandwidth, sf, cr, LORA_SYNC, power, LORA_PREAM, LORA_TXCOVOLT, false);
#endif

    Logger::raw("Radio begin state: ");
    Logger::raw(state);
    Logger::rawln();

    
    
    objectSX1262.setCRC(RADIOLIB_SX126X_LORA_CRC_ON); //appends a checksum to packets
    objectSX1262.setCurrentLimit(140.0); //how much current the radio can draw during tx. 
    objectSX1262.setRxBoostedGainMode(true); //boosted gain for better sensitivity
    objectSX1262.setDio2AsRfSwitch(true); //connect antenna to recieve path
    objectSX1262.setDio1Action(setRxFlag);

    if (state == RADIOLIB_ERR_NONE) {
        Logger::rawln("OK!");
        //restarts listening
        objectSX1262.startReceive();
        return true;
    } else{
        Logger::rawln("FAILED!");
        return false;
    }

}

void Radio::startReceive(){
    objectSX1262.startReceive();
} 

float Radio::getRSSI(){
    return objectSX1262.getRSSI();
}
    
float Radio::getSNR(){
    return objectSX1262.getSNR();
}

bool Radio::packetAvailable(){
    if (!rxFlag) return false;
    rxFlag = false; //clear it first
    return true;
}

int Radio::receivePacket(uint8_t* buffer, size_t bufferSize, size_t& outLen){
    outLen = objectSX1262.getPacketLength();
    if (outLen == 0 || outLen > bufferSize){
        return RADIOLIB_ERR_LORA_HEADER_DAMAGED;
    }
    return objectSX1262.readData(buffer, outLen);
}

uint16_t Radio::getIrqFlags(){
    return objectSX1262.getIrqFlags();
}

//Serial/web API functions
bool Radio::setSpreadFactor(uint8_t newSf) {
    int state = objectSX1262.setSpreadingFactor(newSf);
    if (state == RADIOLIB_ERR_NONE) {
        sf = newSf;
        prefs.putUChar("sf", sf);
        return true;
    }
    return false;
}

bool Radio::setPower(int8_t newPower) {
    int state = objectSX1262.setOutputPower(newPower);
    if (state == RADIOLIB_ERR_NONE) {
        power = newPower;
        prefs.putChar("power", power);
        return true;
    }
    return false;
}

bool Radio::transmit(const uint8_t* data, size_t len) {
    objectSX1262.clearIrqFlags();
    int state = objectSX1262.transmit(data, len);
    objectSX1262.startReceive(); // return to RX after TX
    return state == RADIOLIB_ERR_NONE;
}