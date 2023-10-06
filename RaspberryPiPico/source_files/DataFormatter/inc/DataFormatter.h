#ifndef __DATA_FOTMATTER_H__
#define __DATA_FOTMATTER_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __BIT_MACROS__
#define __BIT_MACROS__
#define BIT_SET(BF, N) BF |= ((uint32_t)0x01 << N)
#define BIT_CLR(BF, N) BF &= ~((uint32_t)0x01 << N)
#define BIT_ISSET(BF, N) ((BF >> N) & 0x01)
#define BIT_TGL(BF, N) BF ^= ((uint8_t)0x01 << N))
#endif


#ifndef __MIDI_BYTE_MASKS__
#define __MIDI_BYTE_MASKS__
#define MIDI_MASK_STAUS_CC (0xB0)
#endif
#define BIT_MASK_0_7  (0b0000000001111111)
#define BIT_MASK_8_14 (0b0011111110000000)

class DataFormatter
{
protected:
    uint8_t board_index = 0;
    void _printBitField32(uint32_t bitField);
     void _printBitField16(uint16_t bitField);
    void _printBitField8(uint8_t bitField, bool linbreak = false);

public:
    DataFormatter(uint8_t board_index);
    void formatData(uint8_t cc_num, uint16_t value, uint8_t *formatted);
    void test();

};

#endif
