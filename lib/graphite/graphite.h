#ifndef GRAPHITE_H_
#define GRAPHITE_H_

#include "../../config.h"
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
#define GRAPHITE_RSSI 					"smartmeter.data.rssi"
#define GRAPHITE_T 					    "smartmeter.data.pcb_t"
#define GRAPHITE_RH 					"smartmeter.data.pcb_rh"
#define GRAPHITE_SEND_TIME              "smartmeter.data.send_time"     // how long it takes to send all packets to the host
#define GRAPHITE_PROC_TIME              "smartmeter.data.process_time"  // how long it took to get and decode all data

void submitToGraphite(String metrics, float value, time_t unix_timestamp = -1);

#endif