// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "WebApi_limit.h"
#include "ArduinoJson.h"
#include "AsyncJson.h"
#include "Hoymiles.h"

void WebApiLimitClass::init(AsyncWebServer* server)
{
    using std::placeholders::_1;

    _server = server;

    _server->on("/api/limit/status", HTTP_GET, std::bind(&WebApiLimitClass::onLimitStatus, this, _1));
    _server->on("/api/limit/config", HTTP_POST, std::bind(&WebApiLimitClass::onLimitPost, this, _1));
}

void WebApiLimitClass::loop()
{
}

void WebApiLimitClass::onLimitStatus(AsyncWebServerRequest* request)
{
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();

    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);

        // Inverter Serial is read as HEX
        char buffer[sizeof(uint64_t) * 8 + 1];
        snprintf(buffer, sizeof(buffer), "%0lx%08lx",
            ((uint32_t)((inv->serial() >> 32) & 0xFFFFFFFF)),
            ((uint32_t)(inv->serial() & 0xFFFFFFFF)));

        root[buffer]["limit"] = inv->SystemConfigPara()->getLimitPercent();
    }

    response->setLength();
    request->send(response);
}

void WebApiLimitClass::onLimitPost(AsyncWebServerRequest* request)
{
    AsyncJsonResponse* response = new AsyncJsonResponse();
    JsonObject retMsg = response->getRoot();
    retMsg[F("type")] = F("warning");

    if (!request->hasParam("data", true)) {
        retMsg[F("message")] = F("No values found!");
        response->setLength();
        request->send(response);
        return;
    }

    String json = request->getParam("data", true)->value();

    if (json.length() > 1024) {
        retMsg[F("message")] = F("Data too large!");
        response->setLength();
        request->send(response);
        return;
    }

    DynamicJsonDocument root(1024);
    DeserializationError error = deserializeJson(root, json);

    if (error) {
        retMsg[F("message")] = F("Failed to parse data!");
        response->setLength();
        request->send(response);
        return;
    }

    if (!(root.containsKey("serial")
            && root.containsKey("limit_value")
            && root.containsKey("limit_type"))) {
        retMsg[F("message")] = F("Values are missing!");
        response->setLength();
        request->send(response);
        return;
    }

    if (root[F("serial")].as<uint64_t>() == 0) {
        retMsg[F("message")] = F("Serial must be a number > 0!");
        response->setLength();
        request->send(response);
        return;
    }

    if (root[F("limit_value")].as<uint32_t>() == 0 || root[F("limit_value")].as<uint32_t>() > 1500) {
        retMsg[F("message")] = F("Limit must between 1 and 1500!");
        response->setLength();
        request->send(response);
        return;
    }

    uint64_t serial = strtoll(root[F("serial")].as<String>().c_str(), NULL, 16);
    uint64_t limit = root[F("serial")].as<uint64_t>();

    auto inv = Hoymiles.getInverterBySerial(serial);

    inv->sendActivePowerControlRequest(Hoymiles.getRadio(), limit, PowerLimitControlType::RelativNonPersistent);

    retMsg[F("type")] = F("success");
    retMsg[F("message")] = F("Settings saved!");

    response->setLength();
    request->send(response);
}