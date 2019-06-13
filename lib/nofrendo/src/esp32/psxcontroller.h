#ifndef PSXCONTROLLER_H
#define PSXCONTROLLER_H

int psxReadInput();
void psxcontrollerInit();
bool getShowMenu();
int getBright();
int getVolume();
bool getShutdown();
bool isSelectPressed(int ctl);
bool isStartPressed(int ctl);
bool isUpPressed(int ctl);
bool isRightPressed(int ctl);
bool isDownPressed(int ctl);
bool isLeftPressed(int ctl);
bool isAPressed(int ctl);
bool isBPressed(int ctl);
bool isMenuPressed(int ctl);
bool isBrightnessPressed(int ctl);
bool isAnyPressed(int ctl);

#endif