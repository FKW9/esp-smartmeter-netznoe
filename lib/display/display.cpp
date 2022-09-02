#include "display.h"

TFT_eSPI tft = TFT_eSPI();

void setupDisplay()
{
	tft.init();

	tft.setRotation(1);

	tft.fillScreen(TFT_BLACK);

	tft.setTextColor(TFT_WHITE, TFT_BLACK);

	delay(100);

	tft.println("------ INITIALIZING ------");
}

void displayRSSI()
{
	tft.setCursor(124, 9);
	tft.fillRect(124,9,36,7,TFT_WHITE);
	tft.setTextColor(TFT_BLACK);
	tft.printf("%ddBm", WiFi.RSSI());
}

void displaySDCardStatus()
{
	tft.setCursor(1, 17);
	tft.setTextColor(TFT_BLACK);
	tft.printf("SD Card free space: ");
	tft.fillRect(110,17,50,7,TFT_WHITE);
	uint64_t s = getFreeSDSpace();
	if (s == 0){
		tft.setTextColor(TFT_RED);
		tft.print("ERROR");
	} else {
		tft.printf("%lluMB\n", s);
	}
}

void displayWiFiInfo()
{
	tft.setCursor(1, 1);
	tft.fillRect(0, 0, 160, 25, TFT_WHITE);
	tft.setTextColor(TFT_BLACK);
	tft.print("WiFi: ");
	if (WiFi.isConnected())
	{
		tft.setTextColor(TFT_DARKGREEN);
		tft.println("Connected");
		tft.setTextColor(TFT_BLACK);
		tft.printf("IP: %s", WiFi.localIP().toString().c_str());
		displayRSSI();
	}
	else
	{
		tft.setTextColor(TFT_RED);
		tft.println("Not Connected");
	}
}

void displayMeterData(struct obisData *data)
{
	int16_t x = tft.getCursorX();
	int16_t y = tft.getCursorY();
}