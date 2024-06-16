#include <Arduino.h>
#include <mbedtls/gcm.h>
#include "dlms.h"
#include "obis.h"
#include "display.h"
#include "hte501.h"
#include "setup.h"
#include "../key.h"
#include "time.h"
#include "graphite.h"

#ifdef TEST_SETUP
#include <random>
#endif

// Variables for DLMS decoding
uint32_t last_read = 0;                      // Timestamp when data was last read
uint16_t receive_buffer_index = 0;           // Current position in the receive buffer
uint8_t receive_buffer[RECEIVE_BUFFER_SIZE]; // Stores the received data
mbedtls_gcm_context aes;

// Variables for RHT sensor
float temperature = 0, humidity = 0;
uint8_t rht_ready = 0;

// Function prototypes
int16_t get_signed_short(uint8_t msb, uint8_t lsb);
uint32_t swap_uint32(uint32_t val);
uint16_t swap_uint16(uint16_t val);
void serial_dump();
unsigned long getTime();

bool wifi_reconnect = false;

// SETUP
void setup()
{
    displaySetup();
    setupSensor();
    startPeriodicMeasurement();

    // Debug port
    Serial.begin(115200);

    // MBus input
    Serial2.begin(2400, SERIAL_8E1);
    Serial2.setRxBufferSize(RECEIVE_BUFFER_SIZE);
    Serial2.setTimeout(2);

    if (!startWiFi())
        ESP.restart();

    etft.printf("\nWarte auf Smartmeter Daten...");
    delay(1000);
}

// MAIN LOOP
void loop()
{
    checkWiFiConnection();
    displayInactiveTimer();
    displayUpdate();

    uint32_t current_time = millis();

    if (Serial.available())
    {
        char c = Serial.read();
        if (c == 'd')
            serial_dump();
        if (c == 'r')
            ESP.restart();
    }

    // Read while data is available
    while (Serial2.available())
    {
        if (receive_buffer_index >= RECEIVE_BUFFER_SIZE)
        {
            Serial.println("Buffer overflow!");
            receive_buffer_index = 0;
            return;
        }

        receive_buffer[receive_buffer_index++] = Serial2.read();

        last_read = current_time;
    }

#ifdef TEST_SETUP
    const uint8_t test_data[] = {0x68, 0xFA, 0xFA, 0x68, 0x53, 0xFF, 0x00, 0x01, 0x67, 0xDB, 0x08, 0x4B, 0x46, 0x4D, 0x67, 0x50, 0x00, 0x08, 0x81, 0x81, 0xF8, 0x20, 0x00, 0x00, 0x60, 0x05, 0xC1, 0xFA, 0x38, 0xBC, 0xC6, 0xD9, 0x59, 0x4E, 0x5A, 0x7C, 0x36, 0x59, 0x13, 0x8B, 0x7E, 0xE9, 0x9A, 0x83, 0x47, 0xBF, 0x50, 0xB9, 0x98, 0xF8, 0x33, 0x85, 0x81, 0x08, 0x45, 0x12, 0xA3, 0x78, 0x0C, 0xAC, 0xBF, 0x5B, 0xCB, 0x36, 0x67, 0x44, 0xC9, 0x9E, 0x99, 0xE5, 0x3A, 0x12, 0xE8, 0x67, 0xD4, 0xED, 0xE6, 0x8E, 0xF8, 0x2A, 0x91, 0xF6, 0xD7, 0x6C, 0x08, 0x6C, 0x73, 0xAD, 0x57, 0x54, 0xCE, 0x8F, 0x13, 0x13, 0xD9, 0xF7, 0x38, 0xC8, 0x50, 0xD4, 0x93, 0x62, 0x75, 0x0A, 0x9B, 0x6F, 0xCB, 0xE3, 0x3A, 0xDE, 0x62, 0xA1, 0x6C, 0x62, 0xA9, 0xB5, 0xCA, 0xC7, 0x93, 0x31, 0xE9, 0x78, 0x62, 0x81, 0x60, 0x4B, 0xC9, 0x24, 0x22, 0xA1, 0x24, 0xE0, 0xC3, 0xB9, 0xC9, 0x79, 0xD0, 0xE2, 0x65, 0xDA, 0x60, 0x14, 0x03, 0xC5, 0x56, 0xB8, 0x96, 0x09, 0x44, 0xC4, 0x24, 0xBA, 0xD4, 0x9D, 0x63, 0xAD, 0xAB, 0xFB, 0xAE, 0xDA, 0x07, 0x5D, 0x66, 0x88, 0x2F, 0x5C, 0xB5, 0x5A, 0x7A, 0xAD, 0x43, 0xFC, 0x24, 0x47, 0x0F, 0x3B, 0x23, 0xBD, 0x6B, 0xBB, 0xFF, 0x71, 0xC6, 0x44, 0x7A, 0x3D, 0xEB, 0x2E, 0xA4, 0x7D, 0x80, 0x60, 0xD7, 0xAA, 0xF6, 0x2F, 0x14, 0x9C, 0x0D, 0xD4, 0xCF, 0x60, 0xFD, 0xC9, 0x1C, 0x20, 0xED, 0x9A, 0x6D, 0xCC, 0xF0, 0x6D, 0xA5, 0x4D, 0xFB, 0x9B, 0x5F, 0x45, 0x60, 0xDF, 0xB5, 0x36, 0x4E, 0x0E, 0x01, 0x05, 0x6A, 0x07, 0x36, 0x4A, 0x60, 0x5F, 0x85, 0x75, 0xF3, 0x84, 0x1D, 0x51, 0x7D, 0x07, 0x22, 0x06, 0x14, 0xCF, 0xC7, 0xCF, 0x98, 0x82, 0x5E, 0xE7, 0x20, 0xC5, 0x1C, 0xBC, 0x59, 0x16, 0x68, 0x14, 0x14, 0x68, 0x53, 0xFF, 0x11, 0x01, 0x67, 0x4C, 0xC1, 0xD2, 0x17, 0xB8, 0xD8, 0x03, 0x4C, 0xC7, 0x51, 0x5F, 0xE0, 0x20, 0x95, 0x61, 0x0D, 0x16, 0x68, 0xFA, 0xFA, 0x68, 0x53, 0xFF, 0x00, 0x01, 0x67, 0xDB, 0x08, 0x4B, 0x46, 0x4D, 0x67, 0x50, 0x00, 0x08, 0x81, 0x81, 0xF8, 0x20, 0x00, 0x00, 0x60, 0x05, 0xC1, 0xFA, 0x38, 0xBC, 0xC6, 0xD9, 0x59, 0x4E, 0x5A, 0x7C, 0x36, 0x59, 0x13, 0x8B, 0x7E, 0xE9, 0x9A, 0x83, 0x47, 0xBF, 0x50, 0xB9, 0x98, 0xF8, 0x33, 0x85, 0x81, 0x08, 0x45, 0x12, 0xA3, 0x78, 0x0C, 0xAC, 0xBF, 0x5B, 0xCB, 0x36, 0x67, 0x44, 0xC9, 0x9E, 0x99, 0xE5, 0x3A, 0x12, 0xE8, 0x67, 0xD4, 0xED, 0xE6, 0x8E, 0xF8, 0x2A, 0x91, 0xF6, 0xD7, 0x6C, 0x08, 0x6C, 0x73, 0xAD, 0x57, 0x54, 0xCE, 0x8F, 0x13, 0x13, 0xD9, 0xF7, 0x38, 0xC8, 0x50, 0xD4, 0x93, 0x62, 0x75, 0x0A, 0x9B, 0x6F, 0xCB, 0xE3, 0x3A, 0xDE, 0x62, 0xA1, 0x6C, 0x62, 0xA9, 0xB5, 0xCA, 0xC7, 0x93, 0x31, 0xE9, 0x78, 0x62, 0x81, 0x60, 0x4B, 0xC9, 0x24, 0x22, 0xA1, 0x24, 0xE0, 0xC3, 0xB9, 0xC9, 0x79, 0xD0, 0xE2, 0x65, 0xDA, 0x60, 0x14, 0x03, 0xC5, 0x56, 0xB8, 0x96, 0x09, 0x44, 0xC4, 0x24, 0xBA, 0xD4, 0x9D, 0x63, 0xAD, 0xAB, 0xFB, 0xAE, 0xDA, 0x07, 0x5D, 0x66, 0x88, 0x2F, 0x5C, 0xB5, 0x5A, 0x7A, 0xAD, 0x43, 0xFC, 0x24, 0x47, 0x0F, 0x3B, 0x23, 0xBD, 0x6B, 0xBB, 0xFF, 0x71, 0xC6, 0x44, 0x7A, 0x3D, 0xEB, 0x2E, 0xA4, 0x7D, 0x80, 0x60, 0xD7, 0xAA, 0xF6, 0x2F, 0x14, 0x9C, 0x0D, 0xD4, 0xCF, 0x60, 0xFD, 0xC9, 0x1C, 0x20, 0xED, 0x9A, 0x6D, 0xCC, 0xF0, 0x6D, 0xA5, 0x4D, 0xFB, 0x9B, 0x5F, 0x45, 0x60, 0xDF, 0xB5, 0x36, 0x4E, 0x0E, 0x01, 0x05, 0x6A, 0x07, 0x36, 0x4A, 0x60, 0x5F, 0x85, 0x75, 0xF3, 0x84, 0x1D, 0x51, 0x7D, 0x07, 0x22, 0x06, 0x14, 0xCF, 0xC7, 0xCF, 0x98, 0x82, 0x5E, 0xE7, 0x20, 0xC5, 0x1C, 0xBC, 0x59, 0x16, 0x68, 0x14, 0x14, 0x68, 0x53, 0xFF, 0x11, 0x01, 0x67, 0x4C, 0xC1, 0xD2, 0x17, 0xB8, 0xD8, 0x03, 0x4C, 0xC7, 0x51, 0x5F, 0xE0, 0x20, 0x95, 0x61, 0x0D, 0x16};
    static unsigned long previous_millis = 0;

    if (millis() - previous_millis > 4000)
    {
        previous_millis = last_read = millis();
        memcpy(&receive_buffer, &test_data, sizeof(test_data));
        receive_buffer_index = sizeof(test_data) - 1;
    }
#endif

    if (receive_buffer_index > 0 && current_time - last_read > READ_TIMEOUT)
    {
        if (receive_buffer_index < 256)
        {
            Serial.println("Received packet with invalid size!");
            receive_buffer_index = 0;
            return;
        }

#ifdef DEBUG_OUTPUT
        for (int i = 0; i < 256; i++)
        {
            Serial.printf("%02X", receive_buffer[i]);
        }
#endif

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

#ifdef DEBUG_OUTPUT
        for (int i = 0; i < payload_length; i++)
        {
            Serial.printf("%02X", plaintext[i]);
        }
#endif

        // Decode data
        uint16_t current_position = DECODER_START_OFFSET;
        meterData meter_data;

        do
        {
            if (plaintext[current_position + OBIS_TYPE_OFFSET] != DATA_OCTET_STRING)
            {
                // For Sagemcom the Meter Number is bugged, so skip the garbage...
                if (current_position >= 229)
                {
                    break;
                }
                else
                {
                    Serial.print("Unsupported OBIS header type!");
                    receive_buffer_index = 0;
                    return;
                }
            }

            uint8_t data_length = plaintext[current_position + OBIS_LENGTH_OFFSET];

            if (data_length != 0x06)
            {
                // read timestamp
                if ((data_length == 0x0C) && (current_position == DECODER_START_OFFSET))
                {
                    uint16_t year;
                    uint8_t month, day, hour, minute, second;
                    int16_t utc_offset;

                    year = (plaintext[current_position + 2] << 8) + plaintext[current_position + 3];
                    month = plaintext[current_position + 4];
                    day = plaintext[current_position + 5];
                    hour = plaintext[current_position + 7];
                    minute = plaintext[current_position + 8];
                    second = plaintext[current_position + 9];

                    if (hour == 1 && minute == 00 && (second >= 0 && second <= 5))
                        wifi_reconnect = true;

                    sprintf(meter_data.timestamp_str, "%02u.%02u.%04u %02u:%02u:%02u", day, month, year, hour, minute, second);

                    // get utc offset in seconds
                    utc_offset = get_signed_short(plaintext[current_position + 11], plaintext[current_position + 12]) * 60 * -1;

                    // convert to unix timestamp
                    struct tm tm;
                    if (strptime(meter_data.timestamp_str, "%d.%m.%Y %H:%M:%S", &tm) != NULL)
                    {
                        meter_data.timestamp_unix = mktime(&tm) - utc_offset;
                        Serial.print("Unix Time ");
                        Serial.println(meter_data.timestamp_unix);
                    }
                    else
                    {
                        Serial.println("Invalid Timestamp");
                    }

                    Serial.print("Timestamp ");
                    Serial.print(meter_data.timestamp_str);
                    Serial.printf(" UTC %+03d:00\r\n", utc_offset / 3600);

                    current_position = 34;
                    data_length = plaintext[current_position + OBIS_LENGTH_OFFSET];
                }
                else if ((data_length == 0x0C) && (current_position > 225))
                {
                    uint8_t meterNumber[data_length];
                    memcpy(&meterNumber[0], &plaintext[current_position + 2], data_length);

                    Serial.print("Meter Number: ");
                    for (int i = 0; i < data_length; i++)
                    {
                        Serial.printf("%02X", meterNumber[i]);
                    }

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
                    meter_data.power_plus = float_value;
                    Serial.print("ActivePowerPlus ");
                    Serial.println(float_value);
                    break;

                case TYPE_ACTIVE_POWER_MINUS:
                    meter_data.power_minus = float_value;
                    Serial.print("ActivePowerMinus ");
                    Serial.println(float_value);
                    break;

                case TYPE_ACTIVE_ENERGY_PLUS:
                    meter_data.energy_plus = float_value + ACTIVE_ENERGY_PLUS_OFFSET;
                    Serial.print("ActiveEnergyPlus ");
                    Serial.println(float_value);
                    break;

                case TYPE_ACTIVE_ENERGY_MINUS:
                    meter_data.energy_minus = float_value;
                    Serial.print("ActiveEnergyMinus ");
                    Serial.println(float_value);
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
                    meter_data.voltage_l1 = float_value;
                    Serial.print("VoltageL1 ");
                    Serial.println(float_value);
                    break;

                case TYPE_VOLTAGE_L2:
                    meter_data.voltage_l2 = float_value;
                    Serial.print("VoltageL2 ");
                    Serial.println(float_value);
                    break;

                case TYPE_VOLTAGE_L3:
                    meter_data.voltage_l3 = float_value;
                    Serial.print("VoltageL3 ");
                    Serial.println(float_value);
                    break;

                case TYPE_CURRENT_L1:
                    meter_data.current_l1 = float_value;
                    Serial.print("CurrentL1 ");
                    Serial.println(float_value);
                    break;

                case TYPE_CURRENT_L2:
                    meter_data.current_l2 = float_value;
                    Serial.print("CurrentL2 ");
                    Serial.println(float_value);
                    break;

                case TYPE_CURRENT_L3:
                    meter_data.current_l3 = float_value;
                    Serial.print("CurrentL3 ");
                    Serial.println(float_value);
                    break;

                case TYPE_POWER_FACTOR:
                    meter_data.cos_phi = float_value;
                    Serial.print("PowerFactor ");
                    Serial.println(float_value);
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

        meter_data.rssi = WiFi.RSSI();
        // Read RHT sensor
        if (newMeasurementReady(rht_ready))
        {
            if (rht_ready)
            {
                if (fetchPeriodicTemperatureHumidity(temperature, humidity))
                {
                    meter_data.humidity = humidity;
                    meter_data.temperature = temperature;
                }
            }
        }

        // send the data
        uint32_t start_send_t = millis();
        submitToGraphite(GRAPHITE_CURRENT_L1, meter_data.current_l1, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_CURRENT_L2, meter_data.current_l2, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_CURRENT_L3, meter_data.current_l3, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_ACTIVE_POWER_PLUS, meter_data.power_plus, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_ACTIVE_ENERGY_PLUS, meter_data.energy_plus, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_POWER_FACTOR, meter_data.cos_phi, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_RSSI, meter_data.rssi, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_T, meter_data.temperature, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_RH, meter_data.humidity, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_VOLTAGE_L1, meter_data.voltage_l1, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_VOLTAGE_L2, meter_data.voltage_l2, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_VOLTAGE_L3, meter_data.voltage_l3, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_ACTIVE_POWER_MINUS, meter_data.power_minus, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_ACTIVE_ENERGY_MINUS, meter_data.energy_minus, meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_SEND_TIME, (float)(millis() - start_send_t), meter_data.timestamp_unix);
        submitToGraphite(GRAPHITE_PROC_TIME, (float)(start_send_t - current_time), meter_data.timestamp_unix);

        // update the display
        displayMeterData(&meter_data);

        if (wifi_reconnect)
        {
            wifi_reconnect = false;
            if (!startWiFi())
                ESP.restart();
        }
    }
}

void serial_dump()
{
    uint32_t sketch_size = ESP.getSketchSize();
    uint32_t sketch_space = ESP.getFreeSketchSpace();
    String sketch_MD5 = ESP.getSketchMD5();

    Serial.printf("Name: %s\r\n", HOSTNAME);
    float sketchPct = 100 * sketch_size / sketch_space;
    Serial.printf("Sketch Size: %i (total: %i, %.1f%% used)\r\n", sketch_size, sketch_space, sketchPct);
    Serial.printf("ESP sdk: %s\r\n", ESP.getSdkVersion());
    Serial.println("WiFi SSID: " + WiFi.SSID());
    Serial.println("WiFi IP address: " + WiFi.localIP().toString());

    int64_t sec = esp_timer_get_time() / 1000000;
    int64_t up_days = int64_t(floor(sec / 86400));
    int up_hours = int64_t(floor(sec / 3600)) % 24;
    int up_min = int64_t(floor(sec / 60)) % 60;
    int up_sec = sec % 60;

    Serial.printf("System up: %" PRId64 ":%02i:%02i:%02i (d:h:m:s)\r\n", up_days, up_hours, up_min, up_sec);
    Serial.printf("CPU Freq: %i MHz\r\n", ESP.getCpuFreqMHz());
    Serial.printf("Heap: %i, free: %i, min free: %i, max block: %i\r\n", ESP.getHeapSize(), ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
    if (psramFound())
    {
        Serial.printf("Psram: %i, free: %i, min free: %i, max block: %i\r\n", ESP.getPsramSize(), ESP.getFreePsram(), ESP.getMinFreePsram(), ESP.getMaxAllocPsram());
    }
    else
    {
        Serial.println("Psram: Not found");
    }
    if (SPIFFS.begin())
    {
        Serial.printf("Spiffs: %i, used: %i\r\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
    }
    else
    {
        Serial.println("Spiffs: Not found");
    }
    return;
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

int16_t get_signed_short(uint8_t msb, uint8_t lsb)
{
    // byteorder = big
    int16_t _msb = msb;
    if (_msb >= 128)
        _msb -= 256;
    return _msb * 256 + lsb;
}