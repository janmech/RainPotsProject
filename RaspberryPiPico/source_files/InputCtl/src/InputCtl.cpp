#include "InputCtl.h"

InputCtl::InputCtl(
    RpConfig *config,
    queue_t *message_queue,
    PIO pio_instance,
    uint out_sm,
    uint *led_pins,
    PotiCtl *ctl_0,
    PotiCtl *ctl_1)
{
    this->config = config;
    this->message_queue = message_queue;
    this->pio_instance = pio_instance;
    this->out_sm = out_sm;
    this->led_pins = led_pins;
    this->poti_ctl_0 = ctl_0;
    this->poti_ctl_1 = ctl_1;
}

void InputCtl::init()
{
    // Load Button Config from storage
    for (uint8_t i = 0; i < 6; i++)
    {
        controller_config_t controller_config;
        controller_config.index = i;
        this->config->readControllerConfig(&controller_config);
        this->button_mode[i] = controller_config.mode;
        this->button_value[i] = controller_config.value;
    }
    this->button_inc_steps = this->config->readIncSteps();
    this->controller_status = this->config->readControllerStatus();
    this->button_inc_display_zero = this->config->readIncrementDisplayZero();
    this->button_radio_group_display_zero = this->config->readRadioGroupDisplayZero();

    this->setButtonLEDsToButtonValue();
    this->updateButtonLeds();

    this->setIndicatorLedsToButtonValue();
    this->updateIndicatorLeds();
}

void InputCtl::pioListener(uint32_t raw_buttons)
{
    static alarm_id_t a_id_long_press_btn_0 = -1;
    static alarm_id_t a_id_long_press_btn_1 = -1;
    static bool old_states[6] = {false, false, false, false, false, false};

    for (uint8_t button_index = 0; button_index < 6; button_index++)
    {
        bool bit_state = BIT_ISSET(raw_buttons, button_index);
        // Only react to button state changes
        if (old_states[button_index] != bit_state)
        {
            switch (bit_state)
            {
            case true:
                this->executeButtonPress(button_index);
                if (button_index == 0)
                {
                    a_id_long_press_btn_0 = add_alarm_in_ms(LONG_PRESS_INTERVAL, InputCtl::callback_btn_0_long_press, this, true);
                }
                if (button_index == 1)
                {
                    a_id_long_press_btn_1 = add_alarm_in_ms(LONG_PRESS_INTERVAL, InputCtl::callback_btn_1_long_press, this, true);
                }
                break;

            case false:
                if (button_index == 0)
                {
                    cancel_alarm(a_id_long_press_btn_0);
                    a_id_long_press_btn_0 = -1;
                    if (this->ignore_release_btn_0)
                    {
                        this->ignore_release_btn_0 = false;
                    }
                    else
                    {
                        this->executeButtonRelease(button_index);
                    }
                }
                else if (button_index == 1)
                {
                    cancel_alarm(a_id_long_press_btn_1);
                    a_id_long_press_btn_1 = -1;
                    if (this->ignore_release_btn_1)
                    {
                        this->ignore_release_btn_1 = false;
                    }
                    else
                    {
                        this->executeButtonRelease(button_index);
                    }
                }
                else
                {
                    this->executeButtonRelease(button_index);
                }
                break;
            }
        }
        old_states[button_index] = bit_state;
    }
}

uint8_t InputCtl::getUiMode()
{
    return this->ui_mode;
}

void InputCtl::toggleUiMode(uint8_t target_mode)
{
    if (this->ui_mode == UI_MODE_PERFORM)
    {
        this->setUiMode(target_mode);
    }
    else
    {
        this->setUiMode(UI_MODE_PERFORM);
    }
}

void InputCtl::executeLongPress(uint8_t button_index)
{
    // Board Configuration for mow only by remote command because we cannot rely on all buttons being installed
    // switch (button_index)
    // {
    // case 0:
    //     this->ignore_release_btn_1 = true;
    //     this->toggleUiMode(UI_MODE_CALIBRATION);
    //     break;
    // case 1:
    //     this->ignore_release_btn_0 = true;
    //     this->toggleUiMode(UI_MODE_CONFIG);
    //     break;
    // }
}

void InputCtl::calibrate(uint8_t position)
{
    this->toggleUiMode(UI_MODE_CALIBRATION);
    busy_wait_ms(200);

    if (position == MSG_CALIBRATION_MIN)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t ctl_index_pot_0 = i + 6;
            uint8_t ctl_index_pot_1 = i + 10;

            this->poti_ctl_0->setMinFromRaw(i);
            this->poti_ctl_1->setMinFromRaw(i);

            this->config->writeControllerMin(ctl_index_pot_0, (uint32_t)poti_ctl_0->getMin(i));
            this->config->writeControllerMin(ctl_index_pot_1, (uint32_t)poti_ctl_1->getMin(i));
        }
        this->indicateSetPotiMin();
    }

    if (position == MSG_CALIBRATION_CENTER)
    {

        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t ctl_index_pot_0 = i + 6;
            uint8_t ctl_index_pot_1 = i + 10;

            this->poti_ctl_0->setCenterFromOutValue(i);
            this->poti_ctl_1->setCenterFromOutValue(i);

            this->config->writeControllerCenter(ctl_index_pot_0, (uint32_t)poti_ctl_0->getCenter(i));
            this->config->writeControllerCenter(ctl_index_pot_1, (uint32_t)poti_ctl_1->getCenter(i));
        }
        this->indicateSetPotiCenter();
    }

    if (position == MSG_CALIBRATION_MAX)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            uint8_t ctl_index_pot_0 = i + 6;
            uint8_t ctl_index_pot_1 = i + 10;

            this->poti_ctl_0->setMaxFromRaw(i);
            this->poti_ctl_1->setMaxFromRaw(i);

            this->config->writeControllerMax(ctl_index_pot_0, (uint32_t)poti_ctl_0->getMax(i));
            this->config->writeControllerMax(ctl_index_pot_1, (uint32_t)poti_ctl_1->getMax(i));
        }
        this->indicateSetPotiMax();
    }
    this->toggleUiMode(UI_MODE_CALIBRATION);
}

void InputCtl::setButtonConfig(uint8_t *button_config)
{
    this->toggleUiMode(UI_MODE_CONFIG);
    busy_wait_ms(200);
    for (uint8_t i = 0; i < MSG_CONTROLLER_MODE_DATA_BYTE_COUNT - 2; i++)
    {
        this->button_value[i] = 0;
        if (i == 1)
        {
            this->button_mode[i] = CONTROLLER_MODE_INCREMENT;
            this->button_inc_steps = button_config[i];
            this->config->writeIncSteps(this->button_inc_steps);
            this->config->writeControllerMode(1, this->button_mode[1]);
            this->setButtonLed(1, this->button_inc_steps > 1);
            this->updateButtonLeds();
            this->setIndicatorLEDsValue(this->button_inc_steps - 1);
            this->updateIndicatorLeds();
            busy_wait_ms(300);
        }
        else
        {
            uint8_t button_mode;
            switch (button_config[i])
            {
            case 0x00:
                button_mode = CONTROLLER_MODE_TOGGLE;
                break;
            case 0x01:
                button_mode = CONTROLLER_MODE_MOMENTARY;
                break;
            case 0x04:
                button_mode = CONTROLLER_MODE_RADIO_GROUP;
                break;
            default:
                button_mode = CONTROLLER_MODE_TOGGLE;
                break;
            }
            this->button_mode[i] = button_mode;
            // TODO: FIND LED PATTERN TO display CONTROLLER_MODE_RADIO_GROUP
            this->setButtonLed(i, !(bool)this->button_mode[i]);
            this->updateButtonLeds();
            this->config->writeControllerMode(i, this->button_mode[i]);
        }
        busy_wait_ms(30);
    }
    busy_wait_ms(100);
    this->toggleUiMode(UI_MODE_CONFIG);
    busy_wait_ms(200);
}

bool InputCtl::getControllerStatus(uint8_t ctl_index)
{
    bool is_active = (bool)BIT_ISSET(this->controller_status, ctl_index);
    return is_active;
}

void InputCtl::setControllerStatus(uint8_t ctl_index, bool active)
{
    switch (active)
    {
    case true:
        BIT_SET(this->controller_status, ctl_index);
        break;

    case false:
        BIT_CLR(this->controller_status, ctl_index);
        break;
    }
    this->config->writeControllerStatus(this->controller_status);
}

void InputCtl::setAllControllerStatus(uint8_t *status_bytes)
{
    this->controller_status = 0x0000;
    for (uint8_t i = 0; i < MSG_CONTROLLER_STATUS_DATA_BYTE_COUNT; i++)
    {
        bool status = (status_bytes[i] > 0x00);
        this->setControllerStatus(i, status);
    }
    // Give some visual fedeback
    bool inidator_led_status = true;
     this->setButtonLed(0, inidator_led_status);

    for (uint8_t i = 0; i < 10; i++)
    {
        this->setButtonLed(0, inidator_led_status);
        this->updateButtonLeds();
        busy_wait_ms(80);
        inidator_led_status = !inidator_led_status;
    }
    this->setButtonLEDsToButtonValue();
    this->updateButtonLeds();
}

void InputCtl::executeRemoteCommand(uint8_t *cmd_bytes)
{
    switch (cmd_bytes[0])
    {
    case MSG_CALIBRATION_MIN:
        this->calibrate(MSG_CALIBRATION_MIN);
        break;
    case MSG_CALIBRATION_CENTER:
        this->calibrate(MSG_CALIBRATION_CENTER);
        break;
    case MSG_CALIBRATION_MAX:
        this->calibrate(MSG_CALIBRATION_MAX);
        break;
    case MSG_CONTROLLER_MODE:
        uint8_t cmd_button_modes[MSG_CONTROLLER_MODE_DATA_BYTE_COUNT - 1];
        for (uint8_t i = 0; i < MSG_CONTROLLER_MODE_DATA_BYTE_COUNT - 2; i++)
        {
            cmd_button_modes[i] = cmd_bytes[i + 1];
        }
        this->setButtonConfig(cmd_button_modes);
        this->button_inc_display_zero = (bool)cmd_bytes[MSG_CONTROLLER_MODE_DATA_BYTE_COUNT - 1];
        this->config->writeIncrementDisplayZero(this->button_inc_display_zero);
        this->button_value[1] = 0;

        this->button_radio_group_display_zero = (bool)cmd_bytes[MSG_CONTROLLER_MODE_DATA_BYTE_COUNT];
        this->config->writeRadioGroupDisplayZero(this->button_radio_group_display_zero);

        this->setButtonLEDsToButtonValue();
        this->setIndicatorLedsToButtonValue();
        break;
    case MSG_CONTROLLER_STATUS:
        uint8_t cmd_ctl_states[MSG_CONTROLLER_STATUS_DATA_BYTE_COUNT];
        for (uint8_t i = 0; i < MSG_CONTROLLER_STATUS_DATA_BYTE_COUNT; i++)
        {
            cmd_ctl_states[i] = cmd_bytes[i + 1];
        }
        this->setAllControllerStatus(cmd_ctl_states);
        break;
    case MSG_SET_BUTTON_VALUES:
        for (uint8_t i = 0; i < MSG_SET_BUTTON_VALUES_DATA_BYTE_COUNT; i++)
        {
            uint8_t button_value = cmd_bytes[i + 1];
            if (!this->getControllerStatus(i) || button_value == MSG_SET_BUTTON_VALUE_UNCHANGED || this->button_mode[i] == CONTROLLER_MODE_MOMENTARY)
            {
                continue;
            }
            if (this->button_mode[i] == CONTROLLER_MODE_INCREMENT)
            {
                button_value = (button_value > this->getButtonIncrementMaxValue()) ? this->getButtonIncrementMaxValue() : button_value;
            }
            if (this->button_mode[i] == CONTROLLER_MODE_RADIO_GROUP)
            {
                if (i == 2)
                {
                    uint8_t max_radio_val = (this->button_radio_group_display_zero) ? 4 : 3;
                    button_value = (button_value > max_radio_val) ? max_radio_val : button_value;
                }
                else
                {
                    button_value = 0;
                }
            }
            this->button_value[i] = button_value;
        }
        this->setButtonLEDsToButtonValue();
        this->setIndicatorLedsToButtonValue();
        this->updateIndicatorLeds();
        this->updateButtonLeds();
        break;
    default:
        // DO NOTHING
        break;
    }
}

/*
 * Protected methods
 */

void InputCtl::executeButtonRelease(uint8_t button_index)
{
    if (this->ui_mode == UI_MODE_PERFORM && this->getControllerStatus(button_index))
    {
        uint8_t queue_button_index = button_index;
        if (this->button_mode[button_index] == CONTROLLER_MODE_TOGGLE)
        {
            this->button_value[button_index] = !(bool)this->button_value[button_index];
        }
        if (this->button_mode[button_index] == CONTROLLER_MODE_MOMENTARY)
        {
            this->button_value[button_index] = 0;
        }
        if (this->button_mode[button_index] == CONTROLLER_MODE_INCREMENT)
        {
            this->button_value[button_index] = (this->button_value[button_index] + 1) % (this->getButtonIncrementMaxValue() + 1);
            this->setIndicatorLedsToButtonValue();
            this->updateIndicatorLeds();
        }
        if (this->button_mode[button_index] == CONTROLLER_MODE_RADIO_GROUP)
        {
            // If button 2 to 5 are set to radio group. the value is stored and sent out as button 2
            queue_button_index = 2;
            uint8_t radio_group_value = button_index - 2;
            if (this->button_radio_group_display_zero)
            {
                radio_group_value = radio_group_value + 1;
                if (this->button_value[2] == button_index - 1)
                {
                    radio_group_value = 0;
                }
            }
            this->button_value[2] = radio_group_value;
        }
        this->pushBottonStateToQueue(queue_button_index);
        this->setButtonLEDsToButtonValue();
        this->updateButtonLeds();
    }

    // Calibration and configuration only supported by remote command for now
}

void InputCtl::executeButtonPress(uint8_t button_index)
{
    if (this->ui_mode == UI_MODE_PERFORM)
    {
        if (this->button_mode[button_index] == CONTROLLER_MODE_MOMENTARY)
        {
            this->button_value[button_index] = 1;
            this->pushBottonStateToQueue(button_index);
            this->setButtonLEDsToButtonValue();
            this->updateButtonLeds();
        }
    }
}

uint8_t InputCtl::getButtonValue(uint8_t button_index)
{
    return this->button_value[button_index];
}

void InputCtl::pushBottonStateToQueue(uint8_t button_index)
{
    queue_entry_t q_entry;
    q_entry.index = button_index;
    switch (this->button_mode[button_index])
    {
    case CONTROLLER_MODE_MOMENTARY:
    case CONTROLLER_MODE_TOGGLE:
        q_entry.value = this->getButtonValue(button_index) * 511;
        break;

    case CONTROLLER_MODE_INCREMENT:
        q_entry.value = (1. / ((double)this->getButtonIncrementMaxValue())) * this->getButtonValue(button_index) * 511;
        break;
    case CONTROLLER_MODE_RADIO_GROUP:
        double devisor = (this->button_radio_group_display_zero) ? 4. : 3.;
        q_entry.value = (1. / (devisor)) * this->getButtonValue(2) * 511;
        break;
    }

    queue_add_blocking(this->message_queue, &q_entry);
}

uint8_t InputCtl::getButtonIncrementMaxValue()
{
    uint8_t max_val = this->button_inc_steps - 1;
    // Without including 'zero' we have one step less as maximum steps
    max_val = (!this->button_inc_display_zero && max_val > 3) ? 3 : max_val;
    return max_val;
}

void InputCtl::setButtonLed(uint8_t index, bool state)
{
    switch (state)
    {
    case true:
        BIT_SET(this->button_led_states, index);
        break;

    case false:
        BIT_CLR(this->button_led_states, index);
        break;
    }
}

void InputCtl::setButtonLEDsToButtonValue()
{

    for (uint8_t i = 0; i < 6; i++)
    {
        bool state = (bool)this->button_value[i];
        // On the increment button, when we do  not display zero, the buttlon led is always off
        if (i == 1 && !this->button_inc_display_zero)
        {
            state = false;
        }
        if (this->button_mode[2] == CONTROLLER_MODE_RADIO_GROUP && (i >= 2 && i <= 5))
        {
            if (this->button_radio_group_display_zero)
            {
                state = this->button_value[2] == i - 1;
            }
            else
            {
                state = this->button_value[2] == i - 2;
            }
        }
        this->setButtonLed(i, state);
    }
}

void InputCtl::updateButtonLeds()
{
    pio_sm_put(this->pio_instance, this->out_sm, this->button_led_states);
}

void InputCtl::setIndicatorLedsToButtonValue()
{

    this->indicator_led_value = (this->button_inc_display_zero) ? this->getButtonValue(1) : this->getButtonValue(1) + 1;
}

void InputCtl::setIndicatorLEDsValue(uint8_t value)
{
    this->indicator_led_value = value;
}

void InputCtl::updateIndicatorLeds()
{
    bool led_off_value = false;
    bool led_on_value = true;
    // When in UI_MODE_CONFIG indicator LEDS are inverted
    if (this->ui_mode == UI_MODE_CONFIG)
    {
        led_off_value = !led_off_value;
        led_on_value = !led_on_value;
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        gpio_put(this->led_pins[i], led_off_value);
    }
    if (this->indicator_led_value > 0)
    {
        gpio_put(this->led_pins[this->indicator_led_value - 1], led_on_value);
    }
}

void InputCtl::setUiMode(uint8_t ui_mode)
{
    this->indicateModeChange();
    this->ui_mode = ui_mode;
    switch (this->ui_mode)
    {
    case UI_MODE_CONFIG:
        this->setIndicatorLEDsValue(this->button_inc_steps - 1);
        this->updateIndicatorLeds();
        for (uint8_t i = 0; i < 6; i++)
        {
            bool led_status;
            if (i == 1)
            {
                led_status = true;
            }
            else
            {
                led_status = !(bool)this->button_mode[i];
            }
            this->setButtonLed(i, led_status);
        }
        this->updateButtonLeds();
        break;

    case UI_MODE_CALIBRATION:
        this->setIndicatorLEDsValue(0);
        this->updateIndicatorLeds();
        for (uint8_t i = 1; i < 6; i++)
        {
            this->setButtonLed(i, 0);
        }
        this->setButtonLed(0, 1);
        this->updateButtonLeds();
        break;
    case UI_MODE_PERFORM:
        this->setButtonLEDsToButtonValue();
        this->updateButtonLeds();

        this->setIndicatorLedsToButtonValue();
        this->updateIndicatorLeds();
        break;
    }
}

void InputCtl::setAllIndicatorLEDs(bool state)
{
    for (uint8_t j = 0; j < 4; j++)
    {
        gpio_put(this->led_pins[j], state);
    }
}

void InputCtl::indicateModeChange()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        this->setAllIndicatorLEDs(true);
        busy_wait_ms(100);
        this->setAllIndicatorLEDs(false);
        busy_wait_ms(100);
    }
}

void InputCtl::indicateSetPotiMin()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        gpio_put(this->led_pins[0], 1);
        gpio_put(this->led_pins[1], 1);
        busy_wait_ms(100);
        this->setAllIndicatorLEDs(false);
        busy_wait_ms(100);
    }
}

void InputCtl::indicateSetPotiMax()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        gpio_put(this->led_pins[2], 1);
        gpio_put(this->led_pins[3], 1);
        busy_wait_ms(100);
        this->setAllIndicatorLEDs(false);
        busy_wait_ms(100);
    }
}

void InputCtl::indicateSetPotiCenter()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        gpio_put(this->led_pins[1], 1);
        gpio_put(this->led_pins[2], 1);
        busy_wait_ms(100);
        this->setAllIndicatorLEDs(false);
        busy_wait_ms(100);
    }
}
