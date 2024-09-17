#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_legacy.h"
#include "esp_event_loop.h"
#include "esp_partition.h"
#include "esp_err.h"
// #include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
// #include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdspi_host_legacy.h"
#include "sdmmc_cmd.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "psxcontroller.h"
#include "nofrendo.h"
#include "menu.h"
#include "spi_flash_mmap.h"
#include "sys/stat.h"
// #include "esp_bt.h"

#define NOFRENDO_DEBUG 1
#define READ_BUFFER_SIZE 64

#define ASSERT_ESP_OK(returnCode, message)                          \
	if (returnCode != ESP_OK)                                       \
	{                                                               \
		printf("%s. (%s)\n", message, esp_err_to_name(returnCode)); \
		return returnCode;                                          \
	}

// #define SKIP_MENU
int selectedRomIdx;
#ifdef SKIP_MENU
char *selectedRomFilename = "/spiffs/unknown.nes";
#else
char *selectedRomFilename;
#endif

char *osd_getromdata()
{
	char *romdata;
	spi_flash_mmap_handle_t handle;
	esp_err_t err;

	// Locate our target ROM partition where the file will be loaded
	esp_partition_t *part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 0xFF, "rom");
	if (part == 0)
		printf("Couldn't find rom partition!\n");

#ifndef SKIP_MENU
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
#else
	size_t fileSize = part->size;
#endif

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
esp_err_t registerSdCard()
{
	esp_err_t ret;
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	// host.command_timeout_ms=200;
	// host.max_freq_khz = SDMMC_FREQ_PROBING;
	sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
	slot_config.gpio_miso =  CONFIG_SD_MISO;
	slot_config.gpio_mosi =  CONFIG_SD_MOSI;
	slot_config.gpio_sck =   CONFIG_SD_SCK;
	slot_config.gpio_cs =    CONFIG_SD_CS;
	slot_config.dma_channel = 2; //2
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = 2};

	sdmmc_card_t *card;
	//!TODO: Evil hack... don't use spiffs here!
	ret = esp_vfs_fat_sdmmc_mount("/spiffs", &host, &slot_config, &mount_config, &card);
	ASSERT_ESP_OK(ret, "Failed to mount SD card");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
	
	return ESP_OK;

/*
	---------------------------------------------
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.slot = HSPI_HOST;
	host.max_freq_khz = SPI_MASTER_FREQ_8M;
	// host.max_freq_khz = SDMMC_FREQ_PROBING;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = CONFIG_SD_MISO;
    slot_config.gpio_mosi = CONFIG_SD_MOSI;
    slot_config.gpio_sck  = CONFIG_SD_SCK;
    slot_config.gpio_cs   = CONFIG_SD_CS;
	// ret = sdspi_host_init(host.slot, &slot_config);
	// ASSERT_ESP_OK(ret, "Failed to init host")
	// printf("SPI Host init completed (HSPI)\n");

	// sdspi_host_init_slot(host.slot, &slot_config);
	// ASSERT_ESP_OK(ret, "Failed to init slot")
	// printf("SPI Slot init completed (HSPI)\n");
	
	// gpio_set_pull_mode(CONFIG_SD_MISO, GPIO_PULLDOWN_ENABLE | GPIO_PULLUP_ENABLE);
	gpio_set_pull_mode(CONFIG_SD_MISO, GPIO_PULLDOWN_ONLY);
	gpio_set_pull_mode(CONFIG_SD_MOSI, GPIO_PULLUP_ONLY);
	gpio_set_pull_mode(CONFIG_SD_SCK, GPIO_PULLUP_ONLY);
	


	 esp_vfs_fat_sdmmc_mount_config_t mount_config = {		
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

	sdmmc_card_t* card;
	//!TODO: Evil hack... don't use spiffs here!
    ret = esp_vfs_fat_sdmmc_mount("/spiffs", &host, &slot_config, &mount_config, &card);
	ASSERT_ESP_OK(ret, "Failed to mount SD card")
	return ESP_OK;
	*/
}

esp_err_t registerSpiffs()
{
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 32,
		.format_if_mount_failed = false};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	ASSERT_ESP_OK(ret, "Failed to mount SPIFFS partition.");
	typedef struct stat struc_date;
	struc_date st;
	if (stat(ROM_LIST, &st) != 0)
	{
		printf("Cannot locate rom list in %s", ROM_LIST);
	}
	return ret;
}

int app_main(void)
{
// esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
#ifdef CONFIG_SD_CARD
	ASSERT_ESP_OK(registerSdCard(), "Unable to register SD Card");
#else
	ASSERT_ESP_OK(registerSpiffs(), "Unable to register SPIFFS");
#endif
	psxcontrollerInit();
#ifndef SKIP_MENU
	selectedRomFilename = runMenu();
#endif
	printf("NoFrendo start!\n");
	nofrendo_main(0, NULL);
	printf("NoFrendo died? WtF?\n");
	asm("break.n 1");
	return 0;
}
