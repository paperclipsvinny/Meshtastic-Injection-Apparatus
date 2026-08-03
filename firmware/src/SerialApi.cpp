#include "SerialApi.h"
#include <ArduinoJson.h>

void SerialApi::begin(Radio* radio, WifiConfig* wifi) {
    radioRef = radio;
    wifiRef = wifi;
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
        else {
        Serial.println("{\"error\":\"unknown cmd\"}");
    }
}