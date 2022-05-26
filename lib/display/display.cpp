#include "display.h"

TFT_eSPI tft = TFT_eSPI();

void setupDisplay()
{
  tft.init();

  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  delay(100);

  tft.println("------ INITIALIZING ------");
}
