#include "display.h"
#include "screens.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSPI_ext etft = TFT_eSPI_ext(&tft);

uint8_t current_screen = 0;
uint8_t previous_screen = 0;
uint8_t screens = 5;
uint8_t display_on = 1;
uint8_t cursor_pos = 0;
meterData last_meter_data;
bool config_update = false;

void ICACHE_RAM_ATTR buttonEnterPressed();
void ICACHE_RAM_ATTR buttonNextPressed();
void setNextCursor();

void displaySetup()
{
	etft.init();
	etft.setRotation(1);
	etft.fillScreen(TFT_BLACK);
	etft.setTextColor(TFT_WHITE);
	etft.setTTFFont(Arial_8);
	etft.println("=======INITIALISIEREN=======");

	// Button Pins
	pinMode(33, INPUT_PULLUP);
	pinMode(32, INPUT_PULLUP);
	attachInterrupt(33, buttonEnterPressed, FALLING);
	attachInterrupt(32, buttonNextPressed, FALLING);
}

void displaySDCardStatus()
{
	uint64_t s = getFreeSDSpace();
	if (s == 0)
	{
		etft.setTextColor(TFT_RED);
		etft.println("Keine SD Karte gefunden!");
		etft.setTextColor(TFT_WHITE);
	}
	else
	{
		etft.printf("SD Karte: ");
		etft.printf("%lluMB frei\n", s);
	}
}

void displayWiFiInfo()
{
	etft.print("WiFi: ");
	if (WiFi.isConnected())
	{
		etft.setTextColor(TFT_GREEN);
		etft.println("Verbunden");
		etft.setTextColor(TFT_WHITE);
		etft.printf("IP: %s\n", WiFi.localIP().toString().c_str());
		etft.printf("Signal: %ddBm\n", WiFi.RSSI());
	}
	else
	{
		etft.setTextColor(TFT_RED);
		etft.println("Nicht verbunden");
	}
}

void displayMeterData(meterData *data)
{
	last_meter_data = *data;
	displayUpdate(true);
}

void displayUpdate(bool force)
{
	if (display_on && !config_update)
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

			case 4:
				if (!force)
					Screen5();
				break;

			case 5:
				if (!force)
					Screen6();
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
		if (current_screen == 4)
		{
			switch (cursor_pos)
			{
			case 0:
				Serial2.end();
				etft.fillScreen(0);
				etft.setTextColor(0xFFFF);
				etft.setCursor(0, 0);
				etft.setTTFFont(Arial_8);
				etft.println("Restart manually executed...");
				Serial.println("Restart manually executed...");
				delay(3000);
				WiFi.disconnect();
				ESP.restart();
				break;
			case 1:
				current_screen = 5;
				break;
			case 2:
				etft.fillScreen(0);
				etft.setTextColor(0xFFFF);
				etft.setCursor(0, 0);
				etft.setTTFFont(Arial_8);
				config_update = true;
				break;

			default:
				break;
			}
		}
		last_button_time = button_time;
	}
}

void ICACHE_RAM_ATTR buttonNextPressed()
{
	button_time = millis();
	if (button_time - last_button_time > 400)
	{
		if (display_on)
		{
			if (current_screen != 4 || cursor_pos == 2)
			{
				current_screen++;
				cursor_pos = 0;
			}
			else
			{
				setNextCursor();
			}
		}
		else
		{
			display_on = 1;
			digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
			current_screen = 0;
			displayUpdate(true);
		}

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
}

void setNextCursor()
{
	switch (cursor_pos)
	{
	case 0:
		etft.fillTriangle(6, 62, 6, 55, 12, 59, 0x8D5B);
		etft.fillTriangle(6, 86, 6, 79, 12, 83, 0x0000);
		break;

	case 1:
		etft.fillTriangle(6, 86, 6, 79, 12, 83, 0x8D5B);
		etft.fillTriangle(6, 110, 6, 103, 12, 107, 0x0000);
		break;

	case 2:
		etft.fillTriangle(6, 110, 6, 103, 12, 107, 0x8D5B);
		etft.fillTriangle(6, 62, 6, 55, 12, 59, 0x0000);
		break;

	default:
		break;
	}

	if (++cursor_pos > 2)
		cursor_pos = 0;
}

void displayResetTextSettings()
{
	digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
	etft.fillScreen(0x0);
	etft.setCursor(0, 0);
	etft.setTextColor(TFT_WHITE);
	etft.setTTFFont(Arial_8);
}