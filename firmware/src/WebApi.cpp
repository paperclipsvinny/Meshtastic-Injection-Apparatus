#include "WebApi.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

static AsyncWebServer server(80);
Radio* WebApi::radioRef = nullptr;
Crypto* WebApi::cryptoRef = nullptr;

void WebApi::begin(Radio* radio, Crypto* crypto, const char* apSsid, const char* apPassword) {
    radioRef = radio;
    cryptoRef = crypto;

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

    server.begin();
}