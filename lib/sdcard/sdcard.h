#ifndef SDCARD_H_
#define SDCARD_H_

#include <FS.h>
#include <SD_MMC.h>

uint8_t setupSDCard(void);
uint64_t getFreeSDSpace(void);
uint8_t getOldestFile(char *oldest_file, const char *dir);
void deleteFile(const char *path);

#endif