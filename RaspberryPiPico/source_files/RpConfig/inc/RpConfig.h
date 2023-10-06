#ifndef __RP_CONFIG_H__
#define __RP_CONFIG_H__

#include "24LC32.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef __BIT_MACROS__
#define __BIT_MACROS__
#define BIT_SET(BF, N) BF |= ((uint32_t)0x01 << N)
#define BIT_CLR(BF, N) BF &= ~((uint32_t)0x01 << N)
#define BIT_ISSET(BF, N) ((BF >> N) & 0x01)
#define BIT_TGL(BF, N) BF ^= ((uint8_t)0x01 << N))
#endif

#ifndef __CONTROLLER_MODES__
#define __CONTROLLER_MODES__
#define CONTROLLER_MODE_TOGGLE 0
#define CONTROLLER_MODE_MOMENTARY 1
#define CONTROLLER_MODE_INCREMENT 2
#define CONTROLLER_MODE_KNOB 3
#define CONTROLLER_MODE_RADIO_GROUP 4
#endif

// Eeprom Memory Map
#define MEM_ADDRESS_INITIALIZED 0x00
#define MEM_ADDRESS_INC_STEPS 0x01
#define MEM_ADDRESS_CONTROLLER_STATUS 0x02 // uint32_t : 4 bytes wide
#define MEM_ADDRESS_INC_DISPLAY_ZERO 0x06
#define MEM_ADDRESS_RADIO_GROUP_DISPLAY_ZERO 0x07

#define MEM_ADDRESS_BUTTON_CONFIG 0x20
#define CONTROLLER_CONFIG_BYTE_SIZE 0x10
#define MEM_OFFSET_CONTROLLER_MODE 0x00
#define MEM_OFFSET_CONTROLLER_VALUE 0x01
#define MEM_OFFSET_CONTROLLER_MIN 0x03
#define MEM_OFFSET_CONTROLLER_MAX 0x07
#define MEM_OFFSET_CONTROLLER_CENTER 0x0B

#define MEM_INITIALIZED_TOKEN 0x8B

// We use unly one struct for Buttons and Knobs config to simplify the data structre
// Buttons use: index (starting at 0), mode (CONTROLLER_MODE_TOGGLE, CONTROLLER_MODE_MOMENTARY, CONTROLLER_MODE_INCREMENT), value
// Kobs use   : index (starting at 6), mode (CONTROLLER_MODE_KNOB), min, max, center
typedef struct
{
    uint8_t index = 0;
    uint8_t mode = CONTROLLER_MODE_TOGGLE; // Memory width: 1 Byte
    uint8_t value = 0;                     // Memory width: 1 Byte
    uint32_t min = 5;                      // Memory width: 4 Bytes
    uint32_t max = 990;                    // Memory width: 4 Bytes
    uint32_t center = 0;                   // Memory width: 4 Bytes
} controller_config_t;

class RpConfig
{
protected:
    Eeprom24LC32 *storage;

    uint32_t controllerConfigBaseAddress(uint8_t index);
    bool storageIsInitialized();

public:
    RpConfig(Eeprom24LC32 *storage, bool force_config_init = false);
    uint8_t readIncSteps();
    void writeIncSteps(uint8_t max_steps);

    bool readIncrementDisplayZero();
    void writeIncrementDisplayZero(bool display_zero);

    bool readRadioGroupDisplayZero();
    void writeRadioGroupDisplayZero(bool display_zero);


    uint8_t readControllerMode(uint8_t button_index);
    void writeControllerMode(uint8_t button_index, uint8_t value);

    uint8_t readControllerValue(uint8_t button_index);
    void writeControllerValue(uint8_t button_index, uint8_t value);

    uint32_t readControllerMin(uint8_t button_index);
    void writeControllerMin(uint8_t button_index, uint32_t value);

    uint32_t readControllerMax(uint8_t button_index);
    void writeControllerMax(uint8_t button_index, uint32_t value);

    uint32_t readControllerCenter(uint8_t button_index);
    void writeControllerCenter(uint8_t button_index, uint32_t value);

    void readControllerConfig(controller_config_t *button_config);
    void writeControllerConfig(controller_config_t *button_config);

    void initStorage();

    void writeControllerStatus(uint32_t ctl_status);
    uint32_t readControllerStatus();
};

#endif