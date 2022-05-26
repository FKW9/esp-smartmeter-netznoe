#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#define GRAPHITE_ACTIVE_POWER_PLUS 		"smartmeter.momentanleistung.plus"
#define GRAPHITE_ACTIVE_POWER_MINUS 	"smartmeter.momentanleistung.minus"
#define GRAPHITE_ACTIVE_ENERGY_PLUS 	"smartmeter.wirkenergie.plus"
#define GRAPHITE_ACTIVE_ENERGY_MINUS 	"smartmeter.wirkenergie.minus"
#define GRAPHITE_VOLTAGE_L1 			"smartmeter.spannung.L1"
#define GRAPHITE_VOLTAGE_L2 			"smartmeter.spannung.L2"
#define GRAPHITE_VOLTAGE_L3 			"smartmeter.spannung.L3"
#define GRAPHITE_CURRENT_L1 			"smartmeter.strom.L1"
#define GRAPHITE_CURRENT_L2 			"smartmeter.strom.L2"
#define GRAPHITE_CURRENT_L3 			"smartmeter.strom.L3"
#define GRAPHITE_POWER_FACTOR 			"smartmeter.cosphi"
#define GRAPHITE_RSSI 					"test.data.rssi"

void setupWiFi();
void checkWiFiConnection();
void submitToGraphite(time_t unix_timestamp, String metrics, float value);