/**
 * @file I2cController.h
 * @author Jan Mech (mech.jan@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __I2C_CONTROLLER_H__
#define __I2C_CONTROLLER_H__

#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "string.h"

/**
 * @brief I2C BUS ops for Controller device: Initialzation, receiving data, sending data. Inspired by the Arduino Wire.h library
 * 
 */
class I2cController
{
private:
    bool _initialized = false;

protected:
    // Instance-specific properties
    i2c_inst_t *m_i2c_bus; ///< I2C bus device
    uint8_t m_pin_sda;
    uint8_t m_pin_scl;
    uint m_baudrate;
    bool m_pullup;
    bool isReservedAddress(uint8_t addr) {
        return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
    }

public:
    I2cController();
    I2cController(i2c_inst_t *i2_bus, uint baudrate, uint8_t sda, uint8_t scl, bool pullup);
    void init();
    void end();
    uint8_t getPinSda();
    uint8_t getPinScl();
    uint getBaudreate();
    void setBaudrate(uint);
    uint setClock(uint);
    i2c_inst_t *getI2cInstance();
    int write(uint8_t address, uint8_t *data, size_t size, bool nostop);
    int read(uint8_t address, uint8_t *data, size_t size, bool nostop);
    bool isInitialized();
    void scanBus();
    void test();
};
#endif