/**************************************************************************/
/*!
    @file     PICO_ADS1X15.h

    This is a library for the Adafruit ADS1X15 ADC breakout boards.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    Written by Kevin "KTOWN" Townsend for Adafruit Industries.

    BSD license, all text here must be included in any redistribution

    Modified for compatibility with the Rasperry Pi Pico Board by Jan Mech
*/
/**************************************************************************/
#ifndef __ADS1X15_H__
#define __ADS1X15_H__

#include <stdlib.h>
#include <stdint.h>
#include "hardware/i2c.h"
#include "I2cController.h"
#include "I2cController.h"

#define I2C_BUS_0 0
#define I2C_BUS_1 1
/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define ADS1X15_ADDRESS (0x48)                        ///< 1001 000 (ADDR = GND)
/*=========================================================================*/

/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
#define ADS1X15_REG_POINTER_MASK (0x03)               ///< Point mask
#define ADS1X15_REG_POINTER_CONVERT (0x00)            ///< Conversion
#define ADS1X15_REG_POINTER_CONFIG (0x01)             ///< Configuration
#define ADS1X15_REG_POINTER_LOWTHRESH (0x02)          ///< Low threshold
#define ADS1X15_REG_POINTER_HITHRESH (0x03)           ///< High threshold
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
#define ADS1X15_REG_CONFIG_OS_MASK (0x8000)         ///< OS Mask
#define ADS1X15_REG_CONFIG_OS_SINGLE (0x8000)       ///< Write: Set to start a single-conversion
#define ADS1X15_REG_CONFIG_OS_BUSY (0x0000)         ///< Read: Bit = 0 when conversion is in progress
#define ADS1X15_REG_CONFIG_OS_NOTBUSY (0x8000)      ///< Read: Bit = 1 when device is not performing a conversion

#define ADS1X15_REG_CONFIG_MUX_MASK (0x7000)        ///< Mux Mask
#define ADS1X15_REG_CONFIG_MUX_DIFF_0_1 (0x0000)    ///< Differential P = AIN0, N = AIN1 (default)
#define ADS1X15_REG_CONFIG_MUX_DIFF_0_3 (0x1000)    ///< Differential P = AIN0, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_DIFF_1_3 (0x2000)    ///< Differential P = AIN1, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_DIFF_2_3 (0x3000)    ///< Differential P = AIN2, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_SINGLE_0 (0x4000)    ///< Single-ended AIN0
#define ADS1X15_REG_CONFIG_MUX_SINGLE_1 (0x5000)    ///< Single-ended AIN1
#define ADS1X15_REG_CONFIG_MUX_SINGLE_2 (0x6000)    ///< Single-ended AIN2
#define ADS1X15_REG_CONFIG_MUX_SINGLE_3 (0x7000)    ///< Single-ended AIN3

#define ADS1X15_REG_CONFIG_PGA_MASK (0x0E00)        ///< PGA Mask
#define ADS1X15_REG_CONFIG_PGA_6_144V (0x0000)      ///< +/-6.144V range = Gain 2/3
#define ADS1X15_REG_CONFIG_PGA_4_096V (0x0200)      ///< +/-4.096V range = Gain 1
#define ADS1X15_REG_CONFIG_PGA_2_048V (0x0400)      ///< +/-2.048V range = Gain 2 (default)
#define ADS1X15_REG_CONFIG_PGA_1_024V (0x0600)      ///< +/-1.024V range = Gain 4
#define ADS1X15_REG_CONFIG_PGA_0_512V (0x0800)      ///< +/-0.512V range = Gain 8
#define ADS1X15_REG_CONFIG_PGA_0_256V (0x0A00)      ///< +/-0.256V range = Gain 16

#define ADS1X15_REG_CONFIG_MODE_MASK (0x0100)        ///< Mode Mask
#define ADS1X15_REG_CONFIG_MODE_CONTIN (0x0000)      ///< Continuous conversion mode
#define ADS1X15_REG_CONFIG_MODE_SINGLE (0x0100)      ///< Power-down single-shot mode (default)

#define ADS1X15_REG_CONFIG_RATE_MASK (0x00E0)       ///< Data Rate Mask

#define ADS1X15_REG_CONFIG_CMODE_MASK (0x0010)      ///< CMode Mask
#define ADS1X15_REG_CONFIG_CMODE_TRAD (0x0000)      ///< Traditional comparator with hysteresis (default)
#define ADS1X15_REG_CONFIG_CMODE_WINDOW (0x0010)    ///< Window comparator

#define ADS1X15_REG_CONFIG_CPOL_MASK (0x0008)       ///< CPol Mask
#define ADS1X15_REG_CONFIG_CPOL_ACTVLOW (0x0000)    ///< ALERT/RDY pin is low when active (default)
#define ADS1X15_REG_CONFIG_CPOL_ACTVHI (0x0008)     ///< ALERT/RDY pin is high when active

#define ADS1X15_REG_CONFIG_CLAT_MASK (0x0004)       ///< Determines if ALERT/RDY pin latches once asserted
#define ADS1X15_REG_CONFIG_CLAT_NONLAT (0x0000)     ///< Non-latching comparator (default)
#define ADS1X15_REG_CONFIG_CLAT_LATCH (0x0004)      ///< Latching comparator

#define ADS1X15_REG_CONFIG_CQUE_MASK (0x0003)       ///< CQue Mask
#define ADS1X15_REG_CONFIG_CQUE_1CONV (0x0000)      ///< Assert ALERT/RDY after one conversions
#define ADS1X15_REG_CONFIG_CQUE_2CONV (0x0001)      ///< Assert ALERT/RDY after two conversions
#define ADS1X15_REG_CONFIG_CQUE_4CONV (0x0002)      ///< Assert ALERT/RDY after four conversions
#define ADS1X15_REG_CONFIG_CQUE_NONE (0x0003)       ///< Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

/** Gain settings */
typedef enum
{
  GAIN_TWOTHIRDS = ADS1X15_REG_CONFIG_PGA_6_144V,
  GAIN_ONE = ADS1X15_REG_CONFIG_PGA_4_096V,
  GAIN_TWO = ADS1X15_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR = ADS1X15_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT = ADS1X15_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN = ADS1X15_REG_CONFIG_PGA_0_256V
} adsGain_t;

/** Data rates */
#define RATE_ADS1015_128SPS (0x0000)  ///< 128 samples per second
#define RATE_ADS1015_250SPS (0x0020)  ///< 250 samples per second
#define RATE_ADS1015_490SPS (0x0040)  ///< 490 samples per second
#define RATE_ADS1015_920SPS (0x0060)  ///< 920 samples per second
#define RATE_ADS1015_1600SPS (0x0080) ///< 1600 samples per second (default)
#define RATE_ADS1015_2400SPS (0x00A0) ///< 2400 samples per second
#define RATE_ADS1015_3300SPS (0x00C0) ///< 3300 samples per second

#define RATE_ADS1115_8SPS (0x0000)   ///< 8 samples per second
#define RATE_ADS1115_16SPS (0x0020)  ///< 16 samples per second
#define RATE_ADS1115_32SPS (0x0040)  ///< 32 samples per second
#define RATE_ADS1115_64SPS (0x0060)  ///< 64 samples per second 
#define RATE_ADS1115_128SPS (0x0080) ///< 128 samples per second (default)
#define RATE_ADS1115_250SPS (0x00A0) ///< 250 samples per second
#define RATE_ADS1115_475SPS (0x00C0) ///< 475 samples per second
#define RATE_ADS1115_860SPS (0x00E0) ///< 860 samples per second

/**************************************************************************/
/*!
    @brief  Sensor driver for the Adafruit ADS1X15 ADC breakouts.
*/
/**************************************************************************/
class PICO_ADS1X15
{
protected:
  // Instance-specific properties
  // i2c_inst_t* i2c_bus; ///< I2C bus device
  // uint8_t pin_sda;
  // uint8_t pin_scl;
  uint8_t m_bitShift;  ///< bit shift amount
  adsGain_t m_gain;    ///< ADC gain
  uint16_t m_dataRate; ///< Data rate
  I2cController *m_i2c; /// I2c Controller Instance

public:
  int16_t readSingleEnded(uint8_t channel);
  int16_t readDifferentialA0A1();
  int16_t readDifferentialA2A3();
  void startComparatorSingleEnded(uint8_t channel, int16_t threshold);
  int16_t getLastConversionResults();
  float computeVolts(int16_t counts);
  void setGain(adsGain_t gain);
  adsGain_t getGain();
  void setDataRate(uint16_t rate);
  uint16_t getDataRate();
  uint16_t pushConfig(uint16_t config);
  void testWrite();
  bool conversionComplete();
  void writeRegister(uint8_t reg, uint16_t value);
  uint16_t readRegister(uint8_t reg);


private:
    uint8_t buffer[3];
};

/**************************************************************************/
/*!
    @brief  Sensor driver for the Adafruit ADS1015 ADC breakout.
*/
/**************************************************************************/
class ADS1X15 : public PICO_ADS1X15
{
public:
  ADS1X15(I2cController *bustInst);
};

/**************************************************************************/
/*!
    @brief  Sensor driver for the Adafruit ADS1115 ADC breakout.
*/
/**************************************************************************/
class PICO_ADS1115 : public PICO_ADS1X15
{
public:
  PICO_ADS1115(I2cController *bustInst);
};

#endif