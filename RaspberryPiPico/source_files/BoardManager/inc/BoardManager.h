#ifndef __BOARD_MANAGER_H__
#define __BOARD_MANAGER_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"


class BoardManager
{
public:
    BoardManager(char deviceName[], char manufacturer[]);
    char *getBoardId();
    char *getDeviceName();
    char *getDeviceFullName();
    char *getDeviceManufacturer();

protected:
    char m_bid[17];
    char m_device_name[12];
    char m_manufacturer[9];
    char m_full_device_name[31];
};

#endif