#include "graphite.h"
#include "../display/display.h"
#include "../../config.h"

// Graphite Client
WiFiClient graphiteClient;

/**
 * @brief Connect to WiFi, start MDN Service and init OTA
 */
void setupWiFi()
{

  static uint8_t cnt = 0;

  WiFi.setHostname(HOSTNAME);

  // Disable power saving on WiFi to improve responsiveness
  // (https://github.com/espressif/arduino-esp32/issues/1484)
  WiFi.setSleep(false);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_KEY);

  etft.printf("Connecting to: %s ", WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED)
  {
    etft.print(".");

    delay(250);
    cnt++;

    if (cnt > 40)
    {
      etft.println();
      etft.setTextColor(TFT_RED);
      etft.println("Error connecting to WiFi! Restarting...");
      Serial.println("Error connecting to WiFi");
      delay(2000);
      ESP.restart();
    }
  }
  etft.println();

  if (!MDNS.begin(MDNS_HOSTNAME))
  {
    etft.setTextColor(TFT_RED);
    etft.println("Starting mDNS failed! Restarting...");
    Serial.println("Error starting mDNS");
    delay(2000);
    ESP.restart();
  }

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_AUTH);
  ArduinoOTA.begin();

  // etft.fillScreen(TFT_BLACK);
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
    WiFi.disconnect();
    yield();
    setupWiFi();
  }
}

/**
 * @brief Submit a metric to graphite
 *
 * @param unix_timestamp can be -1 to use current time of graphite host
 * @param metrics
 * @param value
 */
void submitToGraphite(time_t unix_timestamp, String metrics, float value)
{
  if (!graphiteClient.connect(GRAPHITE_HOST, GRAPHITE_PORT))
  {
    Serial.print("Sending data to graphite failed!");
    delay(500);
    return;
  }

  String payload = metrics + " " + value + " " + unix_timestamp + "\n";

  graphiteClient.print(payload);
  graphiteClient.stop();
}
