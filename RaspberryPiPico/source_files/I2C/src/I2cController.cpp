
/**
 * @file I2cController.cpp
 * @author Jan Mech (mech.jan@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "I2cController.h"
//#include "vl53l0x_debug.h"

/**
 * @brief Construct a new I2cController::I2cController object
 * 
 * @param i2_bus 
 * @param baudrate 
 * @param sda 
 * @param scl 
 * @param pullup 
 */
I2cController::I2cController(i2c_inst_t *i2_bus, uint baudrate, uint8_t sda, uint8_t scl, bool pullup)
{
    m_i2c_bus = i2_bus;
    m_baudrate = baudrate;
    m_pin_scl = scl;
    m_pin_sda = sda;
    m_pullup = pullup;
}

/**
 * @brief Construct a new I2cController::I2cController object
 * 
 */
I2cController::I2cController()
{
    m_i2c_bus = i2c0;
    m_baudrate = 100000;
    m_pin_scl = 8;
    m_pin_sda = 9;
    m_pullup = true;
}

/**
 * @brief 
 * 
 */
void I2cController::init()
{
    if (_initialized)
    {
        end();
    }
    gpio_init(m_pin_sda);
    gpio_init(m_pin_scl);
    gpio_set_function(m_pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(m_pin_scl, GPIO_FUNC_I2C);
    uint real_baud = i2c_init(m_i2c_bus, m_baudrate);
#ifdef DEBUG
    printf("real baud: %d\n", real_baud));
#endif
    i2c_set_slave_mode(m_i2c_bus, false, 0x1C);

    if (m_pullup)
    {
        gpio_pull_up(m_pin_sda);
        gpio_pull_up(m_pin_scl);
    }
    _initialized = true;
}

/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool I2cController::isInitialized()
{
    return _initialized;
}

/**
 * @brief 
 * 
 */
void I2cController::end()
{
    i2c_deinit(m_i2c_bus);
    _initialized = false;
}

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t I2cController::getPinSda()
{
    return m_pin_sda;
}

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t I2cController::getPinScl()
{
    return m_pin_scl;
}

/**
 * @brief 
 * 
 * @return uint 
 */
uint I2cController::getBaudreate()
{
    return m_baudrate;
}
void I2cController::setBaudrate(uint br)
{
    m_baudrate = br;
}

/**
 * @brief 
 * 
 * @return i2c_inst_t* 
 */
i2c_inst_t *I2cController::getI2cInstance()
{
    return m_i2c_bus;
}

/**
 * @brief 
 * 
 * @param baudrate 
 * @return uint 
 */
uint I2cController::setClock(uint baudrate)
{
    return i2c_set_baudrate(getI2cInstance(), baudrate);
}

/**
 * @brief 
 * 
 * @param address 
 * @param data 
 * @param size 
 * @param nostop 
 * @return int 
 */
int I2cController::write(uint8_t address, uint8_t *data, size_t size, bool nostop)
{
    return i2c_write_blocking(m_i2c_bus, address, data, size, nostop);
}

/**
 * @brief 
 * 
 * @param address 
 * @param data 
 * @param size 
 * @param nostop 
 * @return int 
 */
int I2cController::read(uint8_t address, uint8_t *data, size_t size, bool nostop)
{
    return i2c_read_blocking(m_i2c_bus, address, data, size, nostop);
}

/**
 * @brief 
 * 
 */
void I2cController::scanBus()
{
    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    for (int addr = 0; addr < (1 << 7); ++addr)
    {
        if (addr % 16 == 0)
        {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata[100];
        if (isReservedAddress(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(m_i2c_bus, addr, rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n\n");
}