#include "RainPots.h"

/**
 * @brief
 *
 */
void __isr pio_handler()
{
    if (pio0_hw->irq & 1)
    {
        pio_interrupt_clear(pio_in_out, 0x00);

        uint32_t raw_buttons = pio_sm_get(pio_in_out, sm_in);

        // Stop State machine while processing input to prevent other interrupts
        pio_sm_set_enabled(pio_in_out, sm_in, false);
        inputCtl->pioListener(raw_buttons);
        // Re-start State Machine
        pio_sm_set_enabled(pio_in_out, sm_in, true);
    }
}

/**
 * @brief entry poinmt core1
 *
 */
void main_core1()
{
    // initialize uart0
    gpio_set_function(UART_TX_PIN_NEXT_POT, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN_NEXT_POT, GPIO_FUNC_UART);
    uart_init(uart0, 2400);
    sleep_ms(100);
    uart_set_baudrate(uart0, BAUD_RATE_INTERCOM);
    uart_set_hw_flow(uart0, false, false);
    uart_set_fifo_enabled(uart0, true);
    irq_set_enabled(UART0_IRQ, false);
    uart_set_irq_enables(uart0, false, false);

    // initialize uart1
    gpio_set_function(UART_TX_PIN_PREV_POT, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN_PREV_POT, GPIO_FUNC_UART);
    uart_init(uart1, 2400);
    sleep_ms(100);
    uart_set_baudrate(uart1, BAUD_RATE_INTERCOM);
    uart_set_hw_flow(uart1, false, false);
    uart_set_fifo_enabled(uart1, true);
    irq_set_enabled(UART1_IRQ, false);
    uart_set_irq_enables(uart1, false, false);

    sleep_ms(100);

    queue_entry_t entry;
    DataFormatter dataFormatter(board_index);
    uint8_t formatted_data[4] = {0, 0, 0, 0};

    bool collect_bytes_from_prev = false;
    uint8_t packet_forward[4] = {0, 0, 0, 0};
    uint8_t packet_forward_index = 8;

    uint8_t queue_sent = 0;
    uint8_t forward_sent = 0;

    while (true)
    {
        while (!queue_is_empty(&message_queue))
        {
            queue_remove_blocking(&message_queue, &entry);
#ifdef DEBUG
            printf("%d - %d\n", entry.index, entry.value);
#endif
            // Format to 'MIDI-like' bytes
            dataFormatter.formatData(entry.index, entry.value, formatted_data);
            for (uint8_t i = 0; i < 4; i++)
            {
                uart_putc(uart0, formatted_data[i]);
            }
            queue_sent++;
            // if (queue_sent > 50)
            // { // Make sure that we don't block the forwarding
            //     queue_sent = 0;
            //     break;
            // }
        }

        while (uart_is_readable(uart1))
        {
            uint8_t c = uart_getc(uart1);
            // // Check For status byte
            if ((c & 0xf0) == MIDI_MASK_STAUS_CC)
            {
                collect_bytes_from_prev = true;
                packet_forward_index = 0;
                packet_forward[0] = c;
                packet_forward[1] = 0;
                packet_forward[2] = 0;
                packet_forward[3] = 0;
            }
            else if (collect_bytes_from_prev)
            {
                if (packet_forward_index < 4)
                {
                    packet_forward_index++;
                    packet_forward[packet_forward_index] = c;
                }
                if (packet_forward_index == 3)
                {
                    // send packet
                    collect_bytes_from_prev = false;

                    for (uint8_t i = 0; i < 4; i++)
                    {
                        uart_putc(uart0, packet_forward[i]);
                    }
                }
            }
        }
    }
}

/**
 * @brief
 *
 * @return int
 */
int main()
{
    stdio_init_all();
    sleep_ms(500);
    core_0_init_led_pins();
    core_0_init_board_index();
    // sleep_ms(100 + (1000 * (15 - board_index) + 1));
    sleep_ms(100);
    core_0_start_up_sequence();

    gpio_set_function(4, GPIO_FUNC_NULL);
    gpio_set_function(5, GPIO_FUNC_NULL);

    I2cController i2cController_0(i2c0, 400 * 1000, PIN_I2C_0_SDA, PIN_I2C_0_SCL, true);
    i2cController_0.init();
    sleep_ms(200);

    I2cController i2cController_1(i2c1, 400 * 1000, PIN_I2C_1_SDA, PIN_I2C_1_SCL, false);
    i2cController_1.init();
    sleep_ms(200);

    // INIT SHIFT_IN_OUT PIO
    sm_in = pio_claim_unused_sm(pio_in_out, true);
    sm_out = pio_claim_unused_sm(pio_in_out, true);

    irq_set_exclusive_handler(PIO0_IRQ_0, pio_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
    pio0_hw->inte0 = pio0_hw->inte0 = PIO_IRQ0_INTE_SM0_BITS | PIO_IRQ0_INTE_SM1_BITS;
    uint pio_in_out_offset = pio_add_program(pio_in_out, &shift_in_out_program);

    shift_in_out_program_init(pio_in_out, sm_in, pio_in_out_offset, SHIFT_IN_BASE_PIN, true);
    shift_in_out_program_init(pio_in_out, sm_out, pio_in_out_offset, SHIFT_OUT_BASE_PIN, false);

    // Analog to digital converters
    ADS1X15 adcObj_0(&i2cController_0);
    adcObj_0.setDataRate(RATE_ADS1015_3300SPS);
    adcObj_0.setGain(GAIN_TWOTHIRDS);
    adc_0 = &adcObj_0;

    ADS1X15 adcObj_1(&i2cController_1);
    adcObj_1.setDataRate(RATE_ADS1015_3300SPS);
    adcObj_1.setGain(GAIN_TWOTHIRDS);
    adc_1 = &adcObj_1;

    Eeprom24LC32 storageObj(&i2cController_0);
    storage = &storageObj;

    RpConfig configObj(storage, FORCE_CONFIG_INIT);
    config = &configObj;

    PotiCtl potiCtl_0(adc_0, config, 6);
    potiCtl_0.init();
    PotiCtl potiCtl_1(adc_1, config, 10);
    potiCtl_1.init();

    InputCtl inputCtlObj(config, &message_queue, pio_in_out, sm_out, led_pins, &potiCtl_0, &potiCtl_1);
    inputCtl = &inputCtlObj;
    inputCtl->init();

    queue_init(&message_queue, sizeof(queue_entry_t), 300);
    queue_init(&callback_queue, sizeof(queue_entry_t), 50);

    multicore_launch_core1(main_core1);
    sleep_ms(100);

    uint8_t remote_command_bytes[REMOTE_COMMAND_MAX_DATA_BYTES];
    init_remote_data_bytes_array(remote_command_bytes);

    uint8_t remote_command_bytes_usb[REMOTE_COMMAND_MAX_DATA_BYTES];
    init_remote_data_bytes_array(remote_command_bytes_usb);

    bool msg_collect_bytes = false;
    uint8_t msg_byte_index = 0;
    uint8_t msg_byte_length = 0;

    bool msg_collect_bytes_usb = false;
    uint8_t msg_byte_index_usb = 0;
    uint8_t msg_byte_length_usb = 0;

    while (true)
    {
        // Read USB input

        int chrUsb = getchar_timeout_us(0);
        if (chrUsb != PICO_ERROR_TIMEOUT)
        {
            if (!msg_collect_bytes_usb)
            {
                if ((chrUsb >> 4) == 0x0F)
                {
                    putchar(0x01); // ASCII for: Start of Header (SOH)
                    init_remote_data_bytes_array(remote_command_bytes_usb);
                    msg_collect_bytes_usb = true;
                    msg_byte_index_usb = 0;
                    msg_byte_length_usb = 0;
                }
            }
            else
            {
                if (msg_byte_index_usb == 0)
                {
                    remote_command_bytes_usb[msg_byte_index_usb] = chrUsb;
                    switch (remote_command_bytes_usb[msg_byte_index_usb])
                    {
                    case MSG_CONTROLLER_MODE:
                        msg_byte_length_usb = MSG_CONTROLLER_MODE_DATA_BYTE_COUNT + 1;
                        break;
                    case MSG_CONTROLLER_STATUS:
                        msg_byte_length_usb = MSG_CONTROLLER_STATUS_DATA_BYTE_COUNT + 1;
                        break;
                    case MSG_SET_BUTTON_VALUES:
                        msg_byte_length_usb = MSG_SET_BUTTON_VALUES_DATA_BYTE_COUNT + 1;
                        break;
                    default:
                        msg_byte_length_usb = MSG_DEFAULT_DATA_BYTE_COUNT + 1;
                        break;
                    }
                    msg_byte_index_usb++;
                }
                else if (msg_byte_index_usb < msg_byte_length_usb)
                {
                    remote_command_bytes_usb[msg_byte_index_usb] = chrUsb;
                    msg_byte_index_usb++;
                }
                if (msg_byte_index_usb >= msg_byte_length_usb)
                {
                    msg_collect_bytes_usb = false;
                    inputCtl->executeRemoteCommand(remote_command_bytes_usb);
                    putchar(0x04); // ASCII for: End of Transmission (EOT)
                }
            }
        }

        while (uart_is_readable(uart0))
        {
            uint8_t c = uart_getc(uart0);
            // Forward bytes to other boards in the chain
            uart_putc(uart1, c);
            if (!msg_collect_bytes)
            {
                if ((c >> 4) == 0x0F && (c & 0x0F) == board_index)
                {
                    init_remote_data_bytes_array(remote_command_bytes);
                    msg_collect_bytes = true;
                    msg_byte_index = 0;
                    msg_byte_length = 0;
                }
            }
            else
            {
                if (msg_byte_index == 0)
                {
                    remote_command_bytes[msg_byte_index] = c;
                    switch (remote_command_bytes[msg_byte_index])
                    {
                    case MSG_CONTROLLER_MODE:
                        msg_byte_length = MSG_CONTROLLER_MODE_DATA_BYTE_COUNT + 1;
                        break;
                    case MSG_CONTROLLER_STATUS:
                        msg_byte_length = MSG_CONTROLLER_STATUS_DATA_BYTE_COUNT + 1;
                        break;
                    case MSG_SET_BUTTON_VALUES:
                        msg_byte_length = MSG_SET_BUTTON_VALUES_DATA_BYTE_COUNT + 1;
                        break;
                    default:
                        msg_byte_length = MSG_DEFAULT_DATA_BYTE_COUNT + 1;
                        break;
                    }
                    msg_byte_index++;
                }
                else if (msg_byte_index < msg_byte_length)
                {
                    remote_command_bytes[msg_byte_index] = c;
                    msg_byte_index++;
                }
                if (msg_byte_index >= msg_byte_length)
                {
                    msg_collect_bytes = false;
                    inputCtl->executeRemoteCommand(remote_command_bytes);
                }
            }
        }

        // Read the two ADCs
        for (uint8_t channel_index = 0; channel_index < 4; channel_index++)
        {
            if (inputCtl->getControllerStatus(channel_index + potiCtl_0.getChannelStartIndex()))
            {
                if (potiCtl_0.uppdate(channel_index))
                {
                    if (inputCtl->getUiMode() == UI_MODE_PERFORM)
                    {
                        queue_entry_t q_entry;
                        q_entry.index = channel_index + 6;
                        q_entry.value = potiCtl_0.getValue(channel_index);
                        queue_add_blocking(&message_queue, &q_entry);
                    }
                }
            }
            if (inputCtl->getControllerStatus(channel_index + potiCtl_1.getChannelStartIndex()))
            {
                if (potiCtl_1.uppdate(channel_index))
                {
                    if (inputCtl->getUiMode() == UI_MODE_PERFORM)
                    {
                        queue_entry_t q_entry;
                        q_entry.index = channel_index + 10;
                        q_entry.value = potiCtl_1.getValue(channel_index);
                        queue_add_blocking(&message_queue, &q_entry);
                    }
                }
            }
        }
    }
}

void core_0_init_led_pins()
{
    // INIT LED PINS
    gpio_init(PIN_LED_0);
    gpio_set_dir(PIN_LED_0, GPIO_OUT);
    gpio_init(PIN_LED_1);
    gpio_set_dir(PIN_LED_1, GPIO_OUT);
    gpio_init(PIN_LED_2);
    gpio_set_dir(PIN_LED_2, GPIO_OUT);
    gpio_init(PIN_LED_3);
    gpio_set_dir(PIN_LED_3, GPIO_OUT);
}

void core_0_init_board_index()
{
    // INIT Board Index Pins
    gpio_init(PIN_JP_INDEX_0);
    gpio_set_dir(PIN_JP_INDEX_0, GPIO_IN);
    gpio_pull_up(PIN_JP_INDEX_0);

    gpio_init(PIN_JP_INDEX_1);
    gpio_set_dir(PIN_JP_INDEX_1, GPIO_IN);
    gpio_pull_up(PIN_JP_INDEX_1);

    gpio_init(PIN_JP_INDEX_2);
    gpio_set_dir(PIN_JP_INDEX_2, GPIO_IN);
    gpio_pull_up(PIN_JP_INDEX_2);

    gpio_init(PIN_JP_INDEX_3);
    gpio_set_dir(PIN_JP_INDEX_3, GPIO_IN);
    gpio_pull_up(PIN_JP_INDEX_3);

    sleep_ms(100);
    // read the board index
    bool jumpers[4] = {
        false,
        false,
        false,
        false,
    };
    jumpers[0] = !gpio_get(PIN_JP_INDEX_0);
    jumpers[1] = !gpio_get(PIN_JP_INDEX_1);
    jumpers[2] = !gpio_get(PIN_JP_INDEX_2);
    jumpers[3] = !gpio_get(PIN_JP_INDEX_3);

    board_index = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        board_index = board_index + (uint8_t)jumpers[i] * pow(2, i);
    }

#ifdef DEBUG
    printf("Jumpers: ");
    for (uint8_t i = 0; i < 4; i++)
    {
        printf("%d", jumpers[i]);
    }
    printf("\t SUM: %d", board_index);
    printf("\n\n");
#endif
}

void core_0_start_up_sequence()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        gpio_put(led_pins[i], 1);
        sleep_ms(200);
    }
    sleep_ms(200);
    for (uint8_t i = 0; i < 4; i++)
    {
        gpio_put(led_pins[i], 0);
        sleep_ms(200);
    }

    // Display Board index
    for (uint8_t bit_index = 0; bit_index < 4; bit_index++)
    {
        if (BIT_ISSET(board_index, bit_index))
        {
            gpio_put(led_pins[bit_index], 1);
        }
    }
    sleep_ms(1000);
    for (uint8_t i = 0; i < 4; i++)
    {
        gpio_put(led_pins[i], 1);
    }
    sleep_ms(200);
    for (uint8_t i = 0; i < 4; i++)
    {
        gpio_put(led_pins[i], 0);
    }
}

void init_remote_data_bytes_array(uint8_t *byte_array)
{
    for (uint8_t i = 0; i < REMOTE_COMMAND_MAX_DATA_BYTES; i++)
    {
        byte_array[i] = 0;
    }
}

#ifdef DEBUG
/**
 * @brief Dev/Debug helper function
 *
 * @param bitField
 */
void _printBitField(uint32_t bitField)
{
    for (size_t i = 0; i < 32; i++)
    {
        if (i % 8 == 0)
        {
            printf(" ");
        }
        if (BIT_ISSET(bitField, 31 - i))
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
    }
    printf("\n");
}
#endif