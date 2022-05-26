# ESP32/8266 Smartmeter decoder for Kaifa MA309M

## **Credits**
Main Code was created by [DomiStyle](https://github.com/DomiStyle) in his project [esphome-dlms-meter](https://github.com/DomiStyle/esphome-dlms-meter).
Thank you!

## Warning
This is a modified version which only supports the Kaifa MA309M Smartmeter from Netz NÖ. Also, does not support ESPHome.

## Features
- Reads data from the Smartmeter and sends it to Graphite, a time-series database.
- Displays current values on a 0.96inch OLED display

## Requirements
- [M-Bus to Serial Converter](https://www.mikroe.com/m-bus-slave-click), or similar
- ESP32 or similar boards
- Server which runs Graphite

## Used Hardware
- [ESP-32 Dev Kit v4](https://www.az-delivery.de/products/esp-32-dev-kit-c-v4) and Generic ESP-01 (ESP-8266) Board
- Synology DS218+ NAS which runs Graphite and Grafana
- [0.96inch OLED Display](https://www.az-delivery.de/products/0-96zolldisplay)
- M-Bus Adapter

## Settings
You have to change some settings, so that it fits your setup.
These settings are stored in ```config.h```

## Result
See folder "pictures"!