#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TFT_eSPI_ext.h>
#include "../sdcard/sdcard.h"
#include "../../src/obis.h"

extern TFT_eSPI tft; // Invoke library, pins defined in User_Setup.h
extern TFT_eSPI_ext etft;

extern meterData last_meter_data;
extern bool config_update;

void displaySetup();
void displayUpdate(bool force = false);
void displayMeterData(meterData *data);
void displayWiFiInfo();
void displaySDCardStatus();
void displayInactiveTimer();
void displayResetTextSettings();

#endif