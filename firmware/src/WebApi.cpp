#include "WebApi.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "Logger.h" 
#include "HID.h"


static AsyncWebServer server(80);
Radio* WebApi::radioRef = nullptr;
Crypto* WebApi::cryptoRef = nullptr;
WifiConfig* WebApi::wifiConfigRef = nullptr;

void WebApi::begin(Radio* radio, Crypto* crypto, const char* apSsid, const char* apPassword) {
    radioRef = radio;
    cryptoRef = crypto;
    wifiConfigRef = wifi;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid, apPassword);

    server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        doc["frequency"] = radioRef->getFrequency();
        doc["bandwidth"] = radioRef->getBandwidth();
        doc["spreadFactor"] = radioRef->getSpreadFactor();
        doc["codingRate"] = radioRef->getCodingRate();
        doc["power"] = radioRef->getPower();

        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    server.on("/api/config", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);

            if (err) {
                request->send(400, "application/json", "{\"error\":\"invalid json\"}");
                return;
            }

            bool ok = true;
            if (doc["spreadFactor"].is<int>())
                ok &= radioRef->setSpreadFactor(doc["spreadFactor"]);
            if (doc["power"].is<int>())
                ok &= radioRef->setPower(doc["power"]);

            request->send(ok ? 200 : 400, "application/json",
                          ok ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}");
        }
    );
    if (!LittleFS.begin(true)) {
    Logger::rawln("[WebApi] LittleFS mount failed");
    } else {
        Logger::rawln("[WebApi] LittleFS mounted");
        server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    }
    // fire a command directly via USB HID — no mesh involved
server.on("/api/inject/fire", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (err) {
            request->send(400, "application/json", "{\"error\":\"invalid json\"}");
            return;
        }
        const char* cmd = doc["command"] | "";
        if (strlen(cmd) == 0) {
            request->send(400, "application/json", "{\"error\":\"missing command\"}");
            return;
        }
        // strip !mia: prefix if present
        if (strncmp(cmd, "!mia: ", 6) == 0) cmd += 6;
        HID::executeCommands(cmd);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
);
    // transmit a command over LoRa mesh
server.on("/api/inject/transmit", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (err) {
            request->send(400, "application/json", "{\"error\":\"invalid json\"}");
            return;
        }
        const char* cmd = doc["command"] | "";
        if (strlen(cmd) == 0) {
            request->send(400, "application/json", "{\"error\":\"missing command\"}");
            return;
        }
        // build the full !mia: prefixed message
        String msg = "!mia: ";
        msg += cmd;
        bool ok = radioRef->transmit((const uint8_t*)msg.c_str(), msg.length());
        request->send(ok ? 200 : 500, "application/json",
                      ok ? "{\"status\":\"ok\"}" : "{\"error\":\"transmit failed\"}");
    }
);
    server.on("/api/inject/payloads", HTTP_GET, [](AsyncWebServerRequest* request) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    File root = LittleFS.open("/payloads");
    if (root && root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                JsonObject obj = arr.add<JsonObject>();
                obj["name"] = String(file.name());
                obj["size"] = file.size();
            }
            file = root.openNextFile();
        }
    }
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
});

    server.on("/api/inject/upload", HTTP_POST,
    [](AsyncWebServerRequest* request) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    },
    [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        // create /payloads/ directory if needed
        if (!LittleFS.exists("/payloads")) {
            LittleFS.mkdir("/payloads");
        }
        String path = "/payloads/" + filename;
        if (index == 0) {
            // first chunk — open/create the file
            request->_tempFile = LittleFS.open(path, "w");
        }
        if (request->_tempFile) {
            request->_tempFile.write(data, len);
        }
        if (final && request->_tempFile) {
            request->_tempFile.close();
        }
    }
);

    server.on("/api/inject/payload", HTTP_DELETE,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        deserializeJson(doc, data, len);
        const char* name = doc["name"] | "";
        if (strlen(name) == 0) {
            request->send(400, "application/json", "{\"error\":\"missing name\"}");
            return;
        }
        String path = "/payloads/";
        path += name;
        if (LittleFS.remove(path)) {
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            request->send(404, "application/json", "{\"error\":\"not found\"}");
        }
    }
);
    // reboot
server.on("/api/reboot", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
        delay(100);
        ESP.restart();
    }
);

// board info
server.on("/api/board", HTTP_GET, [](AsyncWebServerRequest* request) {
    JsonDocument doc;
    doc["version"] = "0.9.0";
    doc["mac"] = WiFi.macAddress();
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
});
    // GET PSK
server.on("/api/psk", HTTP_GET, [](AsyncWebServerRequest* request) {
    uint8_t key[32];
    size_t keyLen;
    cryptoRef->getKey(key, &keyLen);
    unsigned char b64[64];
    size_t b64Len;
    mbedtls_base64_encode(b64, sizeof(b64), &b64Len, key, keyLen);
    JsonDocument doc;
    doc["psk"] = String((char*)b64).substring(0, b64Len);
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
});

// POST PSK
server.on("/api/psk", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (err) { request->send(400, "application/json", "{\"error\":\"invalid json\"}"); return; }
        if (!doc["psk"].is<const char*>()) { request->send(400, "application/json", "{\"error\":\"missing psk\"}"); return; }
        String psk = doc["psk"].as<String>();
        unsigned char decoded[32];
        size_t decodedLen;
        int rc = mbedtls_base64_decode(decoded, sizeof(decoded), &decodedLen,
                                        (const unsigned char*)psk.c_str(), psk.length());
        if (rc != 0 || (decodedLen != 16 && decodedLen != 32)) {
            request->send(400, "application/json", "{\"error\":\"psk must decode to 16 or 32 bytes\"}");
            return;
        }
        cryptoRef->setKey(decoded, decodedLen);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
);

// GET WiFi
server.on("/api/wifi", HTTP_GET, [](AsyncWebServerRequest* request) {
    JsonDocument doc;
    doc["enabled"] = wifiConfigRef->apEnabled;
    doc["ssid"] = wifiConfigRef->ssid;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
});

// POST WiFi  
server.on("/api/wifi", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (err) { request->send(400, "application/json", "{\"error\":\"invalid json\"}"); return; }
        if (doc["enabled"].is<bool>()) wifiConfigRef->setEnabled(doc["enabled"]);
        if (doc["ssid"].is<const char*>()) wifiConfigRef->setSsid(doc["ssid"].as<String>());
        if (doc["password"].is<const char*>()) wifiConfigRef->setPassword(doc["password"].as<String>());
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    }
);

// POST Defaults
server.on("/api/defaults", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        deserializeJson(doc, data, len);
        const char* action = doc["action"] | "";
        if (strcmp(action, "save") == 0) {
            // mirror save_defaults from SerialApi
            Preferences defaults;
            defaults.begin("defaults", false);
            defaults.putFloat("freq", radioRef->getFrequency());
            defaults.putFloat("bw", radioRef->getBandwidth());
            defaults.putUChar("sf", radioRef->getSpreadFactor());
            defaults.putUChar("cr", radioRef->getCodingRate());
            defaults.putChar("power", radioRef->getPower());
            uint8_t key[32]; size_t keyLen;
            cryptoRef->getKey(key, &keyLen);
            defaults.putBytes("key", key, keyLen);
            defaults.putBool("ap", wifiConfigRef->apEnabled);
            defaults.putString("ssid", wifiConfigRef->ssid);
            defaults.putString("pass", wifiConfigRef->password);
            defaults.end();
            request->send(200, "application/json", "{\"status\":\"ok\",\"msg\":\"saved\"}");
        } else if (strcmp(action, "reset") == 0) {
            Preferences radio; radio.begin("radio", false); radio.clear(); radio.end();
            Preferences crypto; crypto.begin("crypto", false); crypto.clear(); crypto.end();
            Preferences wifi; wifi.begin("wifi", false); wifi.clear(); wifi.end();
            request->send(200, "application/json", "{\"status\":\"ok\",\"msg\":\"reset\"}");
        } else {
            request->send(400, "application/json", "{\"error\":\"unknown action\"}");
        }
    }
);

    server.begin();
}