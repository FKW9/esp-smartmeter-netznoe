#ifndef HTE501_H_
#define HTE501_H_

#include <Wire.h>

#define HTE_ADDRESS 0x40

void setupSensor(void);
void startPeriodicMeasurement(void);
uint8_t getTemperatureHumidity(float &temperature, float &humidity);
uint8_t changePeriodicMeasurementTime(uint32_t millisec);
uint8_t newMeasurementReady(uint8_t &ready);
uint8_t fetchPeriodicTemperatureHumidity(float &temperature, float &humidity);

#endif