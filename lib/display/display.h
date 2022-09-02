#include <Arduino.h>
#include <WiFi.h>
#include "../sdcard/sdcard.h"
#include <SPI.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;  // Invoke library, pins defined in User_Setup.h

void setupDisplay();
void displayMeterData(struct obisData *data);
void displayWiFiInfo();
void displayRSSI();
void displaySDCardStatus();