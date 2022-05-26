#include "display.h"

// #ifdef ESP8266
//   SSD1306Wire display(0x3c, 2, 0);
// #else
//   SSD1306Wire display(0x3c, SDA, SCL);
// #endif

void setupDisplay()
{
  tft.init();

  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  delay(100);
}
