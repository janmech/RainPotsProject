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

#ifndef __24LCXX_H__
#define __24LCXX_H__

#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "I2cController.h"

//Default to safe 32 bytes
#define I2C_BUFFER_LENGTH_RX 32
#define I2C_BUFFER_LENGTH_TX 32

struct struct_memorySettings
{
    I2cController *i2cPort;
    uint8_t deviceAddress;
    uint32_t memorySize_bytes;
    uint16_t pageSize_bytes;
    uint8_t pageWriteTime_ms;
    bool pollForWriteComplete;
    uint16_t i2cBufferSize;
};

class Eeprom24LC32
{
public:
    Eeprom24LC32(I2cController *i2cPort, uint8_t deviceAddress = 0x50);
    uint8_t readByte(uint32_t memoryAddress);
    void read(uint32_t memoryAddress, uint8_t *buff, uint16_t bufferSize);
    void readPage(uint16_t pageIndex, uint8_t *buff);
    void writeByte(uint32_t memoryAddress, uint8_t dataToWrite);
    void write(uint32_t memoryAddress, uint8_t *dataToWrite, uint16_t blockSize);
    void writePage(uint16_t pageIndex, uint8_t *buff);
    bool isConnected();
    bool isBusy();
    void erase(uint8_t toWrite = 0x00); //Erase the entire memory. Optional: write a given byte to each spot.
    void dump();
    void setMemorySize(uint32_t memSize); //Set the size of memory in bytes
    uint32_t getMemorySize();             //Return size of EEPROM
    void setPageSize(uint16_t pageSize);  //Set the size of the page we can write at a time
    uint16_t getPageSize();
    void setPageWriteTime(uint8_t writeTimeMS); //Set the number of ms required per page write
    uint8_t getPageWriteTime();
    void enablePollForWriteComplete(); //Most EEPROMs all I2C polling of when a write has completed
    void disablePollForWriteComplete();
    uint16_t getI2CBufferSize(); //Return the size of the TX buffer
    uint16_t getMaxPageIndex();
    void writeInt32(uint32_t memoryAddress, int32_t value);
    int32_t readInt32(uint32_t memoryAddress);


private:
    //Variables
    struct_memorySettings settings = {
        .i2cPort = NULL,
        .deviceAddress = 0x50, //0b1010 + (A2 A1 A0) or 0b1010 + (B0 A1 A0) for larger (>512kbit) EEPROMs
        .memorySize_bytes = 32768 / 8,
        .pageSize_bytes = 32,
        .pageWriteTime_ms = 5,
        .pollForWriteComplete = true,
    };
    /**
     * @brief 
     * 
     * @param forceWait 
     */
    void _waitForEepromReady(bool forceWait = true)
    {
        if (isBusy())
        {
            if (!settings.pollForWriteComplete && forceWait)
            {
                busy_wait_us_32(settings.pageWriteTime_ms *1000); //Delay the setv amount of time to record a page
            }
            else
            {
                volatile uint64_t before = time_us_64() / 1000;
                volatile uint64_t after;
                volatile uint64_t time_diff;
                while (isBusy())
                {
                    // Faster access but hammers the I2C bus
                    busy_wait_us_32(100);
                    after = time_us_64() / 1000;
                    time_diff = after - before;
                    if (time_diff >= settings.pageWriteTime_ms)
                    {
                        return;
                    }
                }
            }
        }
    }
};

#endif //__24LCXX_H__
