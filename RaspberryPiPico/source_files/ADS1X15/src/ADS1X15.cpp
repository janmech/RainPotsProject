/**************************************************************************/
/*!
    @file     PICO_ADS1X15.cpp
    @author   K.Townsend (Adafruit Industries)

    @mainpage Adafruit ADS1X15 ADC Breakout Driver

    @section intro_sec Introduction

    This is a library for the Adafruit ADS1X15 ADC breakout boards.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section author Author

    Written by Kevin "KTOWN" Townsend for Adafruit Industries.
    Modified for compatibility with the Rasperry Pi Pico Board by Jan Mech

    @section  HISTORY

    v1.0  - First release
    v1.1  - Added ADS1115 support - W. Earl
    v2.0  - Refactor - C. Nelson

    @section license License

    BSD license, all text here must be included in any redistribution
*/
/**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "I2cController.h"
#include "ADS1X15.h"

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1015 class w/appropriate properties
*/
/**************************************************************************/
ADS1X15::ADS1X15(I2cController *bustInst)
{
  m_bitShift = 4;
  m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */
  m_dataRate = RATE_ADS1015_1600SPS;
  m_i2c = bustInst;
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
*/
/**************************************************************************/
PICO_ADS1115::PICO_ADS1115(I2cController *busInst)
{
  m_bitShift = 0;
  m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */
  m_dataRate = RATE_ADS1115_128SPS;
  m_i2c = busInst;
}

/**************************************************************************/
/*!
    @brief  Sets the gain and input voltage range

    @param gain gain setting to use
*/
/**************************************************************************/
void PICO_ADS1X15::setGain(adsGain_t gain) { m_gain = gain; }

/**************************************************************************/
/*!
    @brief  Gets a gain and input voltage range

    @return the gain setting
*/
/**************************************************************************/
adsGain_t PICO_ADS1X15::getGain() { return m_gain; }

/**************************************************************************/
/*!
    @brief  Sets the data rate

    @param rate the data rate to use
*/
/**************************************************************************/
void PICO_ADS1X15::setDataRate(uint16_t rate) { m_dataRate = rate; }

/**************************************************************************/
/*!
    @brief  Gets the current data rate

    @return the data rate
*/
/**************************************************************************/
uint16_t PICO_ADS1X15::getDataRate() { return m_dataRate; }

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel

    @param channel ADC channel to read

    @return the ADC reading
*/
/**************************************************************************/
int16_t PICO_ADS1X15::readSingleEnded(uint8_t channel)
{
  if (channel > 3)
  {
    return 0;
  }

  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set data rate
  config |= m_dataRate;

  // Set single-ended input channel
  switch (channel)
  {
  case (0):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(ADS1X15_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  while (!conversionComplete())
    ;

  // Read the conversion results
  return getLastConversionResults();
}

uint16_t PICO_ADS1X15::pushConfig(uint16_t bitsToUpdate)
{
  uint16_t config = readRegister(ADS1X15_REG_POINTER_CONFIG);
  config |= bitsToUpdate;
  writeRegister(ADS1X15_REG_POINTER_CONFIG, config);
  return readRegister(ADS1X15_REG_POINTER_CONFIG);
}

/**************************************************************************/
/*!
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.

    @return the ADC reading
*/
/**************************************************************************/
int16_t PICO_ADS1X15::readDifferentialA0A1()
{
  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set data rate
  config |= m_dataRate;

  // Set channels
  config |= ADS1X15_REG_CONFIG_MUX_DIFF_0_1; // AIN0 = P, AIN1 = N

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(ADS1X15_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  while (!conversionComplete())
    ;

  // Read the conversion results
  return getLastConversionResults();
}

/**************************************************************************/
/*!
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN2) and N (AIN3) input.  Generates
            a signed value since the difference can be either
            positive or negative.

    @return the ADC reading
*/
/**************************************************************************/
int16_t PICO_ADS1X15::readDifferentialA2A3()
{
  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_NONE |    // Disable the comparator (default val)
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
                                        // ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
      ADS1X15_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= m_gain;

  // Set data rate
  config |= m_dataRate;

  // Set channels
  config |= ADS1X15_REG_CONFIG_MUX_DIFF_2_3; // AIN2 = P, AIN3 = N

  // Set 'start single-conversion' bit
  config |= ADS1X15_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(ADS1X15_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  while (!conversionComplete())
    ;

  // Read the conversion results
  return getLastConversionResults();
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.

            This will also set the ADC in continuous conversion mode.

    @param channel ADC channel to use
    @param threshold comparator threshold
*/
/**************************************************************************/
void PICO_ADS1X15::startComparatorSingleEnded(uint8_t channel,
                                              int16_t threshold)
{
  // Start with default values
  uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |   // Comparator enabled and asserts on 1
                                        // match
      ADS1X15_REG_CONFIG_CLAT_LATCH |   // Latching mode
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD |   // Traditional comparator (default val)
      ADS1X15_REG_CONFIG_MODE_CONTIN |  // Continuous conversion mode
      ADS1X15_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

  // Set PGA/voltage range
  config |= m_gain;

  // Set data rate
  config |= m_dataRate;

  // Set single-ended input channel
  switch (channel)
  {
  case (0):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_0;
    break;
  case (1):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_1;
    break;
  case (2):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_2;
    break;
  case (3):
    config |= ADS1X15_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1015
  writeRegister(ADS1X15_REG_POINTER_HITHRESH, threshold << m_bitShift);

  // Write config register to the ADC
  writeRegister(ADS1X15_REG_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.

    @return the last ADC reading
*/
/**************************************************************************/
int16_t PICO_ADS1X15::getLastConversionResults()
{
  // Read the conversion results
  uint16_t res = readRegister(ADS1X15_REG_POINTER_CONVERT) >> m_bitShift;
  if (m_bitShift == 0)
  {
    return (int16_t)res;
  }
  else
  {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF)
    {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}

/**************************************************************************/
/*!
    @brief  Returns true if conversion is complete, false otherwise.

    @param counts the ADC reading in raw counts

    @return the ADC reading in volts
*/
/**************************************************************************/
float PICO_ADS1X15::computeVolts(int16_t counts)
{
  // see data sheet Table 3
  float fsRange;
  switch (m_gain)
  {
  case GAIN_TWOTHIRDS:
    fsRange = 6.144f;
    break;
  case GAIN_ONE:
    fsRange = 4.096f;
    break;
  case GAIN_TWO:
    fsRange = 2.048f;
    break;
  case GAIN_FOUR:
    fsRange = 1.024f;
    break;
  case GAIN_EIGHT:
    fsRange = 0.512f;
    break;
  case GAIN_SIXTEEN:
    fsRange = 0.256f;
    break;
  default:
    fsRange = 0.0f;
  }
  return counts * (fsRange / (32768 >> m_bitShift));
}

/**************************************************************************/
/*!
    @brief  Returns true if conversion is complete, false otherwise.
*/
/**************************************************************************/
bool PICO_ADS1X15::conversionComplete()
{
  return (readRegister(ADS1X15_REG_POINTER_CONFIG) & 0x8000) != 0;
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register

    @param reg register address to write to
    @param value value to write to register
*/
/**************************************************************************/
void PICO_ADS1X15::writeRegister(uint8_t reg, uint16_t value)
{
  buffer[0] = reg;
  buffer[1] = value >> 8;
  buffer[2] = value & 0xFF;
  m_i2c->write(ADS1X15_ADDRESS, buffer, 3, false);
}

/**************************************************************************/
/*!
    @brief  Read 16-bits from the specified destination register

    @param reg register address to read from

    @return 16 bit register value read
*/
/**************************************************************************/
uint16_t PICO_ADS1X15::readRegister(uint8_t reg)
{
  buffer[0] = reg;

  m_i2c->write(ADS1X15_ADDRESS, buffer, 1, false);
  m_i2c->read(ADS1X15_ADDRESS, buffer, 2, false);
  return ((buffer[0] << 8) | buffer[1]);
}

void PICO_ADS1X15::testWrite()
{
  uint8_t testBuf[] = {0xAA, 0xCC, 0x33};
  m_i2c->write(ADS1X15_ADDRESS, testBuf, 3, false);
}