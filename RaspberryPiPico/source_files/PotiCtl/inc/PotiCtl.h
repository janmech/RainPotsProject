#ifndef __POTI__CTL_H__
#define __POTI__CTL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "ADS1X15.h"
#include "RpConfig.h"

#define ADC_CHANNEL_COUNT 4
#define ADC_LOCK_THRESH 25
#define ADC_UNLOCK_THRESH 3
#define ADC_OUT_MAX_STEP 10
#define CENTER_LOCK_MARGIN 10

class PotiCtl
{
private:
    ADS1X15 *adc;
    RpConfig *config;
    uint8_t controller_start_index = 6;

    long _clip(double x, double min, double max);
    long _map(double x, double in_min, double in_max, double out_min, double out_max);
    double _smooth(double new_val, double old_val, double smooth);

protected:
    double min[4] = {2., 1., 1., 1.};
    double max[4] = {990., 985., 975., 977.};
    int center[4] = {0, 0, 0, 0};
    double old_0[4] = {0., 0., 0., 0.};
    double old_1[4] = {0., 0., 0., 0.};
    double old_2[4] = {0., 0., 0., 0.};

    double raw_val[4] = {0, 0, 0, 0};
    int out_val[4] = {0, 0, 0, 0};
    uint16_t out_val_centered[4] = {0, 0, 0, 0};
    bool locked[4] = {false, false, false, false};
    uint8_t adc_same_val_count[4] = {0, 0, 0, 0};
    uint16_t centerValue(uint8_t controller_channel);

public:
    PotiCtl(ADS1X15 *adc, RpConfig *config, uint8_t controller_start_index);
    void init();
    bool uppdate(uint8_t channel_index);
    bool uppdate2(uint8_t channel_index);
    uint16_t getValue(uint8_t channel_index);
    double getRawValue(uint8_t channel_index);
    void setMinFromRaw(uint8_t controller_index, double margin = 3);
    void setCenterFromRaw(uint8_t controller_index);
    void setCenterFromOutValue(uint8_t controller_index);
    void setMaxFromRaw(uint8_t controller_index, double margin = 7);
    double getMin(uint8_t controller_index);
    double getCenter(uint8_t controller_index);
    double getMax(uint8_t controller_index);
    uint8_t getChannelStartIndex();
};

#endif