#pragma once
#include <stdint.h>
#include "esp_err.h"

/**
 * @brief running intro and menu (choose rom/game)
 *
 * @return - integer for choosing game partition
 */
#define ROM_LIST "/spiffs/roms.txt"
#define FILENAME_LENGTH 18

char* runMenu();
void setBr(int bright);
void freeMenuResources();

typedef struct menuEntry
{
	int entryNumber;
	char icon;
	char name[FILENAME_LENGTH+1];
	char fileName[FILENAME_LENGTH+1];
} MenuEntry;
int entryCount;
MenuEntry *menuEntries;
