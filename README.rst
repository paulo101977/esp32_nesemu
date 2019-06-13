ESP32-NESEMU, a Nintendo Entertainment System emulator for the ESP32
====================================================================

This is a much-needed cleanup of the the quick and dirty port of Nofrendo, a Nintendo Entertainment System emulator (original by Espressif, https://github.com/espressif/esp32-nesemu).

I am improving on the PrettyEffect menu that MittisBootloop created as well.  His in-game menu is so nicely done I will do what I can to preserve that feature as I add more in-game stuff.

Caution: Fullscreen can cause graphical glitches and it has bad sound.

ESP32+Display+Battery in a NES Controller: https://youtu.be/-xrElh8Uz_s

Warning
-------

Standard caveat: It's open-source and you aren't paying me.  Pull requests are always welcome. :)

Currently I use a partition labeled "Rom" to map roms into memory.  This is because there is no MMAP to a SIPFFS file in the SDK at the time of this writing.  I instead have to dump the file in the the Rom partition so that I can mmap the whole partition.  This works really well, but it also will wear on that spot.  If you are worried, move it to the end of the partition map or something.

Compiling
---------

Use Platform.IO with the ESP32 platform installed, ESP-IDF framework also needs to be installed.  I use VSCode with the Platform.IO plugin but you can also use `pio` command line.

You should review and revise the sdkconfig.h file to match your specific board.  My file is configured for a Heltec Wifi Kit with some changes:
- XTAL frequency set to 26 -- Serial console was going bonkers at the wrong speed until I changed this.
- Bluetooth and Bluedroid not enabled to save RAM (I hope?)

Display
-------

To display the NES output, please connect a 320x240 ili9341-based SPI display to the ESP32 in this way:

    =====  ====== ========================
    Pin    GPIO   Constant
    =====  ====== ========================
    MISO   25     CONFIG_HW_LCD_MISO_GPIO
    MOSI   23     CONFIG_HW_LCD_MOSI_GPIO
    CLK    19     CONFIG_HW_LCD_CLK_GPIO
    CS     22     CONFIG_HW_LCD_CS_GPIO
    DC     21     CONFIG_HW_LCD_DC_GPIO
    RST    18     CONFIG_HW_LCD_RESET_GPIO
    LED    27     CONFIG_HW_LCD_BL_GPIO
    =====  ====== ========================

Also connect the power supply and ground. For now, the LCD is controlled using a SPI peripheral, fed using the 2nd CPU. This is less than ideal; feeding the SPI controller using DMA is better, but was left out due to this being a proof of concept.

GPIO Controller
---------------
   ======   ===============================
   Button   GPIO CONSTANT
   ======   ===============================
   UP       CONFIG_HW_GPIO_UP
   DOWN     CONFIG_HW_GPIO_DOWN
   RIGHT    CONFIG_HW_GPIO_RIGHT
   LEFT     CONFIG_HW_GPIO_RIGHT
   SELECT   CONFIG_HW_GPIO_SELECT
   START    CONFIG_HW_GPIO_START
   B        CONFIG_HW_GPIO_B
   A        CONFIG_HW_GPIO_A
   ON/OFF   TBD
   MENU     CONFIG_HW_GPIO_MENU
   ======   ===============================

Connect also 3.3V to the Buttons

Sound
-----

Connect one Speaker-Pin to GPIO 26 and the other one to GND

ROM
---

This includes no Roms. You'll have to flash your own Roms and modify the roms.txt according to your needs.
Don't change format used in roms.txt because you might cause the menu to load incorrectly.  Review the file for further instructions.

Use the Platform.IO task for uploading the SPIFFS volume in order to upload roms.txt and rom files.

Copyright
---------

Code in this repository is Copyright (C) 2016 Espressif Systems, licensed under the Apache License 2.0 as described in the file LICENSE. Code in the components/nofrendo is Copyright (c) 1998-2000 Matthew Conte (matt@conte.com) and licensed under the GPLv2.
Any changes in this repository are otherwise presented to you copyright myself and lisensed under the same Apache 2.0 license as the Espressif Systems repository.
