#include <Arduino.h>
#include <RadioLib.h>
#include <Preferences.h> //for persistent storage
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "mbedtls/aes.h" //esp32 hardware AES

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
//Keyboard setup
USBHIDKeyboard Keyboard;

//rx interrupt flag, set by radio, cleared by loop.
volatile bool rxFlag = false;

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
            
//Duckyscript Command extraction
void executecommand(char* keyword, char* argument){
    Serial.print(keyword);
    if (argument != NULL) {
        Serial.print(argument);
    }
    Serial.println();
    
    //STRING
    if (strcmp(keyword, "STRING") == 0 && argument != NULL){
        Keyboard.print(argument);
    } 

    //STRINGLN(type text and enter)
    else if (strcmp(keyword, "STRINGLN") == 0 && argument != NULL){
        Keyboard.println(argument);
    }

    // ENTER
    else if (strcmp(keyword, "ENTER") == 0) {
        Keyboard.write(KEY_RETURN);
    }
    // TAB
    else if (strcmp(keyword, "TAB") == 0) {
        Keyboard.write(KEY_TAB);
    }
    // ESCAPE
    else if (strcmp(keyword, "ESCAPE") == 0 || strcmp(keyword, "ESC") == 0) {
        Keyboard.write(KEY_ESC);
    }
    // SPACE
    else if (strcmp(keyword, "SPACE") == 0) {
        Keyboard.write(' ');
    }
    // BACKSPACE
    else if (strcmp(keyword, "BACKSPACE") == 0) {
        Keyboard.write(KEY_BACKSPACE);
    }
    // DELETE
    else if (strcmp(keyword, "DELETE") == 0) {
        Keyboard.write(KEY_DELETE);
    }
    // UP / DOWN / LEFT / RIGHT
    else if (strcmp(keyword, "UP") == 0) {
        Keyboard.write(KEY_UP_ARROW);
    }
    else if (strcmp(keyword, "DOWN") == 0) {
        Keyboard.write(KEY_DOWN_ARROW);
    }
    else if (strcmp(keyword, "LEFT") == 0) {
        Keyboard.write(KEY_LEFT_ARROW);
    }
    else if (strcmp(keyword, "RIGHT") == 0) {
        Keyboard.write(KEY_RIGHT_ARROW);
    }
    // F1-F12
    else if (strcmp(keyword, "F1") == 0) { Keyboard.write(KEY_F1); }
    else if (strcmp(keyword, "F2") == 0) { Keyboard.write(KEY_F2); }
    else if (strcmp(keyword, "F3") == 0) { Keyboard.write(KEY_F3); }
    else if (strcmp(keyword, "F4") == 0) { Keyboard.write(KEY_F4); }
    else if (strcmp(keyword, "F5") == 0) { Keyboard.write(KEY_F5); }
    else if (strcmp(keyword, "F6") == 0) { Keyboard.write(KEY_F6); }
    else if (strcmp(keyword, "F7") == 0) { Keyboard.write(KEY_F7); }
    else if (strcmp(keyword, "F8") == 0) { Keyboard.write(KEY_F8); }
    else if (strcmp(keyword, "F9") == 0) { Keyboard.write(KEY_F9); }
    else if (strcmp(keyword, "F10") == 0) { Keyboard.write(KEY_F10); }
    else if (strcmp(keyword, "F11") == 0) { Keyboard.write(KEY_F11); }
    else if (strcmp(keyword, "F12") == 0) { Keyboard.write(KEY_F12); }
    // DELAY - pause execution
    else if (strcmp(keyword, "DELAY") == 0 && argument != NULL) {
        int ms = atoi(argument);  // Convert string to integer
        delay(ms);
    }
    // GUI / WINDOWS - Windows key combos
    else if (strcmp(keyword, "GUI") == 0 || strcmp(keyword, "WINDOWS") == 0) {
        Keyboard.press(KEY_LEFT_GUI);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.write(argument[0]);  // Press the key with GUI held
        }
        Keyboard.release(KEY_LEFT_GUI);
    }
    // CTRL - Control key combos
    else if (strcmp(keyword, "CTRL") == 0 || strcmp(keyword, "CONTROL") == 0) {
        Keyboard.press(KEY_LEFT_CTRL);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.write(argument[0]);
        }
        Keyboard.release(KEY_LEFT_CTRL);
    }
    // ALT - Alt key combos
    else if (strcmp(keyword, "ALT") == 0) {
        Keyboard.press(KEY_LEFT_ALT);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.write(argument[0]);
        }
        Keyboard.release(KEY_LEFT_ALT);
    }
    // SHIFT - Shift key combos
    else if (strcmp(keyword, "SHIFT") == 0) {
        Keyboard.press(KEY_LEFT_SHIFT);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.write(argument[0]);
        }
        Keyboard.release(KEY_LEFT_SHIFT);
    }
    // CAPSLOCK
    else if (strcmp(keyword, "CAPSLOCK") == 0) {
        Keyboard.write(KEY_CAPS_LOCK);
    }
    // PRINTSCREEN
    else if (strcmp(keyword, "PRINTSCREEN") == 0) {
        Keyboard.write(HID_KEY_PRINT_SCREEN);
    }
    // REM - comment, do nothing
    else if (strcmp(keyword, "REM") == 0) {
        // Ignore comments
    }
    // Unknown command
    else {
        Serial.print("[MIA] Unknown command: ");
        Serial.println(keyword);
    }
}

//used for multiple commands, seperated by a comma.
void executecommands(char* commandstring){
    char* cmdCopy = strdup(commandstring); //make a copy since strtok modifies the string
    char* singleCmd = strtok(cmdCopy, ",");

    while (singleCmd != NULL){
        //trim leading whitespace
        while (*singleCmd == ' ') singleCmd++;
        //trim trailing whitespace
        char* end = singleCmd + strlen(singleCmd) - 1;
        while (end > singleCmd && *end == ' '){
            *end = '\0';
            end--;
        }
        if (strlen(singleCmd) > 0){
            //find space between keyword and arg
            char* space = strchr(singleCmd, ' ');
            char* argument = NULL;    
                        
            if (space != NULL){
                *space = '\0';       //terminates keyword
                argument = space + 1;  //point to argument  
                            
                //skip whitespace after keyword
                while (*argument == ' ') argument++;
            }
            //execute the command
            executecommand(singleCmd, argument);
            delay(100); //added for reliablity
        }
        singleCmd = strtok(NULL, ",");
    }
    free(cmdCopy);
}



void setup() {
    pinMode(VEXT_ENABLE, OUTPUT);
    digitalWrite(VEXT_ENABLE, LOW);
    pinMode(LED, OUTPUT);

    Serial.begin(115200);
    delay(2000); //wait for serial monitor init

    Serial.println("===============================");
    Serial.println("  M.esh I.njection A.pparatus ");
    Serial.println("===============================");
    Serial.println("Booting...");

    //USB HID Init
    Serial.print("Initializing USB HID...");
    USB.begin();
    Keyboard.begin();
    Serial.println("OK!");

    //initialize spi for LORA
    spi.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

    //lora init
    Serial.print("Initializing LoRa...");
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
        Serial.println("OK!");
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
            Serial.println("\n----Packet Recieved---");
            Serial.print("Length: ");
            Serial.print(len);
            Serial.println(" bytes");

            Serial.print("\n RSSI: ");
            Serial.print(radio.getRSSI());
            Serial.println(" dBm");
            Serial.print("\n SNR: ");
            Serial.print(radio.getSNR());
            Serial.println(" dB");

            //print raw hex
            Serial.print("Hex: ");
            for (size_t i = 0; i < len; i++) {
                if (buffer[i] < 0x10) Serial.print("0");
                Serial.print(buffer[i], HEX);
                Serial.print(" ");
        
            }
            Serial.println();

            //header parsing functionality
            uint32_t dest = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
            uint32_t source = buffer[4] | buffer[5] << 8 | buffer[6] << 16 | buffer[7] << 24;
            uint32_t packetid = buffer[8] | buffer[9] << 8 | buffer[10] << 16 | buffer[11] << 24;
            uint8_t flags = buffer[12];
            uint8_t channel = buffer[13];
            
           Serial.printf("From: %08X To: %08X ID: %08X Flags: %02X Ch: %02X\n",
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
            
            Serial.print("Nonce: ");
            for (int i = 0; i < 16; i++){
                if (nonce[i] < 0x10) Serial.print("0");
                Serial.print(nonce[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
            
            
            
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
                Serial.print("Encrypted: ");
                for (size_t i = 0; i < payloadlen && i < 16; i++) {
                    if (buffer[16+i] < 0x10) Serial.print("0");
                    Serial.print(buffer[16+i], HEX);
                    Serial.print(" ");
                }
                Serial.println();

            //print first 16 bytes of plaintext, in hex (still protobuf encoded)    
            Serial.print("Decrypted: ");
            for (int i = 0; i < payloadlen; i++) {
                if (decrypted[i] < 0x10) Serial.print("0");
                Serial.print(decrypted[i], HEX); 
                Serial.print (" ");
            }
            Serial.println();
            
            //protobuf reading logic (only works for text message app messages)
            if (payloadlen >=2 && decrypted[0] == 0x08 && decrypted[1] == 0x01){
                Serial.println("Text message detected (header 0x08 0x01)");
                //Field 2 (payload) starts at byte 2, 12 = field 2, wire type 2 (length delimited)
                if (payloadlen >=4 && decrypted[2] == 0x12){
                    uint8_t textlen = decrypted[3];
                    Serial.print("Text length = ");
                    Serial.print(textlen);
                    Serial.print(".");
                    Serial.println();
                    Serial.print("<Text:> ");
                    //extract text into buffer
                    char textBuffer[200];
                    int len = 0;
                    for (int i = 0; i < textlen &&(4 + i) < payloadlen && i < 199; i++){
                        textBuffer[i] = decrypted[4 + i];
                        len++;
                    }
                    textBuffer[len] = '\0'; //null terminate
                    Serial.println(textBuffer);
                    
                    //check for "!mia:" prefix
                    if (strncmp(textBuffer, "!mia:", 5) == 0){
                        Serial.println("[MIA] Command Detected!");
                        //get pointer to command 
                        char* command = textBuffer + 5;
                        //handle leading whitespace
                        while (*command == ' ') command++;
                        executecommands(command);
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
            Serial.print("[DEBUG] Listening... rxFlag = ");
            Serial.println(rxFlag);
        }

         
    static unsigned long lastRSSI = 0;
    if (millis() - lastRSSI > 2000){
        lastRSSI = millis();
        Serial.print("RF RSSI: ");
        Serial.print(radio.getRSSI());
        Serial.println(" dBm");
    }
    static unsigned long lastIrq = 0;
if (millis() - lastIrq > 500) {
    lastIrq = millis();
    uint16_t irq = radio.getIrqFlags();
    if (irq != 0) {
        Serial.print("[IRQ] flags: 0x");
        Serial.println(irq, HEX);
    }
}
}


