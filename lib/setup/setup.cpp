#include "setup.h"

/**
 * @brief Connect to WiFi, init OTA
 */
uint8_t startWiFi()
{
	WiFi.disconnect();
	WiFi.setAutoReconnect(true);
	WiFi.setAutoConnect(true);
	WiFi.setHostname(HOSTNAME);
	WiFi.mode(WIFI_STA);

	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	static int cnt = 0;
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(100);

		if (++cnt > 600)
		{
			delay(1000);
			return 0;
		}
	}
    // Disable power saving on WiFi to improve responsiveness
    // (https://github.com/espressif/arduino-esp32/issues/1484)
	WiFi.setSleep(false);
	WiFi.setTxPower(WIFI_POWER_19_5dBm);

    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.begin();

    displayWiFiInfo();

    return 1;
}

/**
 * @brief Check if still connected, else reconnect.
 * + OTA handle
 */
void checkWiFiConnection()
{
    ArduinoOTA.handle();
    if (WiFi.status() != WL_CONNECTED)
    {
		if (!startWiFi())
			ESP.restart();
    }
}