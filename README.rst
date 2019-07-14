ESP32-NESEMU, a Nintendo Entertainment System emulator for the ESP32
====================================================================

This is a much-needed cleanup of the the quick and dirty port of Nofrendo, a Nintendo Entertainment System emulator (original by Espressif, https://github.com/espressif/esp32-nesemu).

I am improving on the PrettyEffect menu that MittisBootloop created as well.  His in-game menu is so nicely done I will do what I can to preserve that feature as I add more in-game stuff.  Currently I have added SRAM (SPIFFS) and SD Card support for save games as well as speed-adjustable turbo buttons.

Note: Video stretch modes cause a little slowness, but most games are still very playable now that I've optimized video rendering.  Sound has been improved significantly, though it does have a little noise due to the built-in DAC fighting for time to process sound buffers.  An external DAC could solve this in theory, but I don't have any plans to explore sound improvements at this time since it's rather passable now.  If you find any ways to improve it, I'd love to hear from you, just open an issue or send a pull request.

ESP32+Display+Battery in a NES Controller: https://youtu.be/-xrElh8Uz_s

Warning
-------

Standard caveat: It's open-source and you aren't paying me.  Pull requests are always welcome. :)

Currently I use a partition labeled "Rom" to map roms into memory.  This is because there is no ability to MMAP a SIPFFS file in the SDK at the time of this writing.  I instead have to dump the file in the the Rom partition so that I can mmap the whole partition.  This works really well, but it also will wear on that spot.  If you are worried, move it to the end of the partition map or something.

Compiling
---------

Use Platform.IO with the ESP32 platform installed, ESP-IDF framework also needs to be installed.  I use VSCode with the Platform.IO plugin but you can also use `pio` command line.

You should review and revise the sdkconfig.h file to match your specific board.  My file is configured for a Heltec Wifi Kit with some changes:

    * XTAL frequency set to 26 -- Serial console was going bonkers at the wrong speed until I changed this (for Heltec boards -- others like WROVER-KIT and similar work at the default setting, 40)
   
    * Bluetooth and Bluedroid not enabled to save RAM (I hope?)

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

PSX Controller
--------------

   PSX controller support works for PS1/PS2 controllers (no analog controller support though, just digital right now.)  Comment out or undefine the GPIO controller enabe setting as it overrides PSX settings.
   
   =======   ===========   =======================================
   PSX Pin   Description   Constant
   =======   ===========   =======================================
   --        Enable PSX    CONFIG_HW_CONTROLLER_PSX
   1         DAT / MISO    CONFIG_HW_PSX_DAT
   2         CMD / MOSI    CONFIG_HW_PSX_CMD
   3         Vibration     n/a (vibration not supported right now)
   4         GND
   5         VCC
   6         ATT / CS      CONFIG_HW_PSX_ATT
   7         CLK / SCK     CONFIG_HW_PSX_CLK
   =======   ===========   =======================================

Other mappings to PSX buttons are defined and managed in the lib/nofrendo/src/esp32/psxcontroller.c file.  Of main interest is this part:

    #define A_BUTTON PSX_CIRCLE

    #define B_BUTTON PSX_X

    #define TURBO_A_BUTTON PSX_TRIANGLE

    #define TURBO_B_BUTTON PSX_SQUARE

    #define MENU_BUTTON PSX_L1

    #define POWER_BUTTON PSX_R1

GPIO Controller
---------------

You can use this control option if you really love soldering.  Before going this route I recommend you support your local thrift store (HELLO GOODWILL!) to see if you can't get a PSX controller for $5 or something.

To enable GPIO control, define the CONFIG_HW_CONTROLLER_GPIO constant and undefine (comment out) CONFIG_HW_CONTROLLER_PSX.  Make sure whatever pins you use are not assigned to other functions, such as DAC or internal flash!

   ======   ===============================
   Button   GPIO CONSTANT
   ======   ===============================
   Enable   CONFIG_HW_CONTROLLER_GPIO
   Up       CONFIG_HW_GPIO_UP
   Down     CONFIG_HW_GPIO_DOWN
   Right    CONFIG_HW_GPIO_RIGHT
   Left     CONFIG_HW_GPIO_LEFT
   Select   CONFIG_HW_GPIO_SELECT
   Start    CONFIG_HW_GPIO_START
   B        CONFIG_HW_GPIO_B
   A        CONFIG_HW_GPIO_A
   TurboB   CONFIG_HW_GPIO_TURBO_B
   TurboA   CONFIG_HW_GPIO_TURBO_A
   Power    CONFIG_HW_GPIO_POWER *currently not working
   Menu     CONFIG_HW_GPIO_MENU
   ======   ===============================

Connect also 3.3V to the Buttons

Sound
-----

Connect one Speaker-Pin to GPIO 26 and the other one to GND

ROM
---

This includes no Roms. You'll have to flash your own Roms and modify the roms.txt according to your needs.
Don't change format used in roms.txt because you might cause the menu to load incorrectly.  Review the file for further instructions.

For SPIFFS (SRAM), Use the Platform.IO task for uploading the /data folder into the SPIFFS volume, which should contain roms.txt and other rom files.

SD Card settings
----------------

You have the option to enable SD Card support (see the platformio.ini file) via "CONFIG_SD_CARD".  If you define this variable then also assign the pins for the SPI bus as well.  If you do not define this option it will default to SPIFFS on the SRAM, which requires re-flashing the partition.  SD Card support, on the other hand, just requires putting the "roms.txt" file and roms in the root folder of the SD Card.

   ======  ===========  ===============================
   SD_PIN  DESCRIPTION  CONSTANT
   ======  ===========  ===============================
   --      Enable SD    CONFIG_SD_CARD
   1       CS           CONFIG_SD_CS
   2       MOSI         CONFIG_SD_MOSI
   3       GND
   4       VCC
   5       Clock/SCK    CONFIG_SD_SCK
   6       GND
   7       MISO         CONFIG_SD_MISO
   ======  ===========  ===============================


Adjusting in-game settings
--------------------------

After booting a game, press the menu button for the in-game menu.  Press the button again to close the menu after making any changes.

Copyright
---------

Code in this repository is Copyright (C) 2016 Espressif Systems, licensed under the Apache License 2.0 as described in the file LICENSE. Code in the components/nofrendo is Copyright (c) 1998-2000 Matthew Conte (matt@conte.com) and licensed under the GPLv2.
Any changes in this repository are otherwise presented to you copyright myself and lisensed under the same Apache 2.0 license as the Espressif Systems repository.
