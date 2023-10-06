#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "I2cPeripheral.h"

I2cPeripheral::I2cPeripheral(uint8_t address, i2c_inst_t *i2_bus = i2c0, uint baudrate = 100000, uint8_t sda = 8, uint8_t scl = 9)
{
    m_i2c_bus = i2_bus;
    m_baudrate = baudrate;
    m_pin_scl = scl;
    m_pin_sda = sda;
}

I2cPeripheral::I2cPeripheral(uint8_t address)
{
    m_i2c_bus = i2c0;
    m_baudrate = 100000;
    m_pin_scl = 8;
    m_pin_sda = 9;
    m_address = address;
}

void I2cPeripheral::begin()
{
    i2c_init(m_i2c_bus, m_baudrate);
    i2c_set_slave_mode(m_i2c_bus, true, m_address);
    gpio_set_function(m_pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(m_pin_scl, GPIO_FUNC_I2C);
}

uint8_t I2cPeripheral::getPinSda()
{
    return m_pin_sda;
}

uint8_t I2cPeripheral::getPinScl()
{
    return m_pin_scl;
}

uint I2cPeripheral::getBaudreate()
{
    return m_baudrate;
}

i2c_inst_t *I2cPeripheral::getI2cInstance()
{
    return m_i2c_bus;
}