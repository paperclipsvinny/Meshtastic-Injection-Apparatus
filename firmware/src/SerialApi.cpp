#include "SerialApi.h"
#include <ArduinoJson.h>
#include "mbedtls/base64.h"
#include "Crypto.h"

void SerialApi::begin(Radio* radio, WifiConfig* wifi, WebApi* webApi, Crypto* crypto) {
    radioRef = radio;
    wifiRef = wifi;
    webApiRef = webApi;
    cryptoRef = crypto;
}

void SerialApi::handle() {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n') {
            inputBuffer.trim();
            if (inputBuffer.length() > 0) {
                processCommand(inputBuffer);
            }
            inputBuffer = "";
        } else if (c != '\r') {
            inputBuffer += c;
        }
    }
}

void SerialApi::processCommand(const String& line) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, line);

    if (err) {
        Serial.println("{\"error\":\"invalid json\"}");
        return;
    }

    const char* cmd = doc["cmd"] | "";

    if (strcmp(cmd, "get_config") == 0) {
        JsonDocument resp;
        resp["frequency"] = radioRef->getFrequency();
        resp["bandwidth"] = radioRef->getBandwidth();
        resp["spreadFactor"] = radioRef->getSpreadFactor();
        resp["codingRate"] = radioRef->getCodingRate();
        resp["power"] = radioRef->getPower();

        serializeJson(resp, Serial);
        Serial.println();
    }
    else if (strcmp(cmd, "set_config") == 0) {
        bool ok = true;

        if (doc["spreadFactor"].is<int>()) {
            ok &= radioRef->setSpreadFactor(doc["spreadFactor"]);
        }
        if (doc["power"].is<int>()) {
            ok &= radioRef->setPower(doc["power"]);
        }

        Serial.println(ok ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}");
    }

    else if (strcmp(cmd, "get_wifi") == 0) {
    JsonDocument resp;
    resp["enabled"] = wifiRef->apEnabled;
    resp["ssid"] = wifiRef->ssid;
    // password intentionally omitted from get_wifi to protect PSK
    serializeJson(resp, Serial);
    Serial.println();
    }
    else if (strcmp(cmd, "set_wifi") == 0) {
        if (doc["enabled"].is<bool>())
            wifiRef->setEnabled(doc["enabled"]);
        if (doc["ssid"].is<const char*>())
            wifiRef->setSsid(doc["ssid"].as<String>());
        if (doc["password"].is<const char*>())
            wifiRef->setPassword(doc["password"].as<String>());
    
        Serial.println("{\"status\":\"ok\"}");
    }
    else if (strcmp(cmd, "get_psk") == 0) {
        uint8_t key[32];
        size_t keyLen;
        cryptoRef->getKey(key, &keyLen);
        
        unsigned char b64[64];
        size_t b64Len;
        mbedtls_base64_encode(b64, sizeof(b64), &b64Len, key, keyLen);
        
        JsonDocument resp;
        resp["psk"] = String((char*)b64).substring(0, b64Len);
        serializeJson(resp, Serial);
        Serial.println();
    }
    else if (strcmp(cmd, "set_psk") == 0) {
        if (doc["psk"].is<const char*>()) {
            String psk = doc["psk"].as<String>();
            unsigned char decoded[32];
            size_t decodedLen;
            int rc = mbedtls_base64_decode(decoded, sizeof(decoded), &decodedLen,
                                            (const unsigned char*)psk.c_str(), psk.length());
            if (rc != 0 || (decodedLen != 16 && decodedLen != 32)) {
            Serial.println("{\"error\":\"psk must decode to 16 or 32 bytes\"}");
        } else {
            cryptoRef->setKey(decoded, decodedLen);
            Serial.println("{\"status\":\"ok\"}");
        }

        } else {
            Serial.println("{\"error\":\"missing psk field\"}");
        }
    }


        else {
        Serial.println("{\"error\":\"unknown cmd\"}");
    }
}