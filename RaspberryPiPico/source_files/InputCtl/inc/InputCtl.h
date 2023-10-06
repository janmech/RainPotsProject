#ifndef __INPUT__CTL_H__
#define __INPUT__CTL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/pio.h"
#include "RpConfig.h"
#include "PotiCtl.h"

#ifndef __BIT_MACROS__
#define __BIT_MACROS__
#define BIT_SET(BF, N) BF |= ((uint32_t)0x01 << N)
#define BIT_CLR(BF, N) BF &= ~((uint32_t)0x01 << N)
#define BIT_ISSET(BF, N) ((BF >> N) & 0x01)
#define BIT_TGL(BF, N) BF ^= ((uint8_t)0x01 << N))
#endif

#ifndef __QUEUE_ENTRY_T__
#define __QUEUE_ENTRY_T__
typedef struct
{
    uint8_t index;
    uint16_t value;
} queue_entry_t;
#endif

#ifndef __CONTROLLER_MODES__
#define __CONTROLLER_MODES__
#define CONTROLLER_MODE_TOGGLE 0
#define CONTROLLER_MODE_MOMENTARY 1
#define CONTROLLER_MODE_INCREMENT 2
#define CONTROLLER_MODE_KNOB 3
#define CONTROLLER_MODE_RADIO_GROUP 4
#endif

#ifndef __UI_MODES__
#define __UI_MODES__
#define UI_MODE_PERFORM 0
#define UI_MODE_CONFIG 1
#define UI_MODE_CALIBRATION 2
#endif

#define LONG_PRESS_INTERVAL 1000

// External Message definitions
#define MSG_CALIBRATION_MIN 0xE0
#define MSG_CALIBRATION_CENTER 0xE1
#define MSG_CALIBRATION_MAX 0xE2
#define MSG_CONTROLLER_MODE 0xE3
#define MSG_CONTROLLER_MODE_DATA_BYTE_COUNT 8
#define MSG_CONTROLLER_STATUS 0xE4
#define MSG_CONTROLLER_STATUS_DATA_BYTE_COUNT 14
#define MSG_SET_BUTTON_VALUES 0xE5
#define MSG_SET_BUTTON_VALUES_DATA_BYTE_COUNT 6
#define MSG_SET_BUTTON_VALUE_UNCHANGED 0x0F
#define MSG_DEFAULT_DATA_BYTE_COUNT 0



class InputCtl
{
protected:
    PIO pio_instance;
    uint out_sm;
    queue_t *message_queue;
    uint8_t button_inc_steps = 5;
    bool button_inc_display_zero = true;
    bool button_radio_group_display_zero = false;
    uint32_t controller_status = 0xFFFF;
    uint *led_pins;
    RpConfig *config;
    PotiCtl *poti_ctl_0;
    PotiCtl *poti_ctl_1;
    uint8_t ui_mode = UI_MODE_PERFORM;
    uint32_t button_led_states = 0x00;
    uint8_t indicator_led_value = 0x00;
    uint8_t button_mode[6] = {CONTROLLER_MODE_TOGGLE, CONTROLLER_MODE_INCREMENT, CONTROLLER_MODE_TOGGLE, CONTROLLER_MODE_TOGGLE, CONTROLLER_MODE_TOGGLE, CONTROLLER_MODE_MOMENTARY};
    uint8_t button_value[6] = {
        0,
        0,
        0,
        0,
        0,
        0,
    };

    bool ignore_release_btn_0 = false;
    bool ignore_release_btn_1 = false;

    void executeButtonPress(uint8_t button_index);
    void executeButtonRelease(uint8_t button_index);
    // bool setButtonState(uint8_t button_index, bool state);
    uint8_t getButtonValue(uint8_t button_index);
    void setButtonLed(uint8_t index, bool state);
    void setButtonLEDsToButtonValue();
    void updateButtonLeds();
    void setIndicatorLedsToButtonValue();
    void setIndicatorLEDsValue(uint8_t value);
    void updateIndicatorLeds();
    void setUiMode(uint8_t ui_mode);
    void setAllIndicatorLEDs(bool state);
    void indicateModeChange();
    void indicateSetPotiMin();
    void indicateSetPotiMax();
    void indicateSetPotiCenter();
    void pushBottonStateToQueue(uint8_t button_index);
    uint8_t getButtonIncrementMaxValue();

public:
    InputCtl(RpConfig *config, queue_t *message_queue, PIO pio_instance, uint out_sm, uint *led_pins, PotiCtl *ctl_0, PotiCtl *ctl_1);
    void init();
    void pioListener(uint32_t raw_buttons);
    uint8_t getUiMode();
    void toggleUiMode(uint8_t target_mode);
    void executeLongPress(uint8_t button_index);
    void calibrate(uint8_t position);
    void setButtonConfig(uint8_t *button_config);
    bool getControllerStatus(uint8_t ctl_index);
    void setControllerStatus(uint8_t ctl_index, bool active);
    void setAllControllerStatus(uint8_t *status_bytes);
    void executeRemoteCommand(uint8_t *cmd_bytes);

    static int64_t callback_btn_0_long_press(alarm_id_t id, void *user_data)
    {
        InputCtl *thisCtl = reinterpret_cast<InputCtl *>(user_data);
        thisCtl->executeLongPress(0);
        return 0;
    }

    static int64_t callback_btn_1_long_press(alarm_id_t id, void *user_data)
    {
        InputCtl *thisCtl = reinterpret_cast<InputCtl *>(user_data);
        thisCtl->executeLongPress(1);
        return 0;
    }
};

#endif