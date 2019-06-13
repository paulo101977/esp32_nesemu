// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include "psxcontroller.h"
#include "sdkconfig.h"
#include "pretty_effect.h"
#include "esp_deep_sleep.h"

#define PSX_CLK CONFIG_HW_PSX_CLK
#define PSX_DAT CONFIG_HW_PSX_DAT
#define PSX_ATT CONFIG_HW_PSX_ATT
#define PSX_CMD CONFIG_HW_PSX_CMD

#define DELAY() asm("nop; nop; nop; nop;nop; nop; nop; nop;nop; nop; nop; nop;nop; nop; nop; nop;")

#if CONFIG_HW_CONTROLLER_PSX || CONFIG_HW_CONTROLLER_GPIO

int volume, bright;
int inpDelay;
bool shutdown;

/* Sends and receives a byte from/to the PSX controller using SPI */
static int psxSendRecv(int send)
{
	int x;
	int ret = 0;
	volatile int delay;

#if 0
	while(1) {
		GPIO.out_w1ts=(1<<PSX_CMD);
		GPIO.out_w1ts=(1<<PSX_CLK);
		GPIO.out_w1tc=(1<<PSX_CMD);
		GPIO.out_w1tc=(1<<PSX_CLK);
	}
#endif

	GPIO.out_w1tc = (1 << PSX_ATT);
	for (delay = 0; delay < 100; delay++)
		;
	for (x = 0; x < 8; x++)
	{
		if (send & 1)
		{
			GPIO.out_w1ts = (1 << PSX_CMD);
		}
		else
		{
			GPIO.out_w1tc = (1 << PSX_CMD);
		}
		DELAY();
		for (delay = 0; delay < 100; delay++)
			;
		GPIO.out_w1tc = (1 << PSX_CLK);
		for (delay = 0; delay < 100; delay++)
			;
		GPIO.out_w1ts = (1 << PSX_CLK);
		ret >>= 1;
		send >>= 1;
		if (GPIO.in & (1 << PSX_DAT))
			ret |= 128;
	}
	return ret;
}

bool showMenu;

static void psxDone()
{
	DELAY();
	GPIO_REG_WRITE(GPIO_OUT_W1TS_REG, (1 << PSX_ATT));
}

bool getShowMenu()
{
	return showMenu;
}

//Bit0 Bit1 Bit2 Bit3 Bit4 Bit5 Bit6 Bit7
//SLCT           STRT UP   RGHT DOWN LEFT
//Bit8 Bit9 Bt10 Bt11 Bt12 Bt13 Bt14 Bt15
//L2   R2   L1   R1    /\   O    X   |_|
#define PSX_SELECT 1
#define PSX_START 1 << 3
#define PSX_UP 1 << 4
#define PSX_RIGHT 1 << 5
#define PSX_DOWN 1 << 6
#define PSX_LEFT 1 << 7
#define PSX_L2 1 << 8
#define PSX_R2 1 << 9
#define PSX_L1 1 << 10
#define PSX_R1 1 << 11
#define PSX_TRIANGLE 1 << 12
#define PSX_CIRCLE 1 << 13
#define PSX_X 1 << 14
#define PSX_SQUARE 1 << 15
#define A_BUTTON PSX_CIRCLE
#define B_BUTTON PSX_X
#define MENU_BUTTON PSX_TRIANGLE
#define BRIGHTNESS_BUTTON PSX_SQUARE

bool isSelectPressed(int ctl)
{
	return !(ctl & PSX_SELECT);
}
bool isStartPressed(int ctl)
{
	return !(ctl & PSX_START);
}
bool isUpPressed(int ctl)
{
	return !(ctl & PSX_UP);
}
bool isRightPressed(int ctl)
{
	return !(ctl & PSX_RIGHT);
}
bool isDownPressed(int ctl)
{
	return !(ctl & PSX_DOWN);
}
bool isLeftPressed(int ctl)
{
	return !(ctl & PSX_LEFT);
}
bool isAPressed(int ctl)
{
	return !(ctl & A_BUTTON);
}
bool isBPressed(int ctl)
{
	return !(ctl & B_BUTTON);
}
bool isMenuPressed(int ctl)
{
	return !(ctl & MENU_BUTTON);
}
bool isBrightnessPressed(int ctl)
{
	return !(ctl & BRIGHTNESS_BUTTON);
}
bool isAnyDirectionPressed(int ctl) {
	return isUpPressed(ctl) || isDownPressed(ctl) || isLeftPressed(ctl) || isRightPressed(ctl);
}

bool isAnyActionPressed(int ctl) {
	return isStartPressed(ctl) || isSelectPressed(ctl) || isMenuPressed(ctl) || isBrightnessPressed(ctl);
}

bool isAnyPressed(int ctl)
{
	return isAnyDirectionPressed(ctl) || isAnyActionPressed(ctl) || isAPressed(ctl) || isBPressed(ctl);
}

int psxReadInput()
{
	int b2b1 = 65535;
	if (inpDelay > 0)
		inpDelay--;
#ifndef CONFIG_HW_CONTROLLER_GPIO
	int b1, b2;
	psxSendRecv(0x01);		//wake up
	psxSendRecv(0x42);		//get data
	psxSendRecv(0xff);		//should return 0x5a
	b1 = psxSendRecv(0xff); //buttons byte 1
	b2 = psxSendRecv(0xff); //buttons byte 2
	psxDone();
	b2b1 = (b2 << 8) | b1;
#else
	if (gpio_get_level(CONFIG_HW_GPIO_UP) == 1)
		b2b1 -= PSX_UP;
	if (gpio_get_level(CONFIG_HW_GPIO_DOWN) == 1)
		b2b1 -= PSX_DOWN;
	if (gpio_get_level(CONFIG_HW_GPIO_RIGHT) == 1)
		b2b1 -= PSX_RIGHT;
	if (gpio_get_level(CONFIG_HW_GPIO_LEFT) == 1)
		b2b1 -= PSX_LEFT;
	if (gpio_get_level(CONFIG_HW_GPIO_SELECT) == 1)
		b2b1 -= PSX_SELECT;
	if (gpio_get_level(CONFIG_HW_GPIO_START) == 1)
		b2b1 -= PSX_START;
	if (gpio_get_level(CONFIG_HW_GPIO_B) == 1)
		b2b1 -= B_BUTTON;
	if (gpio_get_level(CONFIG_HW_GPIO_A) == 1)
		b2b1 -= A_BUTTON;
	if (gpio_get_level(CONFIG_HW_GPIO_MENU) == 1)
		b2b1 -= MENU_BUTTON;
	if (gpio_get_level(CONFIG_HW_GPIO_BRIGHTNESS) == 1)
		b2b1 -= BRIGHTNESS_BUTTON;
#endif
	if (showMenu && inpDelay == 0)
	{
		if (isUpPressed(b2b1) && volume < 4)
			volume++;
		if (isDownPressed(b2b1) && volume > 0)
			volume--;
		if (isRightPressed(b2b1) && bright < 4)
			bright++;
		if (isLeftPressed(b2b1) && bright > 0)
			bright--;
		if (isAPressed(b2b1))
			setYStretch(1 - getYStretch());
		if (isBPressed(b2b1))
			setXStretch(1 - getXStretch());
		if (isAnyPressed(b2b1))
			inpDelay = 15;
	}
	else
	{
		if (isBrightnessPressed(b2b1) && inpDelay > 0)
			bright = -1;
		if (bright < 0)
		{
			// ! todo: Implement sleep mode for PSX
			/*
	if (bright == -1 && inpDelay == 0)
	{
		esp_sleep_enable_timer_wakeup(1000 * 100);
		vTaskDelay(100);
		esp_deep_sleep_start();
	}

	if (bright == -1 && inpDelay > 100)
	{
		bright = -2;
		shutdown = 1;
		esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
		gpio_pullup_dis(CONFIG_HW_GPIO_BRIGHTNESS);
		gpio_pulldown_en(CONFIG_HW_GPIO_BRIGHTNESS);
		esp_deep_sleep_enable_ext0_wakeup(CONFIG_HW_GPIO_BRIGHTNESS, 1);

		vTaskDelay(1500);
		esp_deep_sleep_start();
	}			
			*/
			bright = 4;
		}
		inpDelay += 2;
	}
	if (isMenuPressed(b2b1) && inpDelay > 0)
	{
		showMenu = 1 - showMenu;
		inpDelay = 15;
	}
	return b2b1;
}

int getBright()
{
	return bright;
}

int getVolume()
{
	return volume;
}

bool getShutdown()
{
	return shutdown;
}

void psxcontrollerInit()
{
	printf("Game controller initalizing\n");
#if CONFIG_HW_CONTROLLER_GPIO
	initGPIO(CONFIG_HW_GPIO_START);
	initGPIO(CONFIG_HW_GPIO_SELECT);
	initGPIO(CONFIG_HW_GPIO_UP);
	initGPIO(CONFIG_HW_GPIO_DOWN);
	initGPIO(CONFIG_HW_GPIO_LEFT);
	initGPIO(CONFIG_HW_GPIO_RIGHT);
	initGPIO(CONFIG_HW_GPIO_B);
	initGPIO(CONFIG_HW_GPIO_A);
	initGPIO(CONFIG_HW_GPIO_MENU);
	initGPIO(CONFIG_HW_GPIO_BRIGHTNESS);
	printf("GPIO Control initated\n");
#else
	volatile int delay;
	int t;
	showMenu = 0;
	shutdown = 0;
	gpio_config_t gpioconf[2] = {
		{.pin_bit_mask = (1 << PSX_CLK) | (1 << PSX_CMD) | (1 << PSX_ATT),
		 .mode = GPIO_MODE_OUTPUT,
		 .pull_up_en = GPIO_PULLUP_DISABLE,
		 .pull_down_en = GPIO_PULLDOWN_DISABLE,
		 .intr_type = GPIO_PIN_INTR_DISABLE},
		{.pin_bit_mask = (1 << PSX_DAT),
		 .mode = GPIO_MODE_INPUT,
		 .pull_up_en = GPIO_PULLUP_ENABLE,
		 .pull_down_en = GPIO_PULLDOWN_DISABLE,
		 .intr_type = GPIO_PIN_INTR_DISABLE}};
	gpio_config(&gpioconf[0]);
	gpio_config(&gpioconf[1]);

	//Send a few dummy bytes to clean the pipes.
	psxSendRecv(0);
	psxDone();
	for (delay = 0; delay < 500; delay++)
		DELAY();
	psxSendRecv(0);
	psxDone();
	for (delay = 0; delay < 500; delay++)
		DELAY();
	//Try and detect the type of controller, so we can give the user some diagnostics.
	psxSendRecv(0x01);
	t = psxSendRecv(0x00);
	psxDone();
	if (t == 0 || t == 0xff)
	{
		printf("No PSX/PS2 controller detected (0x%X). You will not be able to control the game.\n", t);
	}
	else
	{
		printf("PSX controller type 0x%X\n", t);
	}
#endif
	inpDelay = 0;
	volume = 0;
	bright = 2;
}

#else

int psxReadInput()
{
	return 0xFFFF;
}

void psxcontrollerInit()
{
	printf("PSX controller disabled in menuconfig; no input enabled.\n");
}

#endif