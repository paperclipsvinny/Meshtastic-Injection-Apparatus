#include <Arduino.h>
#include <RadioLib.h>
#include <Preferences.h> //for persistent storage
#include "mbedtls/aes.h" //esp32 hardware AES
#include "Logger.h"
#include "HID.h"

//PIN DEFINITIONS
#define LED 35
#define VEXT_ENABLE 36 //Voltage External (peripheral power)
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
#define LOra_PREAM 16

//New Radio Instance
SPIClass spi(HSPI);
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, spi);

//rx interrupt flag, set by radio, cleared by loop.
volatile bool rxFlag = false;

//initial declaration for retransmitted packet variables 
uint32_t lastExecutedPacketId = 0;
uint32_t lastExecutedSource = 0;

//prevents collisions 
void IRAM_ATTR setRxFlag(){
    rxFlag = true;
}

//AES context:
mbedtls_aes_context aesCtx; //creates AES "context", holds state

//AES Key (default = 0x01, aka AQ== in B64)
const uint8_t defaultKey[16] = {
    0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
    0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01,
};
            
void setup() {
    pinMode(VEXT_ENABLE, OUTPUT);
    digitalWrite(VEXT_ENABLE, LOW);
    pinMode(LED, OUTPUT);

    Serial.begin(115200);
    delay(2000); //wait for serial monitor init

    Logger::rawln("===============================");
    Logger::rawln("  M.esh I.njection A.pparatus ");
    Logger::rawln("===============================");
    Logger::info("Booting...");

    //USB HID Init
    Logger::raw("Initializing USB HID...");
    HID::begin  ();
    Logger::rawln("OK!");

    //initialize spi for LORA
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

    //lora init
    Logger::raw("Initializing LoRa...");
    //following goes: freq, BW, SF, CR, Sync word, power, preamble len, txcoVoltages, use LDO
    int state = radio.begin(906.875, 250.0, 11, 5, 0x2B, 22, 16, 1.8, false);
    radio.setCRC(RADIOLIB_SX126X_LORA_CRC_ON); //appends a checksum to packets
    radio.setCurrentLimit(140.0); //how much current the radio can draw during tx. 
    radio.setRxBoostedGainMode(true); //boosted gain for better sensitivity
    radio.setDio2AsRfSwitch(true); //connect antenna to recieve path
    radio.setDio1Action(setRxFlag);



    //radio settings as variables for later
    //int spread = 9; 
    /*
        //configure radio params
        radio.setSpreadingFactor(11); //how the radio encodes bits into chrips
        radio.setBandwidth(250.0); //measured in khz
        radio.setCodingRate(5); //from 5-8
        radio.setSyncWord(0x14); //public is 0x12, meshtastic is 0x14
        Serial.println("Radio Configured: SF9, 250khz, CR5, SyncWord 0x14");
    
    
    */
    if (state == RADIOLIB_ERR_NONE) {
        Logger::rawln("OK!");
    //restarts listening
    radio.startReceive();
    }
    
    //AES init
    mbedtls_aes_init(&aesCtx);
    mbedtls_aes_setkey_enc(&aesCtx, defaultKey, 128);
}


void loop(){
   /*  //check for serial commands
    if (Serial.available()){
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
    if (cmd == "test"){
        delay(3000);
        Serial.println("Injecting: Hello from MIA!");
        Keyboard.print("Hello from MIA!");
        Keyboard.write('\n');
        Serial.println("Done.");
    } 
    */

    //check for LORA Packets
    if (rxFlag) {
        rxFlag = false; //clear it first
        
        //buffer for recieved bytes
        uint8_t buffer[255];
        size_t len = radio.getPacketLength();
        int state = radio.readData(buffer, len); //read packet from radio

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

            //nonce generation:
            uint8_t nonce[16];
            memset(nonce, 0, 16); //zero all 16 bytes first
            //bytes 0-3: packetid (little-endian)
            nonce[0] = packetid & 0xFF;
            nonce[1] = (packetid >> 8) & 0xFF;
            nonce[2] = (packetid >> 16) & 0xFF;
            nonce[3] = (packetid >> 24) & 0xFF;
            //bytes 4-7: zeros padding (packetid is only 32 bit)
            //bytes 8-11: source (little-endian) 
            nonce[8] = source & 0xFF;
            nonce[9] = (source >> 8) & 0xFF;
            nonce[10] = (source >> 16) & 0xFF;
            nonce[11] = (source >> 24) & 0xFF;
            //bytes 12-15 zero'd as well (matters when payload size > 16 bytes)
            
            Logger::raw("Nonce: ");
            for (int i = 0; i < 16; i++){
                Logger::hexByte(nonce[i]);
            }
            Logger::rawln();
            
            
            
            //decrypt payload, added bounds checking
            size_t payloadlen = len - 16;
            if (payloadlen > 0 && payloadlen < 200){
                uint8_t decrypted[200];
                size_t nonceOffset = 0;
                uint8_t streamBlock[16];
                
                //mbedtls increments counter, copy before use & use a fresh nonce
                uint8_t nonceCopy[16];
                memcpy(nonceCopy, nonce, 16);

                mbedtls_aes_crypt_ctr(&aesCtx, payloadlen, &nonceOffset, nonce, streamBlock, &buffer[16], decrypted);
                
                //print first 16 bytes of ciphertext, more data = more information
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

            //fast blink for visual feedback when packets recieved
            for (int i = 0; i < 3; i++){
                digitalWrite(LED, HIGH);
                delay(50);
                digitalWrite(LED, LOW);
                delay(50);
            }

        }
        radio.startReceive();

    }
        static unsigned long lastStatus = 0;
        if (millis() - lastStatus > 5000){
            lastStatus = millis();
            Logger::raw("[DEBUG] Listening... rxFlag = ");
            Logger::raw(rxFlag);
            Logger::rawln();
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