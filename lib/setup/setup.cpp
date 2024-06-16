#include "setup.h"

void startConfigAP(bool force)
{
    WiFiManager wm;
    wm.setHostname(HOSTNAME);
    wm.setClass("invert");
    wm.setShowStaticFields(true);
    wm.setShowInfoErase(true);

    wm.setConnectTimeout(180);
    wm.setConnectRetries(255);
    // wm.setShowDnsFields(true);

    if (wm.getWiFiIsSaved() && !force)
    {
        etft.println("Verbinde mit: " + wm.getWiFiSSID());
    }
    else
    {
        etft.print("Bitte mit WiFi \"");
        etft.setTextColor(TFT_GREEN);
        etft.print(HOSTNAME);
        etft.setTextColor(TFT_WHITE);
        etft.printf("\"\r\nverbinden, um Einstellungen zu \r\naendern!\r\nAdresse: 192.168.4.1\r\n");
    }

    if (force)
    {
        etft.print("Bitte mit WiFi \"");
        Serial.println("Starting config portal");
        wm.setConfigPortalTimeout(120);
        if (!wm.startConfigPortal(HOSTNAME))
        {
            etft.setTextColor(TFT_RED);
            etft.println("Error connecting or timeout! Restarting...");
            Serial.println("Failed to connect or hit timeout");
            delay(3000);
            ESP.restart();
        }
    }
    else if (!wm.autoConnect(HOSTNAME))
    {
        etft.println();
        etft.setTextColor(TFT_RED);
        etft.println("Error connecting to WiFi! Restarting...");
        Serial.println("Error connecting to WiFi");
        delay(2000);
        ESP.restart();
    }
}

/**
 * @brief Connect to WiFi, start MDN Service, start WifiManager, init OTA
 */
void setupWiFi()
{
    WiFi.mode(WIFI_STA);

    startConfigAP(false);

    if (!MDNS.begin(HOSTNAME))
    {
        etft.setTextColor(TFT_RED);
        etft.println("Starting mDNS failed! Restarting...");
        Serial.println("Error starting mDNS");
        delay(2000);
        ESP.restart();
    }

    // Disable power saving on WiFi to improve responsiveness
    // (https://github.com/espressif/arduino-esp32/issues/1484)
	WiFi.setSleep(false);
	WiFi.setTxPower(WIFI_POWER_19_5dBm);

    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.begin();

    displayWiFiInfo();
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
        displayResetTextSettings();
        etft.setTextColor(TFT_RED);
        etft.println("Connection lost!");
        etft.setTextColor(TFT_WHITE);
        WiFi.disconnect();
        yield();
        setupWiFi();
    }
}