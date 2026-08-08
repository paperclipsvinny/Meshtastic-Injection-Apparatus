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

    if (doc["spreadFactor"].is<int>())
        ok &= radioRef->setSpreadFactor(doc["spreadFactor"]);
    if (doc["power"].is<int>())
        ok &= radioRef->setPower(doc["power"]);
    if (doc["frequency"].is<float>())
        ok &= radioRef->setFrequency(doc["frequency"]);
    if (doc["bandwidth"].is<float>())
        ok &= radioRef->setBandwidth(doc["bandwidth"]);
    if (doc["codingRate"].is<int>())
        ok &= radioRef->setCodingRate(doc["codingRate"]);

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
    else if (strcmp(cmd, "save_defaults") == 0) {
    // save current active values into the "defaults" namespace
    Preferences defaults;
    defaults.begin("defaults", false);

    // radio
    defaults.putFloat("freq", radioRef->getFrequency());
    defaults.putFloat("bw", radioRef->getBandwidth());
    defaults.putUChar("sf", radioRef->getSpreadFactor());
    defaults.putUChar("cr", radioRef->getCodingRate());
    defaults.putChar("power", radioRef->getPower());

    // crypto
    uint8_t key[32];
    size_t keyLen;
    cryptoRef->getKey(key, &keyLen);
    defaults.putBytes("key", key, keyLen);
    defaults.putUChar("keylen", keyLen);

    // wifi
    defaults.putBool("ap", wifiRef->apEnabled);
    defaults.putString("ssid", wifiRef->ssid);
    defaults.putString("pass", wifiRef->password);

    defaults.end();
    Serial.println("{\"status\":\"ok\",\"msg\":\"current config saved as defaults\"}");
}
else if (strcmp(cmd, "reset_defaults") == 0) {
    Preferences defaults;
    defaults.begin("defaults", true); // read-only
    bool hasSaved = defaults.getBytesLength("key") > 0;
    defaults.end();

    if (hasSaved) {
        // load user-saved defaults back into active namespaces
        Preferences d;
        d.begin("defaults", true);

        Preferences radio;
        radio.begin("radio", false);
        radio.putFloat("freq", d.getFloat("freq", LORA_FREQ));
        radio.putFloat("bw", d.getFloat("bw", LORA_BAND));
        radio.putUChar("sf", d.getUChar("sf", LORA_SF));
        radio.putUChar("cr", d.getUChar("cr", LORA_CR));
        radio.putChar("power", d.getChar("power", LORA_POWER));
        radio.end();

        Preferences crypto;
        crypto.begin("crypto", false);
        uint8_t key[32];
        size_t keyLen = d.getBytes("key", key, 32);
        crypto.putBytes("key", key, keyLen);
        crypto.end();

        Preferences wifi;
        wifi.begin("wifi", false);
        wifi.putBool("enabled", d.getBool("ap", false));
        wifi.putString("ssid", d.getString("ssid", "Mesh Injection Apparatus"));
        wifi.putString("password", d.getString("pass", "Mesh-Inject-7f3K9pQ2"));
        wifi.end();

        d.end();
        Serial.println("{\"status\":\"ok\",\"msg\":\"defaults restored, reboot to apply\"}");
    } else {
        // no user defaults saved — wipe to compiled-in values
        Preferences radio; radio.begin("radio", false); radio.clear(); radio.end();
        Preferences crypto; crypto.begin("crypto", false); crypto.clear(); crypto.end();
        Preferences wifi; wifi.begin("wifi", false); wifi.clear(); wifi.end();
        Serial.println("{\"status\":\"ok\",\"msg\":\"factory reset, reboot to apply\"}");
    }
}
else {
    Serial.println("{\"error\":\"unknown cmd\"}");
}
}