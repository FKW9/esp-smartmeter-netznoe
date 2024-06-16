#include "display.h"
#include "screens.h"

#define HEADER "SMARTMETER V2.0"

void Screen1()
{
	etft.fillScreen(0xFFFF);

	// TextBox 2
	etft.drawRect(0, 0, 160, 20, 0x0861);
	etft.fillRect(1, 1, 158, 18, 0x39C7);
	etft.setTextColor(0xFFFF);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(15, 4);
	etft.print(F(HEADER));

	// TextBox 2
	etft.drawRect(0, 19, 160, 27, 0x0000);
	etft.fillRect(1, 20, 158, 25, 0x1A6F);
	etft.setCursor(13, 27);
	etft.print(last_meter_data.timestamp_str);

	// TextBox 2
	etft.drawRect(0, 99, 25, 29, 0x0000);
	etft.fillRect(1, 100, 23, 27, 0xF590);
	etft.setTextColor(TFT_RED);
	etft.setTTFFont(Arial_16_Bold);
	etft.setCursor(9, 105);
	etft.print(F("I"));

	// TextBox 2
	etft.drawRect(0, 72, 25, 28, 0x0000);
	etft.fillRect(1, 73, 23, 26, 0x8D5B);
	etft.setTextColor(TFT_BLUE);
	etft.setTTFFont(Arial_14_Bold);
	etft.setCursor(5, 79);
	etft.print(F("U"));

	// TextBox 2
	etft.drawRect(0, 45, 25, 28, 0x0000);
	etft.fillRect(1, 46, 23, 26, 0x0000);
	etft.drawLine(24, 45, 0, 72, 0x0000);

	// TextBox 2
	etft.drawRect(24, 99, 46, 29, 0x0000);
	etft.fillRect(25, 100, 44, 27, 0xF590);
	etft.setTextColor(0x0000);
	etft.setTTFFont(Arial_9_Bold);
	etft.setCursor(34, 110);
	etft.printf("%.1f", last_meter_data.current_l1);

	// TextBox 2
	etft.drawRect(69, 99, 46, 29, 0x0000);
	etft.fillRect(70, 100, 44, 27, 0xF590);
	etft.setCursor(79, 110);
	etft.printf("%.1f", last_meter_data.current_l2);

	// TextBox 2
	etft.drawRect(114, 99, 46, 29, 0x0000);
	etft.fillRect(115, 100, 44, 27, 0xF590);
	etft.setCursor(124, 110);
	etft.printf("%.1f", last_meter_data.current_l3);

	// TextBox 2
	etft.drawRect(24, 72, 46, 28, 0x0000);
	etft.fillRect(25, 73, 44, 26, 0x8D5B);
	etft.setCursor(31, 82);
	etft.printf("%.1f", last_meter_data.voltage_l1);

	// TextBox 2
	etft.drawRect(69, 72, 46, 28, 0x0000);
	etft.fillRect(70, 73, 44, 26, 0x8D5B);
	etft.setCursor(76, 82);
	etft.printf("%.1f", last_meter_data.voltage_l2);

	// TextBox 2
	etft.drawRect(114, 72, 46, 28, 0x0000);
	etft.fillRect(115, 73, 44, 26, 0x8D5B);
	etft.setCursor(121, 82);
	etft.printf("%.1f", last_meter_data.voltage_l3);

	// TextBox 2
	etft.drawRect(24, 45, 46, 28, 0x0000);
	etft.fillRect(25, 46, 44, 26, 0x39C7);
	etft.setTextColor(0xFFFF);
	etft.setTTFFont(Arial_16_Bold);
	etft.setCursor(33, 50);
	etft.print(F("L1"));

	// TextBox 2
	etft.drawRect(69, 45, 46, 28, 0x0000);
	etft.fillRect(70, 46, 44, 26, 0x39C7);
	etft.setCursor(78, 50);
	etft.print(F("L2"));

	// TextBox 2
	etft.drawRect(114, 45, 46, 28, 0x0000);
	etft.fillRect(115, 46, 44, 26, 0x39C7);
	etft.setCursor(123, 50);
	etft.print(F("L3"));
}

void Screen2()
{
	etft.fillScreen(0xFFFF);

	// TextBox 2
	etft.drawRect(0, 0, 160, 20, 0x0861);
	etft.fillRect(1, 1, 158, 18, 0x39C7);
	etft.setTextColor(0xFFFF);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(15, 4);
	etft.print(F(HEADER));

	// TextBox 2
	etft.drawRect(0, 19, 160, 27, 0x0000);
	etft.fillRect(1, 20, 158, 25, 0x1A6F);
	etft.setCursor(13, 27);
	etft.print(last_meter_data.timestamp_str);

	// TextBox 2
	etft.drawRect(0, 45, 76, 28, 0x0000);
	etft.fillRect(1, 46, 74, 26, 0x8D5B);
	etft.setTextColor(0x02C3);
	etft.setTTFFont(Arial_14_Bold);
	etft.setCursor(15, 50);
	etft.print(F("P    ="));

	// TextBox 2
	etft.drawRect(0, 72, 76, 28, 0x0000);
	etft.fillRect(1, 73, 74, 26, 0x8D5B);
	etft.setCursor(15, 77);
	etft.print(F("P    ="));

	// TextBox 1
	etft.setTTFFont(Arial_9_Bold);
	etft.setCursor(28, 60);
	etft.print(F("AB"));

	// TextBox 22
	etft.setCursor(29, 87);
	etft.print(F("ZU"));

	// TextBox 2
	etft.drawRect(0, 99, 76, 29, 0x0000);
	etft.fillRect(1, 100, 74, 27, 0x8D5B);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(0, 107);
	etft.print(F(" cos(phi)"));

	// TextBox 2
	etft.drawRect(75, 99, 85, 29, 0x0000);
	etft.fillRect(76, 100, 83, 27, 0x8D5B);
	etft.setTextColor(0x0000);
	etft.setTTFFont(Arial_14_Bold);
	etft.setCursor(82, 106);
	etft.printf("%.3f", last_meter_data.cos_phi);

	// TextBox 2
	etft.drawRect(75, 72, 85, 28, 0x0000);
	etft.fillRect(76, 73, 83, 26, 0x8D5B);
	etft.setCursor(82, 79);
	etft.printf("%.0f W", last_meter_data.power_plus);

	// TextBox 2
	etft.drawRect(75, 45, 85, 28, 0x0000);
	etft.fillRect(76, 46, 83, 26, 0x8D5B);
	etft.setCursor(82, 52);
	etft.printf("%.0f W", last_meter_data.power_minus);

	// TextBox 2
	etft.setTextColor(0x02C3);
	etft.setCursor(62, 106);
	etft.print("=");
}

void Screen3()
{
	etft.fillScreen(0xFFFF);

	// TextBox 2
	etft.drawRect(0, 0, 160, 20, 0x0861);
	etft.fillRect(1, 1, 158, 18, 0x39C7);
	etft.setTextColor(0xFFFF);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(15, 4);
	etft.print(F(HEADER));

	// TextBox 2
	etft.drawRect(0, 19, 160, 27, 0x0000);
	etft.fillRect(1, 20, 158, 25, 0x1A6F);
	etft.setCursor(13, 27);
	etft.print(last_meter_data.timestamp_str);

	// TextBox 2
	etft.drawRect(0, 45, 51, 28, 0x0000);
	etft.fillRect(1, 46, 49, 26, 0x8D5B);
	etft.setTextColor(0xC2C2);
	etft.setTTFFont(Arial_14_Bold);
	etft.setCursor(1, 50);
	etft.print(F("E    ="));

	// TextBox 1
	etft.setTTFFont(Arial_9_Bold);
	etft.setCursor(15, 60);
	etft.print(F("AB"));

	// TextBox 2
	etft.drawRect(0, 72, 51, 29, 0x0000);
	etft.fillRect(1, 73, 49, 27, 0x8D5B);
	etft.setTTFFont(Arial_14_Bold);
	etft.setCursor(1, 77);
	etft.print(F("E    ="));

	// TextBox 22
	etft.setTTFFont(Arial_9_Bold);
	etft.setCursor(16, 87);
	etft.print(F("ZU"));

	// TextBox 2
	etft.drawRect(50, 72, 110, 29, 0x0000);
	etft.fillRect(51, 73, 108, 27, 0x8D5B);
	etft.setTextColor(0x0000);
	etft.setTTFFont(Arial_13_Bold);
	etft.setCursor(52, 79);
	if (last_meter_data.energy_plus >= 10e6)
		etft.printf("%.0f kWh", last_meter_data.energy_plus / 1e3);
	else
		etft.printf("%.1f kWh", last_meter_data.energy_plus / 1e3);

	// TextBox 2
	etft.drawRect(50, 45, 110, 28, 0x0000);
	etft.fillRect(51, 46, 108, 26, 0x8D5B);
	etft.setCursor(52, 52);
	if (last_meter_data.energy_plus >= 10e6)
		etft.printf("%.0f kWh", last_meter_data.energy_minus / 1e3);
	else
		etft.printf("%.1f kWh", last_meter_data.energy_minus / 1e3);

	// TextBox 2
	etft.drawRect(0, 100, 160, 28, 0x0000);
	etft.fillRect(1, 101, 158, 26, 0x8D5B);
	etft.setTTFFont(Arial_8_Bold);
	etft.setCursor(27, 109);
	etft.print(F("(seit Aufzeichnung)"));
}

void Screen4()
{
	etft.fillScreen(0xFFFF);

	// TextBox 2
	etft.drawRect(0, 0, 160, 20, 0x0861);
	etft.fillRect(1, 1, 158, 18, 0x39C7);
	etft.setTextColor(0xFFFF);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(15, 4);
	etft.print(F(HEADER));

	// TextBox 2
	etft.drawRect(0, 19, 160, 27, 0x0000);
	etft.fillRect(1, 20, 158, 25, 0x1A6F);
	etft.setCursor(13, 27);
	etft.print(last_meter_data.timestamp_str);

	// TextBox 2
	etft.drawRect(0, 45, 80, 56, 0x0000);
	etft.fillRect(1, 46, 78, 54, 0x8D5B);
	etft.setTextColor(0x0000);
	etft.setTTFFont(Arial_18_Bold);
	etft.setCursor(6, 65);
	etft.printf("%.1fC", last_meter_data.temperature);

	// TextBox 2
	etft.drawRect(79, 45, 81, 56, 0x0000);
	etft.fillRect(80, 46, 79, 54, 0x8D5B);
	etft.setCursor(86, 65);
	etft.printf("%.1f%%", last_meter_data.humidity);

	// TextBox 2
	etft.drawRect(0, 100, 160, 28, 0x0000);
	etft.fillRect(1, 101, 158, 26, 0x8D5B);
	etft.setTTFFont(Arial_8_Bold);
	etft.setCursor(27, 108);
	etft.printf("WiFi Signal: %d dBm", last_meter_data.rssi);
}

void Screen5()
{
	etft.fillScreen(0xFFFF);

	etft.drawRect(0, 0, 160, 20, 0x0861);
	etft.fillRect(1, 1, 158, 18, 0x39C7);
	etft.setTextColor(0xFFFF);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(15, 4);
	etft.print(F(HEADER));

	etft.drawRect(0, 19, 160, 27, 0x0000);
	etft.fillRect(1, 20, 158, 25, 0x1A6F);
	etft.setTextColor(0xE73C);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(30, 27);
	etft.print(F("Einstellungen"));

	etft.drawRect(0, 45, 160, 83, 0x0000);
	etft.fillRect(1, 46, 158, 81, 0x8D5B);

	etft.setTextColor(0x0000);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(17, 54);
	etft.print(F("Neustart"));

	etft.setTextColor(0x0000);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(17, 78);
	etft.print(F("Systeminfo"));

	etft.setTextColor(0x0000);
	etft.setTTFFont(Arial_10_Bold);
	etft.setCursor(17, 102);
	etft.print(F("Einstellungen"));

	etft.fillTriangle(6, 62, 6, 55, 12, 59, 0x0000);
}

void Screen6()
{
	etft.fillScreen(0x0);
	etft.setTextColor(0xFFFF);
	etft.setTTFFont(Arial_8);
	etft.setCursor(0, 0);

	etft.printf("Name: %s\r\n", WiFi.getHostname());
	etft.println("WiFi SSID: " + WiFi.SSID());
	etft.printf("WiFi Signal: %d dBm\r\n", WiFi.RSSI());
	etft.println("WiFi IP: " + WiFi.localIP().toString());
	int64_t sec = esp_timer_get_time() / 1000000;
	int64_t up_days = int64_t(floor(sec / 86400));
	int up_hours = int64_t(floor(sec / 3600)) % 24;
	int up_min = int64_t(floor(sec / 60)) % 60;
	int up_sec = sec % 60;

	etft.printf("Up Time: %" PRId64 ":%02i:%02i:%02i (d:h:m:s)\r\n", up_days, up_hours, up_min, up_sec);
	etft.println("SD Card disabled\r\n");
	etft.setTextColor(TFT_GREEN);
	etft.setCursor(0, 120);
	etft.print("Press NEXT to continue");
}