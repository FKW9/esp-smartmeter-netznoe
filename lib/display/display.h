#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Arduino.h>
#include <WiFi.h>
#include "../sdcard/sdcard.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TFT_eSPI_ext.h>
#include "../../src/obis.h"

extern TFT_eSPI tft;  // Invoke library, pins defined in User_Setup.h
extern TFT_eSPI_ext etft;

extern obisData last_meter_data;

void setupDisplay();
void displayUpdate(bool force = false);
void displayMeterData(obisData *data);
void displayWiFiInfo();
void displaySDCardStatus();

#endif