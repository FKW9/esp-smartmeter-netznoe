#ifndef SETUP_H_
#define SETUP_H_

#include <ArduinoOTA.h>
#include "../display/display.h"
#include "../../config.h"

uint8_t startWiFi();
void checkWiFiConnection();

#endif