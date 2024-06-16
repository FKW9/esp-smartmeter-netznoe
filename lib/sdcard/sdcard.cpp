#include "sdcard.h"

uint8_t setupSDCard(void)
{
	if (!SD_MMC.begin())
	{
		Serial.println("Card Mount Failed");
		return 0;
	}
	uint8_t cardType = SD_MMC.cardType();

	if (cardType == CARD_NONE)
	{
		Serial.println("No SD_MMC card attached");
		return 0;
	}

	Serial.print("SD_MMC Card Type: ");
	if (cardType == CARD_MMC)
	{
		Serial.println("MMC");
	}
	else if (cardType == CARD_SD)
	{
		Serial.println("SDSC");
	}
	else if (cardType == CARD_SDHC)
	{
		Serial.println("SDHC");
	}
	else
	{
		Serial.println("UNKNOWN");
	}

	uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
	Serial.printf("Card Size: %lluMB\n", cardSize);
	Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
	Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
	return 1;
}

uint64_t getFreeSDSpace(void)
{
	return (SD_MMC.totalBytes() / (1024 * 1024)) - (SD_MMC.usedBytes() / (1024 * 1024));
}

/**
 * @brief Get the Oldest File. Each file must be in the format "/YYYY_MM.CSV"
 *
 * @param oldest_file char array in which the oldest file path is stored
 * @param dir directory to search in
 * @return uint8_t 1 on success
 */
uint8_t getOldestFile(char *oldest_file, const char *dir)
{
	File root = SD_MMC.open(dir);
	if (!root)
	{
		Serial.println("Failed to open directory");
		return 0;
	}
	if (!root.isDirectory())
	{
		Serial.println("Not a directory");
		return 0;
	}

	char m_buf[3];
	char y_buf[5];
	int cmonth, cyear, month, year;
	cyear = 9999;
	cmonth = 99;

	File file = root.openNextFile();

	strcpy(oldest_file, "/9999_99.CSV");

	while (file)
	{
		if (!file.isDirectory() && strstr(file.name(), "System") == NULL)
		{
			// year and month of new file
			memcpy(&y_buf[0], &file.name()[1], 4);
			sscanf(y_buf, "%d", &year);
			memcpy(&m_buf[0], &file.name()[6], 2);
			sscanf(m_buf, "%d", &month);

			if ((year < cyear) || (year == cyear && month <= cmonth))
			{
				strcpy(oldest_file, file.name());
				cyear = year;
				cmonth = month;
			}
		}
		file = root.openNextFile();
	}
	return 1;
}

void deleteFile(const char *path)
{
	if (SD_MMC.remove(path))
		Serial.println("File deleted");
	else
		Serial.println("Delete failed");
}