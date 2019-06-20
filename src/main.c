#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "psxcontroller.h"
#include "nofrendo.h"
#include "menu.h"
#include "esp_bt.h"

#define READ_BUFFER_SIZE 64

int selectedRomIdx;
char *selectedRomFilename;

char *osd_getromdata()
{
	const esp_partition_t *part;
	char *romdata;
	spi_flash_mmap_handle_t handle;
	esp_err_t err;

	// Locate our target ROM partition where the file will be loaded
	part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 0xFF, "rom");
	if (part == 0)
		printf("Couldn't find rom partition!\n");

	// Open the file
	printf("Reading rom from %s\n", selectedRomFilename);
	FILE *rom = fopen(selectedRomFilename, "r");
	long fileSize = -1;
	if (!rom)
	{
		printf("Could not read %s\n", selectedRomFilename);
		exit(1);
	}

	// First figure out how large the file is
	fseek(rom, 0L, SEEK_END);
	fileSize = ftell(rom);
	rewind(rom);
	if (fileSize > part->size)
	{
		printf("Rom is too large!  Limit is %dk; Rom file size is %dkb\n", 
		(int)(part->size / 1024), 
		(int)(fileSize / 1024));
		exit(1);
	}

	// Copy the file contents into EEPROM memory
	char buffer[READ_BUFFER_SIZE];
	int offset = 0;
	while (fread(buffer, 1, READ_BUFFER_SIZE, rom) > 0)
	{
		if ((offset % 4096) == 0)
			esp_partition_erase_range(part, offset, 4096);
		esp_partition_write(part, offset, buffer, READ_BUFFER_SIZE);
		offset += READ_BUFFER_SIZE;
	}
	fclose(rom);
	// free(buffer);
	printf("Loaded %d bytes into ROM memory\n", offset);

	// Memory-map the ROM partition, which results in 'data' pointer changing to memory-mapped location
	err = esp_partition_mmap(part, 0, fileSize, SPI_FLASH_MMAP_DATA, (const void **)&romdata, &handle);
	if (err != ESP_OK)
		printf("Couldn't map rom partition!\n");
	printf("Initialized. ROM@%p\n", romdata);

	return (char *)romdata;
}

void esp_wake_deep_sleep()
{
	esp_restart();
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
	return ESP_OK;
}

esp_err_t registerSpiffs()
{
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 32,
		.format_if_mount_failed = false};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			printf("Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			printf("Failed to find SPIFFS partition");
		}
		else
		{
			printf("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
	}
	else
	{
		struct stat st;
		if (stat(ROM_LIST, &st) != 0)
		{
			printf("Cannot locate rom list in %s", ROM_LIST);
		}
	}
	return ret;
}

int app_main(void)
{
	// esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
	if (registerSpiffs() != ESP_OK)
	{
		exit(1);
	}
	psxcontrollerInit();
	selectedRomFilename = runMenu();
	printf("NoFrendo start!\n");
	nofrendo_main(0, NULL);
	printf("NoFrendo died? WtF?\n");
	asm("break.n 1");
	return 0;
}
