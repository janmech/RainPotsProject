#include "RpConfig.h"

RpConfig::RpConfig(Eeprom24LC32 *storage, bool force_config_init)
{
    this->storage = storage;
    if (!this->storageIsInitialized() || force_config_init)
    {
        this->initStorage();
    }
}

void RpConfig::initStorage()
{
    this->storage->erase();
    // Initialize Button configs
    for (uint8_t i = 0; i < 6; i++)
    {
        controller_config_t default_config;
        default_config.index = i;
        // Set values not used for Buttons to 0
        default_config.min = 0;
        default_config.max = 0;
        default_config.center = 0;
        if (i == 1)
        {
            default_config.mode = CONTROLLER_MODE_INCREMENT;
        }
        this->writeControllerConfig(&default_config);
    }
    // Initialize Knob configs
    for (uint8_t j = 6; j < 14; j++)
    {
        controller_config_t default_config;
        default_config.index = j;
        default_config.mode = CONTROLLER_MODE_KNOB;
        default_config.min = 5;
        default_config.max = 990;
        default_config.center = 0;
        this->writeControllerConfig(&default_config);
    }

    this->storage->writeByte(MEM_ADDRESS_INC_STEPS, 0x05);
    this->storage->writeInt32(MEM_ADDRESS_CONTROLLER_STATUS, 0xFFFF);
    this->storage->writeByte(MEM_ADDRESS_INC_DISPLAY_ZERO, 0x01);
    this->storage->writeByte(MEM_ADDRESS_RADIO_GROUP_DISPLAY_ZERO, 0x00);
    this->storage->writeByte(MEM_ADDRESS_INITIALIZED, MEM_INITIALIZED_TOKEN);
}

uint8_t RpConfig::readControllerMode(uint8_t button_index)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_MODE;
    busy_wait_ms(5);
    return this->storage->readByte(mem_address);
};

void RpConfig::writeControllerMode(uint8_t button_index, uint8_t value)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_MODE;
    this->storage->writeByte(mem_address, value);
};

uint8_t RpConfig::readControllerValue(uint8_t button_index)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_VALUE;
    busy_wait_ms(5);
    return this->storage->readByte(mem_address);
}

void RpConfig::writeControllerValue(uint8_t button_index, uint8_t value)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_VALUE;
    this->storage->writeByte(mem_address, value);
}

uint32_t RpConfig::readControllerMin(uint8_t button_index)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_MIN;
    busy_wait_ms(5);
    return this->storage->readInt32(mem_address);
};

void RpConfig::writeControllerMin(uint8_t button_index, uint32_t value)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_MIN;
    this->storage->writeInt32(mem_address, value);
    busy_wait_ms(5);
};

uint32_t RpConfig::readControllerMax(uint8_t button_index)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_MAX;
    busy_wait_ms(5);
    return this->storage->readInt32(mem_address);
};

void RpConfig::writeControllerMax(uint8_t button_index, uint32_t value)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_MAX;
    this->storage->writeInt32(mem_address, value);
    busy_wait_ms(5);
};

uint32_t RpConfig::readControllerCenter(uint8_t button_index)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_CENTER;
    busy_wait_ms(5);
    return this->storage->readInt32(mem_address);
};

void RpConfig::writeControllerCenter(uint8_t button_index, uint32_t value)
{
    uint32_t mem_address = this->controllerConfigBaseAddress(button_index) + MEM_OFFSET_CONTROLLER_CENTER;
    this->storage->writeInt32(mem_address, value);
    busy_wait_ms(5);
};

void RpConfig::readControllerConfig(controller_config_t *button_config)
{
    button_config->mode = this->readControllerMode(button_config->index);
    button_config->value = this->readControllerValue(button_config->index);
    button_config->min = this->readControllerMin(button_config->index);
    button_config->max = this->readControllerMax(button_config->index);
    button_config->center = this->readControllerCenter(button_config->index);
};

void RpConfig::writeControllerConfig(controller_config_t *button_config)
{
    this->writeControllerMode(button_config->index, button_config->mode);
    this->writeControllerValue(button_config->index, button_config->value);
    this->writeControllerMin(button_config->index, button_config->min);
    this->writeControllerMax(button_config->index, button_config->max);
    this->writeControllerCenter(button_config->index, button_config->center);
};

uint8_t RpConfig::readIncSteps()
{
    return this->storage->readByte(MEM_ADDRESS_INC_STEPS);
    busy_wait_ms(5);
}

void RpConfig::writeIncSteps(uint8_t max_steps)
{
    this->storage->writeByte(MEM_ADDRESS_INC_STEPS, max_steps);
    busy_wait_ms(5);
}

void RpConfig::writeControllerStatus(uint32_t ctl_status)
{
    this->storage->writeInt32(MEM_ADDRESS_CONTROLLER_STATUS, ctl_status);
    busy_wait_ms(5);
}

bool RpConfig::readIncrementDisplayZero()
{
    return (bool)this->storage->readByte(MEM_ADDRESS_INC_DISPLAY_ZERO);
}

void RpConfig::writeIncrementDisplayZero(bool display_zero)
{
    this->storage->writeByte(MEM_ADDRESS_INC_DISPLAY_ZERO, (uint8_t)display_zero);
    busy_wait_ms(5);
}

bool RpConfig::readRadioGroupDisplayZero()
{
    return (bool)this->storage->readByte(MEM_ADDRESS_RADIO_GROUP_DISPLAY_ZERO);
}

void RpConfig::writeRadioGroupDisplayZero(bool display_zero)
{
    this->storage->writeByte(MEM_ADDRESS_RADIO_GROUP_DISPLAY_ZERO, (uint8_t)display_zero);
    busy_wait_ms(5);
}

uint32_t RpConfig::readControllerStatus()
{
    return this->storage->readInt32(MEM_ADDRESS_CONTROLLER_STATUS);
}

// Protected Methods

uint32_t RpConfig::controllerConfigBaseAddress(uint8_t index)
{
    return MEM_ADDRESS_BUTTON_CONFIG + (index * CONTROLLER_CONFIG_BYTE_SIZE);
}

bool RpConfig::storageIsInitialized()
{
    uint8_t init_token = this->storage->readByte(MEM_ADDRESS_INITIALIZED);
    return init_token == MEM_INITIALIZED_TOKEN;
}
