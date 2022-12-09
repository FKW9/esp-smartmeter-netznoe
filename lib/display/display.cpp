#include "display.h"
#include "screens.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSPI_ext etft = TFT_eSPI_ext(&tft);

uint8_t current_screen = 0;
uint8_t previous_screen = 0;
uint8_t screens = 4;
uint8_t display_on = 1;
meterData last_meter_data;

void ICACHE_RAM_ATTR buttonEnterPressed();
void ICACHE_RAM_ATTR buttonNextPressed();

void setupDisplay()
{
	etft.init();
	etft.setRotation(1);
	etft.fillScreen(TFT_BLACK);
	etft.setTextColor(TFT_WHITE, TFT_BLACK);
	etft.println("<<< INITIALIZING >>>");

	// Button Pins
	pinMode(33, INPUT_PULLUP);
	pinMode(32, INPUT_PULLUP);
	attachInterrupt(33, buttonEnterPressed, FALLING);
	attachInterrupt(32, buttonNextPressed, FALLING);
}

void displaySDCardStatus()
{
	// etft.setCursor(1, 17);
	// etft.setTextColor(TFT_BLACK);
	// etft.fillRect(110, 17, 50, 7, TFT_WHITE);
	uint64_t s = getFreeSDSpace();
	if (s == 0)
	{
		etft.setTextColor(TFT_RED);
		etft.println("NO SD CARD FOUND!");
	}
	else
	{
		etft.printf("SD Free Space: ");
		etft.printf("%lluMB\n", s);
	}
}

void displayWiFiInfo()
{
	etft.print("WiFi: ");
	if (WiFi.isConnected())
	{
		etft.setTextColor(TFT_GREEN);
		etft.println("Connected");
		etft.setTextColor(TFT_WHITE);
		etft.printf("IP: %s\n", WiFi.localIP().toString().c_str());
		etft.printf("Signal: %ddBm\n", WiFi.RSSI());
	}
	else
	{
		etft.setTextColor(TFT_RED);
		etft.println("Not Connected");
	}
}

void displayMeterData(meterData *data)
{
	last_meter_data = *data;
	displayUpdate(true);
}

void displayUpdate(bool force)
{
	if (display_on)
	{
		if ((current_screen != previous_screen) || force)
		{
			switch (current_screen)
			{
			case 0:
				Screen1();
				break;

			case 1:
				Screen2();
				break;

			case 2:
				Screen3();
				break;

			case 3:
				Screen4();
				break;

			default:
				break;
			}
			if (!force)
				previous_screen = current_screen;
		}
	}
}

//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;

void ICACHE_RAM_ATTR buttonEnterPressed()
{
	button_time = millis();
	if (button_time - last_button_time > 400)
	{
		Serial.println("UP pressed");
		last_button_time = button_time;
	}
}

void ICACHE_RAM_ATTR buttonNextPressed()
{
	button_time = millis();
	if (button_time - last_button_time > 400)
	{
		if (display_on)
			current_screen++;
		else
			current_screen = 0;

		if (current_screen >= screens)
			current_screen = 0;

		last_button_time = button_time;
	}
}

void displayInactiveTimer()
{
	if (millis() - last_button_time > 120e3)
	{
		display_on = 0;
		digitalWrite(TFT_BL, !TFT_BACKLIGHT_ON);
	}
	else if (digitalRead(TFT_BL) != TFT_BACKLIGHT_ON)
	{
		display_on = 1;
		digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
	}
}