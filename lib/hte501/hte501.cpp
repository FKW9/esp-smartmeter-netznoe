#include "hte501.h"

unsigned char CMD_SINGLE_SHOT[] = {0x2C, 0x1B};
unsigned char CMD_PERIODIC_READY[] = {0xF3, 0x52};
unsigned char CMD_PERIODIC_START[] = {0x20, 0x1E};
unsigned char CMD_PERIODIC_FETCH[] = {0xE0, 0x00};

void setupSensor(void)
{
    Wire.begin();
}

void wireWrite(unsigned char buf[], int to, bool stopmessage)
{
    Wire.beginTransmission(HTE_ADDRESS);
    for (int i = 0; i <= to; i++)
    {
        Wire.write(buf[i]);
    }
    Wire.endTransmission(stopmessage);
}

void wireRead(unsigned char buf[], int to)
{
    int i = 0;
    Wire.requestFrom(HTE_ADDRESS, to);
    while (Wire.available())
    {
        buf[i++] = Wire.read();
    }
}

unsigned char calcCrc8(unsigned char buf[], unsigned char from, unsigned char to)
{
    unsigned char crcVal = 0xFF;
    unsigned char i = 0;
    unsigned char j = 0;
    for (i = from; i <= to; i++)
    {
        int curVal = buf[i];
        for (j = 0; j < 8; j++)
        {
            if (((crcVal ^ curVal) & 0x80) != 0)
            {
                crcVal = ((crcVal << 1) ^ 0x31);
            }
            else
            {
                crcVal = (crcVal << 1);
            }
            curVal = curVal << 1;
        }
    }
    return crcVal;
}

uint8_t getTemperatureHumidity(float &temperature, float &humidity)
{
    unsigned char i2cResponse[6];
    wireWrite(CMD_SINGLE_SHOT, 1, false);
    wireRead(i2cResponse, 6);
    if (i2cResponse[2] == calcCrc8(i2cResponse, 0, 1) && i2cResponse[5] == calcCrc8(i2cResponse, 3, 4))
    {
        temperature = ((float)(i2cResponse[0]) * 256 + i2cResponse[1]) / 100;
        humidity = ((float)(i2cResponse[3]) * 256 + i2cResponse[4]) / 100;
        return 1;
    }
    else
        return 0;
}

void startPeriodicMeasurement(void)
{
    wireWrite(CMD_PERIODIC_START, 1, false);
}

uint8_t changePeriodicMeasurementTime(uint32_t millisec)
{
    unsigned char sendBytes[2];
    if (3276751 > millisec)
    {
        int value = millisec / 50;
        sendBytes[1] = value / 255;
        sendBytes[0] = value % 256;
        unsigned char crc8[] = {0x10, sendBytes[0], sendBytes[1]};
        unsigned char Command[] = {0x72, 0xA7, 0x10, sendBytes[0], sendBytes[1], calcCrc8(crc8, 0, 2)};
        wireWrite(Command, 5, true);
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t newMeasurementReady(uint8_t &ready)
{
    unsigned char i2cResponse[3];
    wireWrite(CMD_PERIODIC_READY, 1, false);
    wireRead(i2cResponse, 3);
    if (i2cResponse[2] == calcCrc8(i2cResponse, 0, 1))
    {
        ready = i2cResponse[0] >> 7;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t fetchPeriodicTemperatureHumidity(float &temperature, float &humidity)
{
    unsigned char i2cResponse[6];
    wireWrite(CMD_PERIODIC_FETCH, 1, false);
    wireRead(i2cResponse, 6);
    if (i2cResponse[2] == calcCrc8(i2cResponse, 0, 1) && i2cResponse[5] == calcCrc8(i2cResponse, 3, 4))
    {
        temperature = ((float)(i2cResponse[0]) * 256 + i2cResponse[1]) / 100;
        humidity = ((float)(i2cResponse[3]) * 256 + i2cResponse[4]) / 100;
        return 1;
    }
    else
        return 0;
}