#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/util/queue.h"
#include "I2cController.h"
#include "24LC32.h"
#include "ADS1X15.h"
#include "PotiCtl.h"
#include "InputCtl.h"
#include "24LC32.h"
#include "DataFormatter.h"
#include "shift_in_out.pio.h"

// #define DEBUG

#ifndef __BIT_MACROS__
#define __BIT_MACROS__
#define BIT_SET(BF, N) BF |= ((uint32_t)0x01 << N)
#define BIT_CLR(BF, N) BF &= ~((uint32_t)0x01 << N)
#define BIT_ISSET(BF, N) ((BF >> N) & 0x01)
#define BIT_TGL(BF, N) BF ^= ((uint8_t)0x01 << N))
#endif

#ifndef __UI_MODES__
#define __UI_MODES__
#define UI_MODE_PERFORM 0
#define UI_MODE_CONFIG 1
#define UI_MODE_CALIBRATION 2
#endif

#define FORCE_CONFIG_INIT false

// Directly driven LEDs
#define PIN_LED_0 2
#define PIN_LED_1 3
#define PIN_LED_2 22
#define PIN_LED_3 26

// I2C Pins
#define PIN_I2C_0_SDA 8
#define PIN_I2C_0_SCL 9

#define PIN_I2C_1_SDA 6
#define PIN_I2C_1_SCL 7

#define PIN_JP_INDEX_0 21
#define PIN_JP_INDEX_1 20
#define PIN_JP_INDEX_2 19
#define PIN_JP_INDEX_3 18

#define UART_TX_PIN_NEXT_POT 16
#define UART_RX_PIN_NEXT_POT 17

#define UART_TX_PIN_PREV_POT 4
#define UART_RX_PIN_PREV_POT 5

#define BAUD_RATE_INTERCOM 380400

#define SHIFT_IN_BASE_PIN 13
// Pin Mapping SHIFT IN
// +===================+=========+===========+==========+
// | PIN FUNC          | PIN INDEX | PIN DIR | PIN TYPE |
// +===================+===========+=========+==========+
// | Shift IN PL       | base      |  OUT    |   SET    |
// +-------------------+-----------+---------+----------+
// | Shift IN CLK      | base + 1  |  OUT    |   SET    |
// +-------------------+-----------+---------+----------+
// | Shift IN DATA     | base + 2  |  IN     |   IN     |
// +-------------------+-----------+---------+----------+

#define SHIFT_OUT_BASE_PIN 10
// Pin Mapping SHIFT OUT
// +===================+==========+===========+=========+
// | PIN FUNC          | PIN TYPE | PIN INDEX | PIN DIR |
// +===================+==========+===========+=========+
// | Shift OUT LATCH   |    SET   |  base     |   OUT   |
// +-------------------+----------+-----------+---------+
// | Shift OUT CLK     |    SET   |  base + 1 |   OUT   |
// +-------------------+----------+-----------+---------+
// | Shift OUT DATA    |    OUT   |  base + 2 |   OUT   |
// +-------------------+----------+-----------+---------+


#define REMOTE_COMMAND_MAX_DATA_BYTES 24


#ifndef __QUEUE_ENTRY_T__
#define __QUEUE_ENTRY_T__
typedef struct
{
    uint8_t index;
    uint16_t value;
} queue_entry_t;
#endif

// POI (shift in/out)
PIO pio_in_out = pio0;
uint sm_in;
uint sm_out;

uint8_t board_index;

ADS1X15 *adc_0 = NULL;
ADS1X15 *adc_1 = NULL;

Eeprom24LC32 *storage;
RpConfig *config;
InputCtl *inputCtl;

uint led_pins[4] = {PIN_LED_0, PIN_LED_1, PIN_LED_2, PIN_LED_3};

queue_t message_queue;
queue_t callback_queue;

void core_0_init_led_pins();
void core_0_init_board_index();
void core_0_start_up_sequence();
void init_remote_data_bytes_array(uint8_t *byte_array);
#ifdef DEBUG
void _printBitField(uint32_t bits);
#endif