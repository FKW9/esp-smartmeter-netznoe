// WiFi Hostname
#define HOSTNAME "FakeSmartmeter"

// Comment out next line to disable SD Card logging
// Also disables the Fileserver
#define SD_CARD_LOGGING

// Comment out next line to disable Graphite logging
#define USE_GRAPHITE

#ifdef USE_GRAPHITE
    #define GRAPHITE_HOST "192.168.8.42"    // IP and Port of your Graphite Host
    #define GRAPHITE_PORT 2003              // Port must be standard metric line receiver
#endif
