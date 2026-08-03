#include <Arduino.h>
#include <Preferences.h> //for persistent storage
#include "Logger.h"
#include "HID.h"
#include "Radio.h"
#include "Crypto.h"
#include "StatusLED.h"
#include "SerialApi.h" 
#include "WebApi.h"
#include "WifiConfig.h"


//PIN DEFINITIONS
#define LED 35
#define VEXT_ENABLE 36 //Voltage External (peripheral power)

//initial declaration for retransmitted packet variables 
uint32_t lastExecutedPacketId = 0;
uint32_t lastExecutedSource = 0;

//AES Key (default = 0x01, aka AQ== in B64)
const uint8_t defaultKey[16] = {
    0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
    0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01,
};
        
Radio radio; //allocates memory for Radio object
SerialApi serialApi; //same thing for SerialAPI
WebApi webApi; //same, for WebAPI
WifiConfig wifiConfig; //you guessed it


void setup() {
    pinMode(VEXT_ENABLE, OUTPUT);
    digitalWrite(VEXT_ENABLE, LOW);
    StatusLED::begin(LED);

    Serial.begin(115200);
    delay(2000); //wait for serial monitor init

    Logger::rawln("===============================");
    Logger::rawln("  M.esh I.njection A.pparatus ");
    Logger::rawln("===============================");
    Logger::info("Booting...");

    if (!radio.begin()){
        Logger::rawln("Radio init failed - halting.");
        while (true) delay(1000);
    }
    
    //serial API start
    serialApi.begin(&radio, &wifiConfig);
    Logger::rawln("Serial API loaded.");

    if (wifiConfig.apEnabled) {
        webApi.begin(&radio, wifiConfig.ssid.c_str(), wifiConfig.password.c_str());
        Logger::rawln("Web API ready.");
    } else {
        Logger::rawln("Web API disabled (send set_wifi over serial to enable).");
    }



    //USB HID Init
    Logger::raw("Initializing USB HID...");
    HID::begin();
    Logger::rawln("OK!");

    Crypto::begin(defaultKey);
}

void loop(){
    serialApi.handle(); //processes serial commands each iteration
    if(radio.packetAvailable()){
        //buffer for recieved bytes
        uint8_t buffer[255];
        size_t len;
        int state = radio.receivePacket(buffer, sizeof(buffer), len); //read packet from radio
        
        if (state == RADIOLIB_ERR_NONE) {
            Logger::rawln("\n----Packet Recieved---");
            Logger::raw("Length: ");
            Logger::raw((int)len);
            Logger::rawln(" bytes");

            Logger::raw("\n RSSI: ");
            Logger::raw(radio.getRSSI());
            Logger::rawln(" dBm");
            Logger::raw("\n SNR: ");
            Logger::raw(radio.getSNR());
            Logger::rawln(" dB");

            //print raw hex
            Logger::raw("Hex: ");
            for (size_t i = 0; i < len; i++){
                Logger::hexByte(buffer[i]);
            }
            Logger::rawln();

            //header parsing functionality
            uint32_t dest = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
            uint32_t source = buffer[4] | buffer[5] << 8 | buffer[6] << 16 | buffer[7] << 24;
            uint32_t packetid = buffer[8] | buffer[9] << 8 | buffer[10] << 16 | buffer[11] << 24;
            uint8_t flags = buffer[12];
            uint8_t channel = buffer[13];
            
            Logger::infof("From: %08X To: %08X ID: %08X Flags: %02X Ch: %02X",
                        source, dest, packetid, flags, channel);           
            
            //decrypt payload, added bounds checking
            size_t payloadlen = len - 16;
            if (payloadlen > 0 && payloadlen < 200){
                uint8_t decrypted[200];
                Crypto::decrypt(&buffer[16], payloadlen, packetid, source, decrypted);
                Logger::raw("Encrypted: ");
                for (size_t i = 0; i < payloadlen && i < 16; i++) {
                    Logger::hexByte(buffer[16+i]);
                }
                Logger::rawln();

            //print first 16 bytes of plaintext, in hex (still protobuf encoded)    
            Logger::raw("Decrypted: ");
            for (int i = 0; i < payloadlen; i++) {
                Logger::hexByte(decrypted[i]);
            }
            Logger::rawln();
            
            //protobuf reading logic (only works for text message app messages)
            if (payloadlen >=2 && decrypted[0] == 0x08 && decrypted[1] == 0x01){
                Logger::rawln("Text message detected (header 0x08 0x01)");
                //Field 2 (payload) starts at byte 2, 12 = field 2, wire type 2 (length delimited)
                if (payloadlen >=4 && decrypted[2] == 0x12){
                    uint8_t textlen = decrypted[3];
                    Logger::raw("Text length = ");
                    Logger::raw((int)textlen);
                    Logger::rawln(".");
                    Logger::raw("<Text:> ");
                    //extract text into buffer
                    char textBuffer[200];
                    int len = 0;
                    for (int i = 0; i < textlen &&(4 + i) < payloadlen && i < 199; i++){
                        textBuffer[i] = decrypted[4 + i];
                        len++;
                    }
                    textBuffer[len] = '\0'; //null terminate
                    Logger::rawln(textBuffer);
                    
                    //check for "!mia:" prefix
                    if (strncmp(textBuffer, "!mia:", 5) == 0){
                        if (packetid == lastExecutedPacketId && source == lastExecutedSource){
                            Logger::info("[MIA] Duplicate/retransmitted packet - skipping");
                        } else {
                            lastExecutedPacketId = packetid;
                            lastExecutedSource = source;
                            Logger::info("[MIA] Command Detected!");
                            //get pointer to command 
                            char* command = textBuffer + 5;
                            //handle leading whitespace
                            while (*command == ' ') command++;
                            HID::executeCommands(command);
                        }
                    }   
                }
            }      
        }
        StatusLED::blinkReceived();
        }
        radio.startReceive();

    }
         
    static unsigned long lastRSSI = 0;
    if (millis() - lastRSSI > 2000){
        lastRSSI = millis();
        Logger::raw("RF RSSI: ");
        Logger::raw(radio.getRSSI());
        Logger::rawln(" dBm");
    }
    static unsigned long lastIrq = 0;
    if (millis() - lastIrq > 500) {
        lastIrq = millis();
        uint16_t irq = radio.getIrqFlags();
        if (irq != 0) {
            Logger::raw("[IRQ] flags: 0x");
            char irqBuf[8];
            sniprintf(irqBuf, sizeof(irqBuf), "%X", irq);
            Logger::raw(irqBuf);
            Logger::rawln();
        }
    }
}