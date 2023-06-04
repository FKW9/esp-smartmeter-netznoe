#ifndef GRAPHITE_H_
#define GRAPHITE_H_

#include "../../config.h"

#ifdef USE_GRAPHITE

#include <WiFi.h>

#ifdef TEST_SETUP
    #define GRAPHITE_ACTIVE_POWER_PLUS 		"test.smartmeter.momentanleistung.plus"
    #define GRAPHITE_ACTIVE_POWER_MINUS 	"test.smartmeter.momentanleistung.minus"
    #define GRAPHITE_ACTIVE_ENERGY_PLUS 	"test.smartmeter.wirkenergie.plus"
    #define GRAPHITE_ACTIVE_ENERGY_MINUS 	"test.smartmeter.wirkenergie.minus"
    #define GRAPHITE_VOLTAGE_L1 			"test.smartmeter.spannung.L1"
    #define GRAPHITE_VOLTAGE_L2 			"test.smartmeter.spannung.L2"
    #define GRAPHITE_VOLTAGE_L3 			"test.smartmeter.spannung.L3"
    #define GRAPHITE_CURRENT_L1 			"test.smartmeter.strom.L1"
    #define GRAPHITE_CURRENT_L2 			"test.smartmeter.strom.L2"
    #define GRAPHITE_CURRENT_L3 			"test.smartmeter.strom.L3"
    #define GRAPHITE_POWER_FACTOR 			"test.smartmeter.cosphi"
    #define GRAPHITE_RSSI 					"test.smartmeter.data.rssi"
    #define GRAPHITE_T 					    "test.smartmeter.data.pcb_t"
    #define GRAPHITE_RH 					"test.smartmeter.data.pcb_rh"
#else
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
#endif

void submitToGraphite(String metrics, float value, time_t unix_timestamp = -1);

#endif
#endif