#ifndef __I2C_PERIPHERAL_H__
#define __I2C_PERIPHERAL_H__

#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

class I2cPeripheral
{
protected:
    // Instance-specific properties
    i2c_inst_t *m_i2c_bus; ///< I2C bus device
    uint8_t m_pin_sda;
    uint8_t m_pin_scl;
    uint m_baudrate;
    uint8_t m_address;

public:
    I2cPeripheral(uint8_t address);
    I2cPeripheral(uint8_t address, i2c_inst_t *i2_bus, uint baudrate, uint8_t sda, uint8_t scl);
    void begin();
    uint8_t getPinSda();
    uint8_t getPinScl();
    uint getBaudreate();
    i2c_inst_t *getI2cInstance();
};
#endif