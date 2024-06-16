#ifndef SETUP_H_
#define SETUP_H_

#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include "../display/display.h"
#include "../../config.h"

void setupWiFi();
void checkWiFiConnection();
void startConfigAP(bool force = false);

#endif