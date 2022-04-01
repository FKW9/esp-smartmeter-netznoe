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

#ifndef ESP8266
  WiFi.setHostname(HOSTNAME);
#endif

  // Disable power saving on WiFi to improve responsiveness
  // (https://github.com/espressif/arduino-esp32/issues/1484)
  WiFi.setSleep(false);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_KEY);

  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Connecting to " + String(WIFI_SSID));
  display.display();

  String waiter = ".";
  while (WiFi.status() != WL_CONNECTED)
  {
    display.drawStringMaxWidth(0, 20, 128, waiter);
    display.display();

    delay(100);
    cnt++;
    waiter += ".";

    if (cnt > 100)
    {
      display.clear();
      display.drawStringMaxWidth(0, 0, 128, "Error connecting to WiFi! Restarting...");
      display.display();
      Serial.println("Error connecting to WiFi");
      delay(1000);
      ESP.restart();
    }
  }

  if (!MDNS.begin(MDNS_HOSTNAME))
  {
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "Starting mDNS failed! Restarting...");
    display.display();
    Serial.println("Error starting mDNS");
    delay(1000);
    ESP.restart();
  }

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_AUTH);
  ArduinoOTA.begin();

  display.clear();
  display.drawString(0, 0, "WiFi connected!");
  display.drawString(0, 10, "OTA ready: " + WiFi.localIP().toString());
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.display();
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
    delay(500);
    return;
  }

  String payload = metrics + " " + value + " " + unix_timestamp + "\n";

  graphiteClient.print(payload);
  graphiteClient.stop();

  char formatted_value[32];

  // format float for displaying
  if (metrics == GRAPHITE_ACTIVE_ENERGY_PLUS)
  {
    sprintf(formatted_value, "%.0fkWh", value / 1000);
    display.drawString(125, 52, formatted_value);
  }
  else if (metrics == GRAPHITE_POWER_FACTOR)
  {
    sprintf(formatted_value, "%1.3f", value);
    display.drawString(125, 42, formatted_value);
  }
  else if (metrics == GRAPHITE_ACTIVE_POWER_PLUS)
  {
    sprintf(formatted_value, "%.0fW", value);
    display.drawString(125, 32, formatted_value);
  }
  else if (metrics == GRAPHITE_CURRENT_L1)
  {
    if (value < 1)
      sprintf(formatted_value, "%.2f", value);
    if (value >= 1)
      sprintf(formatted_value, "%.1f", value);
    if (value >= 10)
      sprintf(formatted_value, "%.0f", value);
    display.drawString(33, 51, formatted_value);
  }
  else if (metrics == GRAPHITE_CURRENT_L2)
  {
    if (value < 1)
      sprintf(formatted_value, "%.2f", value);
    if (value >= 1)
      sprintf(formatted_value, "%.1f", value);
    if (value >= 10)
      sprintf(formatted_value, "%.0f", value);
    display.drawString(55, 51, formatted_value);
  }
  else if (metrics == GRAPHITE_CURRENT_L3)
  {
    if (value < 1)
      sprintf(formatted_value, "%.2f", value);
    if (value >= 1)
      sprintf(formatted_value, "%.1f", value);
    if (value >= 10)
      sprintf(formatted_value, "%.0f", value);
    display.drawString(77, 51, formatted_value);
  }
  else if (metrics == GRAPHITE_VOLTAGE_L1)
  {
    sprintf(formatted_value, "%.0f", value);
    display.drawString(33, 42, formatted_value);
  }
  else if (metrics == GRAPHITE_VOLTAGE_L2)
  {
    sprintf(formatted_value, "%.0f", value);
    display.drawString(55, 42, formatted_value);
  }
  else if (metrics == GRAPHITE_VOLTAGE_L3)
  {
    sprintf(formatted_value, "%.0f", value);
    display.drawString(77, 42, formatted_value);
  }
  display.display();
}
