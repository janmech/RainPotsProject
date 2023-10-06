#include "DataFormatter.h"

DataFormatter::DataFormatter(uint8_t board_index)
{
    this->board_index = board_index;
}

void DataFormatter::formatData(uint8_t cc_num, uint16_t value, uint8_t *formatted)
{
    // Clip value to allowed maximum
    value = (value > 511) ? 511 : value;

    // byte 0 contains bits 0-7
    // byte 1 contains bits 8-14
    uint8_t byte_msb = 0x00;                // MSB
    uint8_t byte_lsb = 0x00;                // LSB
    byte_lsb = value & BIT_MASK_0_7;        // MSB
    byte_msb = (value >> 7) & BIT_MASK_0_7; // LSB

    uint8_t status_byte = (MIDI_MASK_STAUS_CC | this->board_index);
    formatted[0] = status_byte;
    formatted[1] = cc_num;
    formatted[2] = byte_lsb;
    formatted[3] = byte_msb;
}

void DataFormatter::test()
{
    // for (uint16_t value = 0; value < 512; value++)
    // {
    //     printf("Value %d:\n", value);
    //     uint8_t formatted[4] = {0, 0, 0, 0};
    //     this->formatData(4, value, formatted);
    //     printf("Status: 0x%02X\n", formatted[0]);
    //     printf("CC Num: %d\n", formatted[1]);

    //     printf("LSB: ");
    //     this->_printBitField8(formatted[2], true);
    //     printf("MSB: ");
    //     this->_printBitField8(formatted[3], true);
    //     uint16_t decoded_lsb = (uint16_t)formatted[2];
    //     uint16_t decoded_msb = ((uint16_t)formatted[3]) << 7;

    //     printf("LSB 16: ");
    //     this->_printBitField16(decoded_lsb);
    //     printf("MSB 16: ");
    //     this->_printBitField16(decoded_msb);
    //     uint16_t decoded = (uint16_t)formatted[2] | ((uint16_t)formatted[3]) << 7;
    //     printf("Value: %d\n", decoded);
    //     printf("\n");
    // }
}

// Protected Methods

void DataFormatter::_printBitField32(uint32_t bitField)
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

void DataFormatter::_printBitField16(uint16_t bitField)
{
    for (size_t i = 0; i < 16; i++)
    {
        if (i % 8 == 0)
        {
            printf(" ");
        }
        if (BIT_ISSET(bitField, 15 - i))
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

void DataFormatter::_printBitField8(uint8_t bitField, bool linebreak)
{
    for (size_t i = 0; i < 8; i++)
    {
        if (i % 8 == 0)
        {
            printf(" ");
        }
        if (BIT_ISSET(bitField, 7 - i))
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
    }
    if (linebreak)
    {
        printf("\n");
    }
}
