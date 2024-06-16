// WiFi Hostname
#ifdef TEST_SETUP
    #define HOSTNAME "FakeSmartmeter"
#else
    #define HOSTNAME "Smartmeter"
#endif

// If you get a new Smartmeter, the overall energy counter gets reset.
// Input your latest measurement here to add it to the new values.
// Unit = Wh
#define ACTIVE_ENERGY_PLUS_OFFSET 9520525

#define WIFI_SSID "************"
#define WIFI_PASSWORD "*****************"

#define GRAPHITE_HOST "192.168.8.42"    // IP and Port of your Graphite Host
#define GRAPHITE_PORT 2003              // Port must be standard metric line receiver

