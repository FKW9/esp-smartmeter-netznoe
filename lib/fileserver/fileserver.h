#ifndef FILESERVER_H_
#define FILESERVER_H_

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD_MMC.h>

void setupFileserver();
void stopFileserver();

#endif