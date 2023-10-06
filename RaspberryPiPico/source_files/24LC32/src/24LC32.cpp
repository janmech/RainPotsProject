/*
  This is a library to read/write to external I2C EEPROMs.
  It uses the same template system found in the Arduino
  EEPROM library so you can use the same get() and put() functions.

  https://github.com/sparkfun/SparkFun_External_EEPROM_Arduino_Library
  Best used with the Qwiic EEPROM: https://www.sparkfun.com/products/14764

  Various external EEPROMs have various interface specs
  (overall size, page size, write times, etc). This library works with
  all types and allows the various settings to be set at runtime.

  All read and write restrictions associated with pages are taken care of.
  You can access the external memory as if it was contiguous.

  Development environment specifics:
  Arduino IDE 1.8.x

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#include "24LC32.h"
#include <stdio.h>

Eeprom24LC32::Eeprom24LC32(I2cController *i2cPort, uint8_t deviceAddress)
{

    settings.i2cPort = i2cPort;
    settings.deviceAddress = deviceAddress;
}

// Erase entire EEPROM
void Eeprom24LC32::erase(uint8_t toWrite)
{
    uint8_t tempBuffer[settings.pageSize_bytes];
    for (uint32_t x = 0; x < settings.pageSize_bytes; x++)
        tempBuffer[x] = toWrite;

    for (uint32_t addr = 0; addr < getMemorySize(); addr += settings.pageSize_bytes)
        write(addr, tempBuffer, settings.pageSize_bytes);
}

void Eeprom24LC32::dump()
{
    int max_page_count = settings.memorySize_bytes / settings.pageSize_bytes;
    uint8_t page_data[getPageSize()];
    for (size_t page_index = 0; page_index < max_page_count; page_index++)
    {
        readPage(page_index, page_data);
        for (size_t data_index = 0; data_index < getPageSize(); data_index++)
        {
            printf("0x%02X ", page_data[data_index]);
        }
        printf("\n");
    }
}
// Returns true if device is detected
bool Eeprom24LC32::isConnected()
{
    uint8_t rxdata[100];
    int16_t ret = settings.i2cPort->read(settings.deviceAddress, rxdata, 1, false);
    return (ret >= 0);
}

/**
 * @brief
 *
 * @param i2cAddress
 * @return true
 * @return false
 */
bool Eeprom24LC32::isBusy()
{
    return !isConnected();
}

/**
 * @brief
 *
 * @param memSize
 */
void Eeprom24LC32::setMemorySize(uint32_t memSize)
{
    settings.memorySize_bytes = memSize;
}

/**
 * @brief
 *
 * @return uint32_t
 */
uint32_t Eeprom24LC32::getMemorySize()
{
    return settings.memorySize_bytes;
}

/**
 * @brief
 *
 * @param pageSize
 */
void Eeprom24LC32::setPageSize(uint16_t pageSize)
{
    settings.pageSize_bytes = pageSize;
}

/**
 * @brief
 *
 * @return uint16_t
 */
uint16_t Eeprom24LC32::getPageSize()
{
    return settings.pageSize_bytes;
}

/**
 * @brief
 *
 * @param writeTimeMS
 */
void Eeprom24LC32::setPageWriteTime(uint8_t writeTimeMS)
{
    settings.pageWriteTime_ms = writeTimeMS;
}

/**
 * @brief
 *
 * @return uint8_t
 */
uint8_t Eeprom24LC32::getPageWriteTime()
{
    return settings.pageWriteTime_ms;
}

/**
 * @brief
 *
 */
void Eeprom24LC32::enablePollForWriteComplete()
{
    settings.pollForWriteComplete = true;
}

/**
 * @brief
 *
 */
void Eeprom24LC32::disablePollForWriteComplete()
{
    settings.pollForWriteComplete = false;
}

/**
 * @brief
 *
 * @return uint16_t
 */
uint16_t Eeprom24LC32::getI2CBufferSize()
{
    return I2C_BUFFER_LENGTH_TX;
}

/**
 * @brief Read a byte from a given location
 *
 * @param memoryAddress
 * @return uint8_t
 */
uint8_t Eeprom24LC32::readByte(uint32_t memoryAddress)
{
    uint8_t tempByte;
    read(memoryAddress, &tempByte, 1);
    return tempByte;
}

/**
 * @brief
 *
 * @param pageIndex
 * @param buff
 */
void Eeprom24LC32::readPage(uint16_t pageIndex, uint8_t *buff)
{
    uint16_t maxPageIndex = getMaxPageIndex();
    if (pageIndex > maxPageIndex)
    {
        return;
    }
    uint16_t memoryAddress = pageIndex * settings.pageSize_bytes;
    _waitForEepromReady();
    uint8_t memoryLocation[] = {
        (uint8_t)((memoryAddress) >> 8), // MSB
        (uint8_t)((memoryAddress)&0xFF)  // LSB
    };
    settings.i2cPort->write(settings.deviceAddress, memoryLocation, 2, false);
    int bytesRead = settings.i2cPort->read(settings.deviceAddress, buff, settings.pageSize_bytes, false);
}

/**
 * @brief
 *
 * @param pageIndex
 * @param buff
 */
void Eeprom24LC32::writePage(uint16_t pageIndex, uint8_t *buff)
{
    uint16_t maxPageIndex = getMaxPageIndex();
    if (pageIndex > maxPageIndex)
    {
        return;
    }
    uint16_t memoryAddress = pageIndex * settings.pageSize_bytes;
    write(memoryAddress, buff, settings.pageSize_bytes);
}
/**
 * @brief
 *
 * @param memoryAddress
 * @param buff
 * @param bufferSize
 */
void Eeprom24LC32::read(uint32_t memoryAddress, uint8_t *buff, uint16_t bufferSize)
{
    uint16_t amtToRead = bufferSize;
    if (memoryAddress + amtToRead > settings.memorySize_bytes)
    {
        amtToRead = settings.memorySize_bytes;
    }

    uint8_t i2cAddress = settings.deviceAddress;

    _waitForEepromReady();

    uint8_t memoryLocation[] = {
        (uint8_t)((memoryAddress) >> 8), // MSB
        (uint8_t)((memoryAddress)&0xFF)  // LSB
    };
    settings.i2cPort->write(i2cAddress, memoryLocation, 2, false);

    settings.i2cPort->read(i2cAddress, buff, amtToRead, false);
}

/**
 * @brief Write a byte to a given location
 *
 * @param memoryAddress
 * @param dataToWrite
 */
void Eeprom24LC32::writeByte(uint32_t memoryAddress, uint8_t dataToWrite)
{
    if (readByte(memoryAddress) != dataToWrite)
    { // Update only if data is new
        write(memoryAddress, &dataToWrite, 1);
    }
}

/**
 * @brief
 *
 * @param memoryAddress
 * @param dataToWrite
 * @param bufferSize
 */
void Eeprom24LC32::write(uint32_t memoryAddress, uint8_t *dataToWrite, uint16_t bufferSize)
{
    // Clip buffer if overreaches max meneory address
    if (memoryAddress + bufferSize >= settings.memorySize_bytes)
    {
        bufferSize = settings.memorySize_bytes - memoryAddress;
    }

    uint16_t pageSize = settings.pageSize_bytes;
    // Break the buffer into page sized chunks
    uint16_t bytesWritten = 0;
    while (bytesWritten < bufferSize)
    {
        int amountToWrite = bufferSize - bytesWritten;
        if (amountToWrite > pageSize)
        {
            amountToWrite = pageSize;
        }
        uint8_t i2cAddress = settings.deviceAddress;

        if (amountToWrite > 1)
        {
            // Check for crossing of a page line. Writes cannot cross a page line.
            uint16_t pageNumber1 = (memoryAddress + bytesWritten) / settings.pageSize_bytes;
            uint16_t pageNumber2 = (memoryAddress + bytesWritten + amountToWrite - 1) / settings.pageSize_bytes;

            if (pageNumber2 > pageNumber1)
            {
                // Limit the write amount to go right up to edge of page barrier
                amountToWrite = (pageNumber2 * settings.pageSize_bytes) - (memoryAddress + bytesWritten);
            }
        }
        // See if EEPROM is available or still writing a previous request
        _waitForEepromReady();

        uint8_t memoryLocation[] = {
            (uint8_t)((memoryAddress + bytesWritten) >> 8),  // MSB
            (uint8_t)((memoryAddress + bytesWritten) & 0xFF) // LSB
        };
        uint8_t allData[amountToWrite + 2];
        allData[0] = memoryLocation[0];
        allData[1] = memoryLocation[1];
        for (size_t i = 0; i < amountToWrite; i++)
        {
            allData[i + 2] = dataToWrite[i + bytesWritten];
        }
        int written_mem_add = settings.i2cPort->write(i2cAddress, allData, amountToWrite + 2, false);
        bytesWritten += amountToWrite;
        _waitForEepromReady();
    }
}

/**
 * @brief
 *
 * @return uint16_t
 */
uint16_t Eeprom24LC32::getMaxPageIndex()
{
    return (uint16_t)(settings.memorySize_bytes / settings.pageSize_bytes) - 1;
}

/**
 * @brief
 *
 * @param memoryAddress
 * @param value
 */
void Eeprom24LC32::writeInt32(uint32_t memoryAddress, int32_t value)
{
    uint8_t bytes[4];
    bytes[0] = (value >> 24) & 0xff; // MSB
    bytes[1] = (value >> 16) & 0xff;
    bytes[2] = (value >> 8) & 0xff;
    bytes[3] = value & 0xff; // LSB
    write(memoryAddress, bytes, 4);
}

/**
 * @brief
 *
 * @param memoryAddress
 * @return int32_t
 */
int32_t Eeprom24LC32::readInt32(uint32_t memoryAddress)
{
    uint8_t bytes[4];
    read(memoryAddress, bytes, 4);
    int32_t value = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
    return value;
}