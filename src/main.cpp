#include <Arduino.h>
#include <mbedtls/gcm.h>
#include "dlms.h"
#include "obis.h"
#include "graphite.h"
#include "display.h"
#include "hte501.h"
#include "sdcard.h"
#include "../config.h"

// General variables
char base_version[] = "2.0";
char my_ver[] PROGMEM = __DATE__ " @ " __TIME__;

// Variables for DLMS decoding
uint32_t last_read = 0;                      // Timestamp when data was last read
uint16_t receive_buffer_index = 0;           // Current position in the receive buffer
uint8_t receive_buffer[RECEIVE_BUFFER_SIZE]; // Stores the received data
mbedtls_gcm_context aes;

// Variables for RHT sensor
float temperature = 0, humidity = 0;
uint8_t rht_ready = 0;

// Function prototypes
uint32_t swap_uint32(uint32_t val);
uint16_t swap_uint16(uint16_t val);
void serial_dump();

void setup()
{
    setupDisplay();
    setupSensor();
    startPeriodicMeasurement();

    // Debug port
    Serial.begin(115200);
    Serial.println("Booting...");

    // MBus input from MBus Slave Click
    Serial2.begin(2400, SERIAL_8E1);
    Serial2.setRxBufferSize(RECEIVE_BUFFER_SIZE);
    Serial2.setTimeout(2);

    setupWiFi();
    setupSDCard();
    displaySDCardStatus();
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
        obisData meterData;

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
                    uint8_t month, day, hour, minute, second;

                    year = (plaintext[current_position + 2] << 8) + plaintext[current_position + 3];
                    month = plaintext[current_position + 4];
                    day = plaintext[current_position + 5];
                    hour = plaintext[current_position + 7];
                    minute = plaintext[current_position + 8];
                    second = plaintext[current_position + 9];

                    sprintf(meterData.timestamp_str, "%02u.%02u.%04u %02u:%02u:%02u", day, month, year, hour, minute, second);

                    // convert to unix timestamp for graphite
                    struct tm tm;
                    if (strptime(meterData.timestamp_str, "%d.%m.%Y %H:%M:%S", &tm) != NULL)
                    {
                        meterData.timestamp_unix = mktime(&tm) - 7200;
                        Serial.print("Unix Time: ");
                        Serial.println(meterData.timestamp_unix);
                    }
                    else
                    {
                        Serial.println("Invalid Timestamp");
                        receive_buffer_index = 0;
                        return;
                    }

                    Serial.print("Timestamp: ");
                    Serial.println(meterData.timestamp_str);

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
                    meterData.power_plus = float_value;
                    Serial.print("ActivePowerPlus ");
                    Serial.println(float_value);
                    break;

                case TYPE_ACTIVE_POWER_MINUS:
                    meterData.power_minus = float_value;
                    Serial.print("ActivePowerMinus ");
                    Serial.println(float_value);
                    break;

                case TYPE_ACTIVE_ENERGY_PLUS:
                    meterData.energy_plus = float_value;
                    Serial.print("ActiveEnergyPlus ");
                    Serial.println(float_value);
                    break;

                case TYPE_ACTIVE_ENERGY_MINUS:
                    meterData.energy_minus = float_value;
                    Serial.print("ActiveEnergyMinus ");
                    Serial.println(float_value);
                    break;
                }
                break;

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
                    meterData.voltage_l1 = float_value;
                    Serial.print("VoltageL1 ");
                    Serial.println(float_value);
                    break;

                case TYPE_VOLTAGE_L2:
                    meterData.voltage_l2 = float_value;
                    Serial.print("VoltageL2 ");
                    Serial.println(float_value);
                    break;

                case TYPE_VOLTAGE_L3:
                    meterData.voltage_l3 = float_value;
                    Serial.print("VoltageL3 ");
                    Serial.println(float_value);
                    break;

                case TYPE_CURRENT_L1:
                    meterData.current_l1 = float_value;
                    Serial.print("CurrentL1 ");
                    Serial.println(float_value);
                    break;

                case TYPE_CURRENT_L2:
                    meterData.current_l2 = float_value;
                    Serial.print("CurrentL2 ");
                    Serial.println(float_value);
                    break;

                case TYPE_CURRENT_L3:
                    meterData.current_l3 = float_value;
                    Serial.print("CurrentL3 ");
                    Serial.println(float_value);
                    break;

                case TYPE_POWER_FACTOR:
                    meterData.cos_phi = float_value;
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

        // send the data
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_VOLTAGE_L1, meterData.voltage_l1);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_VOLTAGE_L2, meterData.voltage_l2);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_VOLTAGE_L3, meterData.voltage_l3);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_CURRENT_L1, meterData.current_l1);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_CURRENT_L2, meterData.current_l2);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_CURRENT_L3, meterData.current_l3);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_POWER_FACTOR, meterData.cos_phi);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_ACTIVE_POWER_PLUS, meterData.power_plus);
        submitToGraphite(meterData.timestamp_unix, GRAPHITE_ACTIVE_ENERGY_PLUS, meterData.energy_plus);
        // submitToGraphite(meterData.timestamp_unix, GRAPHITE_ACTIVE_POWER_MINUS, meterData.power_minus);
        // submitToGraphite(meterData.timestamp_unix, GRAPHITE_ACTIVE_ENERGY_MINUS, meterData.energy_minus);
        displayMeterData(&meterData);

        // After DLMS decoding is done, send some other stuff
        submitToGraphite(-1, GRAPHITE_RSSI, WiFi.RSSI());
        displayRSSI();

        // Read RHT sensor
        if (newMeasurementReady(rht_ready)){
            if (rht_ready){
                if (fetchPeriodicTemperatureHumidity(temperature, humidity))
                {
                    submitToGraphite(-1, GRAPHITE_RH, humidity);
                    submitToGraphite(-1, GRAPHITE_T, temperature);

                    #ifdef DEBUG
                        tft.setCursor(0, 80);
                        tft.print(temperature);
                        tft.print(" - ");
                        tft.println(humidity);
                    #endif
                }
            }
        }

        #ifdef DEBUG
            static uint16_t valid_packet_cnt = 0;
            valid_packet_cnt++;
            int16_t x = tft.getCursorX();
            int16_t y = tft.getCursorY();
            tft.setTextColor(TFT_BLUE, TFT_WHITE, true);
            tft.printf("Received valid data! (%d)", valid_packet_cnt);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setCursor(x, y);
        #endif
    }
}


void serial_dump() {

    uint32_t sketch_size = ESP.getSketchSize();
    uint32_t sketch_space = ESP.getFreeSketchSpace();
    String sketch_MD5 = ESP.getSketchMD5();

    Serial.printf("Name: %s\r\n", HOSTNAME);
    // if (haveTime) {
    //     Serial.print("Time: ");
    //     printLocalTime(true);
    // }
    Serial.printf("Firmware: %s (base: %s)\r\n", my_ver, base_version);
    float sketchPct = 100 * sketch_size / sketch_space;
    Serial.printf("Sketch Size: %i (total: %i, %.1f%% used)\r\n", sketch_size, sketch_space, sketchPct);
    Serial.printf("ESP sdk: %s\r\n", ESP.getSdkVersion());
    Serial.printf("WiFi SSID: %s\r\n", WIFI_SSID);
    Serial.println("WiFi IP address: " + WiFi.localIP().toString());

    int64_t sec = esp_timer_get_time() / 1000000;
    int64_t up_days = int64_t(floor(sec/86400));
    int up_hours = int64_t(floor(sec/3600)) % 24;
    int up_min = int64_t(floor(sec/60)) % 60;
    int up_sec = sec % 60;

    Serial.printf("System up: %" PRId64 ":%02i:%02i:%02i (d:h:m:s)\r\n", up_days, up_hours, up_min, up_sec);
    Serial.printf("CPU Freq: %i MHz\r\n", ESP.getCpuFreqMHz());
    Serial.printf("Heap: %i, free: %i, min free: %i, max block: %i\r\n", ESP.getHeapSize(), ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
    if(psramFound()) {
        Serial.printf("Psram: %i, free: %i, min free: %i, max block: %i\r\n", ESP.getPsramSize(), ESP.getFreePsram(), ESP.getMinFreePsram(), ESP.getMaxAllocPsram());
    } else {
        Serial.printf("Psram: Not found.\r\n");
    }
    // Filesystems
    // if (filesystem && (SPIFFS.totalBytes() > 0)) {
    //     Serial.printf("Spiffs: %i, used: %i\r\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
    // } else {
    //     Serial.printf("Spiffs: No filesystem found, please check your board configuration.\r\n");
    //     Serial.printf("- Saving and restoring camera settings will not function without this.\r\n");
    // }
    Serial.println();
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
