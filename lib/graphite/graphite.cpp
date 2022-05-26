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

  tft.print("Connecting to ");
  tft.println(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED)
  {
    tft.print(".");

    delay(100);
    cnt++;

    if (cnt > 100)
    {
      tft.println();
      tft.println("Error connecting to WiFi! Restarting...");
      Serial.println("Error connecting to WiFi");
      delay(1000);
      ESP.restart();
    }
  }
  tft.println();

  if (!MDNS.begin(MDNS_HOSTNAME))
  {
    tft.println();
    tft.println("Starting mDNS failed! Restarting...");
    Serial.println("Error starting mDNS");
    delay(1000);
    ESP.restart();
  }

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_AUTH);
  ArduinoOTA.begin();

  tft.println("WiFi connected!");
  tft.printf("Signal strength: %ddBm\r\n", WiFi.RSSI());
  tft.println("OTA ready: " + WiFi.localIP().toString());
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
 * @param unix_timestamp
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

  #ifdef DEBUG
    String payload = metrics + " " + value + " -1\n";
  #else
    String payload = metrics + " " + value + " " + unix_timestamp + "\n";
  #endif

  graphiteClient.print(payload);
  graphiteClient.stop();

  // char formatted_value[32];

  // // format float for displaying
  // if (metrics == GRAPHITE_ACTIVE_ENERGY_PLUS)
  // {
  //   sprintf(formatted_value, "%.0fkWh", value / 1000);
  //   display.drawString(125, 52, formatted_value);
  // }
  // else if (metrics == GRAPHITE_POWER_FACTOR)
  // {
  //   sprintf(formatted_value, "%1.3f", value);
  //   display.drawString(125, 42, formatted_value);
  // }
  // else if (metrics == GRAPHITE_ACTIVE_POWER_PLUS)
  // {
  //   sprintf(formatted_value, "%.0fW", value);
  //   display.drawString(125, 32, formatted_value);
  // }
  // else if (metrics == GRAPHITE_CURRENT_L1)
  // {
  //   if (value < 1)
  //     sprintf(formatted_value, "%.2f", value);
  //   if (value >= 1)
  //     sprintf(formatted_value, "%.1f", value);
  //   if (value >= 10)
  //     sprintf(formatted_value, "%.0f", value);
  //   display.drawString(33, 51, formatted_value);
  // }
  // else if (metrics == GRAPHITE_CURRENT_L2)
  // {
  //   if (value < 1)
  //     sprintf(formatted_value, "%.2f", value);
  //   if (value >= 1)
  //     sprintf(formatted_value, "%.1f", value);
  //   if (value >= 10)
  //     sprintf(formatted_value, "%.0f", value);
  //   display.drawString(55, 51, formatted_value);
  // }
  // else if (metrics == GRAPHITE_CURRENT_L3)
  // {
  //   if (value < 1)
  //     sprintf(formatted_value, "%.2f", value);
  //   if (value >= 1)
  //     sprintf(formatted_value, "%.1f", value);
  //   if (value >= 10)
  //     sprintf(formatted_value, "%.0f", value);
  //   display.drawString(77, 51, formatted_value);
  // }
  // else if (metrics == GRAPHITE_VOLTAGE_L1)
  // {
  //   sprintf(formatted_value, "%.0f", value);
  //   display.drawString(33, 42, formatted_value);
  // }
  // else if (metrics == GRAPHITE_VOLTAGE_L2)
  // {
  //   sprintf(formatted_value, "%.0f", value);
  //   display.drawString(55, 42, formatted_value);
  // }
  // else if (metrics == GRAPHITE_VOLTAGE_L3)
  // {
  //   sprintf(formatted_value, "%.0f", value);
  //   display.drawString(77, 42, formatted_value);
  // }
  // display.display();
}
