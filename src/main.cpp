#include <Arduino.h>
#include <mbedtls/gcm.h>
#include "dlms.h"
#include "obis.h"
#include "../lib/graphite/graphite.h"
#include "../lib/display/display.h"
#include "../config.h"

#ifdef ESP8266
    HardwareSerial Serial2 = Serial;
#endif

uint32_t last_read = 0;                      // Timestamp when data was last read
uint16_t receive_buffer_index = 0;           // Current position in the receive buffer
uint8_t receive_buffer[RECEIVE_BUFFER_SIZE]; // Stores the received data

mbedtls_gcm_context aes;

uint32_t swap_uint32(uint32_t val);
uint16_t swap_uint16(uint16_t val);

void setup()
{
    #ifndef ESP8266
        Serial.begin(9600); // Debug port
    #endif

    // MBus input from MBus Slave Click
    Serial2.begin(2400, SERIAL_8E1);
    Serial2.setRxBufferSize(RECEIVE_BUFFER_SIZE);
    Serial2.setTimeout(2);

    #ifdef ESP8266
        Serial = Serial2;
    #endif

    setupDisplay();
    setupWiFi();
}

void loop()
{
    checkWiFiConnection();

    uint32_t current_time = millis();

    // Read while data is available
    while (Serial2.available())
    {
        if (receive_buffer_index >= RECEIVE_BUFFER_SIZE)
        {
            Serial.println("Buffer overflow!");
            receive_buffer_index = 0;
        }

        receive_buffer[receive_buffer_index++] = Serial2.read();

        last_read = current_time;
    }

    if (receive_buffer_index > 0 && current_time - last_read > READ_TIMEOUT)
    {
        if (receive_buffer_index < 256)
        {
            Serial.println("Received packet with invalid size!");
            receive_buffer_index = 0;
            return;
        }

        /**
         * @TODO: ADD ROUTINE TO DETERMINE PAYLOAD LENGTHS AUTOMATICALLY
         */

        uint16_t payload_length = 243;
        uint16_t payload_length_msg1 = 228;
        uint16_t payload_length_msg2 = payload_length - payload_length_msg1;

        uint8_t iv[12]; // Initialization vector

        memcpy(&iv[0], &receive_buffer[DLMS_SYST_OFFSET], DLMS_SYST_LENGTH); // Copy system title to IV
        memcpy(&iv[8], &receive_buffer[DLMS_IC_OFFSET], DLMS_IC_LENGTH);     // Copy invocation counter to IV

        uint8_t ciphertext[payload_length];
        memcpy(&ciphertext[0], &receive_buffer[DLMS_HEADER1_LENGTH], payload_length_msg1);
        memcpy(&ciphertext[payload_length_msg1], &receive_buffer[DLMS_HEADER2_OFFSET + DLMS_HEADER2_LENGTH], payload_length_msg2);

        // Start decrypting
        uint8_t plaintext[payload_length];

        mbedtls_gcm_init(&aes);
        mbedtls_gcm_setkey(&aes, MBEDTLS_CIPHER_ID_AES, KEY, KEY_LENGTH * 8);
        mbedtls_gcm_auth_decrypt(&aes, payload_length, iv, sizeof(iv), NULL, 0, NULL, 0, ciphertext, plaintext);
        mbedtls_gcm_free(&aes);

        if (plaintext[0] != 0x0F || plaintext[5] != 0x0C)
        {
            Serial.println("Packet was decrypted but data is invalid!");
            receive_buffer_index = 0;
            return;
        }

        // Decode data
        uint16_t current_position = DECODER_START_OFFSET;
        time_t unix_timestamp = -1;

        do
        {
            if (plaintext[current_position + OBIS_TYPE_OFFSET] != DATA_OCTET_STRING)
            {
                Serial.println("Unsupported OBIS header type!");
                receive_buffer_index = 0;
                return;
            }

            uint8_t data_length = plaintext[current_position + OBIS_LENGTH_OFFSET];

            if (data_length != 0x06)
            {
                // read timestamp
                if ((data_length == 0x0C) && (current_position == DECODER_START_OFFSET))
                {
                    uint8_t dateTime[data_length];
                    memcpy(&dateTime[0], &plaintext[current_position + 2], data_length);

                    uint16_t year;
                    uint8_t month;
                    uint8_t day;
                    uint8_t hour;
                    uint8_t minute;
                    uint8_t second;

                    year = (plaintext[current_position + 2] << 8) + plaintext[current_position + 3];
                    month = plaintext[current_position + 4];
                    day = plaintext[current_position + 5];
                    hour = plaintext[current_position + 7];
                    minute = plaintext[current_position + 8];
                    second = plaintext[current_position + 9];

                    char timeStamp[21];
                    sprintf(timeStamp, "%02u.%02u.%04u %02u:%02u:%02u", day, month, year, hour, minute, second);

                    // convert to unix timestamp for graphite
                    struct tm tm;
                    if (strptime(timeStamp, "%d.%m.%Y %H:%M:%S", &tm) != NULL)
                    {
                        unix_timestamp = mktime(&tm) - 7200;
                        Serial.print("Unix Time: ");
                        Serial.println(unix_timestamp);
                    }
                    else
                    {
                        Serial.println("Invalid Timestamp");
                        receive_buffer_index = 0;
                        return;
                    }

                    Serial.print("Timestamp: ");
                    Serial.println(timeStamp);

                    // Update Display
                    updateTimestamp(timeStamp);

                    current_position = 34;
                    data_length = plaintext[current_position + OBIS_LENGTH_OFFSET];
                }
                else if ((data_length == 0x0C) && (current_position > 225))
                {
                    uint8_t meterNumber[data_length];
                    memcpy(&meterNumber[0], &plaintext[current_position + 2], data_length);

                    // THIS IS THE END OF THE PACKET
                    break;
                }
                else
                {
                    Serial.println("Unsupported OBIS header length");
                    receive_buffer_index = 0;
                    return;
                }
            }

            uint8_t obis_code[data_length];
            memcpy(&obis_code[0], &plaintext[current_position + OBIS_CODE_OFFSET], data_length); // Copy OBIS code to array

            current_position += data_length + 2; // Advance past code, position and type

            uint8_t obis_data_type = plaintext[current_position];
            current_position++; // Advance past data type

            uint8_t obis_data_length = 0x00;
            uint8_t code_type = TYPE_UNKNOWN;

            if (obis_code[OBIS_A] == 0x01)
            {
                // Compare C and D against code
                if (memcmp(&obis_code[OBIS_C], OBIS_VOLTAGE_L1, 2) == 0)
                {
                    code_type = TYPE_VOLTAGE_L1;
                }
                else if (memcmp(&obis_code[OBIS_C], OBIS_VOLTAGE_L2, 2) == 0)
                {
                    code_type = TYPE_VOLTAGE_L2;
                }
                else if (memcmp(&obis_code[OBIS_C], OBIS_VOLTAGE_L3, 2) == 0)
                {
                    code_type = TYPE_VOLTAGE_L3;
                }

                else if (memcmp(&obis_code[OBIS_C], OBIS_CURRENT_L1, 2) == 0)
                {
                    code_type = TYPE_CURRENT_L1;
                }
                else if (memcmp(&obis_code[OBIS_C], OBIS_CURRENT_L2, 2) == 0)
                {
                    code_type = TYPE_CURRENT_L2;
                }
                else if (memcmp(&obis_code[OBIS_C], OBIS_CURRENT_L3, 2) == 0)
                {
                    code_type = TYPE_CURRENT_L3;
                }

                else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_POWER_PLUS, 2) == 0)
                {
                    code_type = TYPE_ACTIVE_POWER_PLUS;
                }
                else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_POWER_MINUS, 2) == 0)
                {
                    code_type = TYPE_ACTIVE_POWER_MINUS;
                }

                else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_ENERGY_PLUS, 2) == 0)
                {
                    code_type = TYPE_ACTIVE_ENERGY_PLUS;
                }
                else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_ENERGY_MINUS, 2) == 0)
                {
                    code_type = TYPE_ACTIVE_ENERGY_MINUS;
                }

                else if (memcmp(&obis_code[OBIS_C], OBIS_POWER_FACTOR, 2) == 0)
                {
                    code_type = TYPE_POWER_FACTOR;
                }
                else
                {
                    Serial.println("Unsupported OBIS code");
                }
            }
            else
            {
                Serial.println("Unsupported OBIS medium");
                receive_buffer_index = 0;
                return;
            }

            uint16_t uint16_value;
            uint32_t uint32_value;
            float float_value;

            switch (obis_data_type)
            {
            case DATA_LONG_DOUBLE_UNSIGNED:
                obis_data_length = 4;

                memcpy(&uint32_value, &plaintext[current_position], 4); // Copy uint8_ts to integer
                uint32_value = swap_uint32(uint32_value);               // Swap uint8_ts

                float_value = uint32_value; // Ignore decimal digits for now

                switch (code_type)
                {
                case TYPE_ACTIVE_POWER_PLUS:
                    Serial.print("ActivePowerPlus ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_ACTIVE_POWER_PLUS, float_value);
                    break;

                case TYPE_ACTIVE_POWER_MINUS:
                    Serial.print("ActivePowerMinus ");
                    Serial.println(float_value);
                    // submitToGraphite(unix_timestamp, GRAPHITE_ACTIVE_POWER_MINUS, float_value);
                    break;

                case TYPE_ACTIVE_ENERGY_PLUS:
                    Serial.print("ActiveEnergyPlus ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_ACTIVE_ENERGY_PLUS, float_value);
                    break;

                case TYPE_ACTIVE_ENERGY_MINUS:
                    Serial.print("ActiveEnergyMinus ");
                    Serial.println(float_value);
                    // submitToGraphite(unix_timestamp, GRAPHITE_ACTIVE_ENERGY_MINUS, float_value);
                    break;
                }
                break;

            case DATA_LONG:
            case DATA_LONG_UNSIGNED:
                obis_data_length = 2;

                memcpy(&uint16_value, &plaintext[current_position], 2); // Copy uint8_ts to integer
                uint16_value = swap_uint16(uint16_value);               // Swap uint8_ts

                if (plaintext[current_position + 5] == SCALE_TENTHS)
                    float_value = uint16_value / 10.0;
                else if (plaintext[current_position + 5] == SCALE_HUNDREDTHS)
                    float_value = uint16_value / 100.0;
                else if (plaintext[current_position + 5] == SCALE_THOUSANDS)
                    float_value = uint16_value / 1000.0;
                else
                    float_value = uint16_value;

                switch (code_type)
                {
                case TYPE_VOLTAGE_L1:
                    Serial.print("VoltageL1 ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_VOLTAGE_L1, float_value);
                    break;

                case TYPE_VOLTAGE_L2:
                    Serial.print("VoltageL2 ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_VOLTAGE_L2, float_value);
                    break;

                case TYPE_VOLTAGE_L3:
                    Serial.print("VoltageL3 ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_VOLTAGE_L3, float_value);
                    break;

                case TYPE_CURRENT_L1:
                    Serial.print("CurrentL1 ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_CURRENT_L1, float_value);
                    break;

                case TYPE_CURRENT_L2:
                    Serial.print("CurrentL2 ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_CURRENT_L2, float_value);
                    break;

                case TYPE_CURRENT_L3:
                    Serial.print("CurrentL3 ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_CURRENT_L3, float_value);
                    break;

                case TYPE_POWER_FACTOR:
                    Serial.print("PowerFactor ");
                    Serial.println(float_value);
                    submitToGraphite(unix_timestamp, GRAPHITE_POWER_FACTOR, float_value);
                    break;
                }
                break;

            case DATA_OCTET_STRING:
                obis_data_length = plaintext[current_position];
                current_position++; // Advance past string length
                break;

            default:
                Serial.println("Unsupported OBIS data type");
                receive_buffer_index = 0;
                return;
                break;
            }

            current_position += obis_data_length; // Skip data length

            current_position += 2; // Skip pause after data

            if (plaintext[current_position] == 0x0F)  // There is still additional data for this type, skip it
                current_position += 4;                // Skip additional data and additional break; this will jump out of bounds on last frame
        } while (current_position <= payload_length); // Loop until end of packet

        receive_buffer_index = 0;
        Serial.println("Received valid data!");

        submitToGraphite(unix_timestamp, GRAPHITE_RSSI, WiFi.RSSI());
    }
}

uint16_t swap_uint16(uint16_t val)
{
    return (val << 8) | (val >> 8);
}

uint32_t swap_uint32(uint32_t val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}
