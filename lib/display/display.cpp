#include "display.h"

void setupDisplay()
{
  tft.init();

  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  delay(100);
}
