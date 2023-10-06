#include "PotiCtl.h"
// #define DEBUG

PotiCtl::PotiCtl(ADS1X15 *adc, RpConfig *config, uint8_t controller_start_index)
{
    this->adc = adc;
    this->config = config;
    this->controller_start_index = controller_start_index;
}

void PotiCtl::init()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        this->min[i] = (double)this->config->readControllerMin(i + this->controller_start_index);
        this->max[i] = (double)this->config->readControllerMax(i + this->controller_start_index);
        this->center[i] = (int)this->config->readControllerCenter(i + this->controller_start_index);
    }
}

bool PotiCtl::uppdate2(uint8_t channel_index)
{
    bool changed = false;
    uint16_t result = this->adc->readSingleEnded(channel_index);

    double capped_value = ((double)result > this->raw_val[channel_index] + ADC_OUT_MAX_STEP) ? (double)result + ADC_OUT_MAX_STEP : (double)result;
    capped_value = ((double)result < this->raw_val[channel_index] - ADC_OUT_MAX_STEP) ? (double)result - ADC_OUT_MAX_STEP : (double)result;
    capped_value = this->_clip(capped_value, 0, 2000);
    this->raw_val[channel_index] = capped_value;
    double min = this->min[channel_index];
    double max = this->max[channel_index];

    double normalized = (double)result - min;
    normalized = normalized / (max - min);
    double linearized = pow(normalized, 10);

    double smoothed = this->_smooth(linearized, this->old_0[channel_index], 0.88);
    this->old_0[channel_index] = smoothed;
    double smoothed_1 = this->_smooth(smoothed, this->old_1[channel_index], 0.88);
    this->old_1[channel_index] = smoothed_1;
    double smoothed_2 = this->_smooth(smoothed_1, this->old_2[channel_index], 0.88);
    this->old_2[channel_index] = smoothed_2;

    double out_val = this->_map(smoothed_2, 0., 1., 0., 511);
    // Here we filter out big jumps and linit the to a maximum step size. The max step size has to be highter than the ADC_UNLOCK_THRESH
    out_val = (out_val > this->out_val[channel_index] + ADC_OUT_MAX_STEP) ? this->out_val[channel_index] + ADC_OUT_MAX_STEP : out_val;
    out_val = (out_val < this->out_val[channel_index] - ADC_OUT_MAX_STEP) ? this->out_val[channel_index] - ADC_OUT_MAX_STEP : out_val;

    // Last step in optaining a clean outpout value:
    // Here we filter out the small value changes that can occur even if the poti is not touched.
    // After reading x amount of times the same value we 'lock' the output, meaning all value changes smaller than defined in ADC_UNLOCK_THRESH
    // will be ignored. The trade of is that when passing from 'not turning potentiometer' to 'turning potentiomer' we lose a bit of resolution
    // for the first new value. But we consider stable values when not turning the potentiomer as more important
    if (out_val == this->out_val[channel_index] && !this->locked[channel_index])
    {
        this->adc_same_val_count[channel_index] = this->adc_same_val_count[channel_index] + 1;
        if (this->adc_same_val_count[channel_index] >= ADC_LOCK_THRESH)
        {
            this->locked[channel_index] = true;
        }
    }
    if (this->locked[channel_index])
    {
        if (abs(out_val - this->out_val[channel_index]) > ADC_UNLOCK_THRESH)
        {
            this->out_val[channel_index] = out_val;
            this->locked[channel_index] = false;
            this->adc_same_val_count[channel_index] = 0;
            changed = true;
        }
    }
    else
    {
        changed = this->out_val[channel_index] != out_val;
        this->out_val[channel_index] = out_val;
    }
    return changed;
}

bool PotiCtl::uppdate(uint8_t channel_index)
{
    bool changed = false;
    uint16_t old_centered_val = this->out_val_centered[channel_index];
    uint16_t result = this->adc->readSingleEnded(channel_index);
    this->raw_val[channel_index] = (double)result;
    double min = this->min[channel_index];
    double max = this->max[channel_index];

    // Map raw value to output nrange (0 - 511) taking the calibration limites into account
    long mapped = this->_map((double)result, min, max, 0, 511);
    // mapped = this->_clip(mapped, 0, 511);
    //  Normalize value for smoothing
    double normalized = (double)mapped / 511.;
    // Exponential average smoothing. We smooth two time with a lower smooting value instead of one time with a high smoothing value
    // This way we achieve a similar smoothing efferct, but much lower latenency
    double smoothed = this->_smooth(normalized, this->old_0[channel_index], 0.93);
    this->old_0[channel_index] = smoothed;
    double smoothed_1 = this->_smooth(smoothed, this->old_1[channel_index], 0.9);
    this->old_1[channel_index] = smoothed_1;
    double smoothed_2 = this->_smooth(smoothed_1, this->old_2[channel_index], 0.88);
    this->old_2[channel_index] = smoothed_2;

    // Re-linearize value: This is to compensate for the effebct of the hardware smoothing on the board
    // and the exponential  avareage smoothing from above. The value is obtained by trial an error
    double linearized = pow(smoothed_1, 9);

    // Map back to integer output range 0 - 511
    int out_val = (int)round(linearized * 511);

    // Here we filter out big jumps and linit the to a maximum step size. The max step size has to be highter than the ADC_UNLOCK_THRESH
    out_val = (out_val > this->out_val[channel_index] + ADC_OUT_MAX_STEP) ? this->out_val[channel_index] + ADC_OUT_MAX_STEP : out_val;
    out_val = (out_val < this->out_val[channel_index] - ADC_OUT_MAX_STEP) ? this->out_val[channel_index] - ADC_OUT_MAX_STEP : out_val;

    // Last step in optaining a clean outpout value:
    // Here we filter out the small value changes that can occur even if the poti is not touched.
    // After reading x amount of times the same value we 'lock' the output, meaning all value changes smaller than defined in ADC_UNLOCK_THRESH
    // will be ignored. The trade of is that when passing from 'not turning potentiometer' to 'turning potentiomer' we lose a bit of resolution
    // for the first new value. But we consider stable values when not turning the potentiomer as more important
    if (out_val == this->out_val[channel_index] && !this->locked[channel_index])
    {
        this->adc_same_val_count[channel_index] = this->adc_same_val_count[channel_index] + 1;
        if (this->adc_same_val_count[channel_index] >= ADC_LOCK_THRESH)
        {
            this->locked[channel_index] = true;
        }
    }
    if (this->locked[channel_index])
    {
        if (abs(out_val - this->out_val[channel_index]) > ADC_UNLOCK_THRESH)
        {
            this->out_val[channel_index] = out_val;
            this->locked[channel_index] = false;
            this->adc_same_val_count[channel_index] = 0;
            changed = true;
        }
    }
    else
    {
        changed = this->out_val[channel_index] != out_val;
        this->out_val[channel_index] = out_val;
    }
    this->out_val_centered[channel_index] = this->centerValue(channel_index);
    return old_centered_val != this->out_val_centered[channel_index];
    // return changed && (this->out_val_centered[channel_index] != (uint16_t)this->out_val[channel_index]);
}

uint16_t PotiCtl::centerValue(uint8_t channel_index)
{
    uint16_t out_val = (uint16_t)this->out_val[channel_index];
    uint16_t center_val = (uint16_t)this->center[channel_index];
    // We have a center value set. Let's remap around a dead zone;
    if (this->center[channel_index] > 0)
    {
        if (out_val > center_val + CENTER_LOCK_MARGIN)
        {
            out_val = (uint16_t)this->_map(out_val, center_val + CENTER_LOCK_MARGIN, 511, 255, 511);
        }
        else if (out_val < center_val - CENTER_LOCK_MARGIN)
        {
            out_val = (uint16_t)this->_map(out_val, 0, center_val - CENTER_LOCK_MARGIN, 0, 255);
        }
        else
        {
            out_val = 255;
        }
    }
    return out_val;
}

uint16_t PotiCtl::getValue(uint8_t channel_index)
{
    return this->out_val_centered[channel_index];

    // uint16_t out_val = (uint16_t)this->out_val[channel_index];
    // uint16_t center_val = (uint16_t)this->center[channel_index];
    // // We have a center value set. Let's remap around a dead zone;
    // if (this->center[channel_index] > 0)
    // {
    //     if (out_val > center_val + CENTER_LOCK_MARGIN)
    //     {
    //         out_val = (uint16_t)this->_map(out_val, center_val + CENTER_LOCK_MARGIN, 511, 255, 511);
    //     }
    //     else if (out_val < center_val - CENTER_LOCK_MARGIN)
    //     {
    //         out_val = (uint16_t)this->_map(out_val, 0, center_val - CENTER_LOCK_MARGIN, 0, 255);
    //     }
    //     else
    //     {
    //         out_val = 255;
    //     }
    // }
    // return out_val;
}

double PotiCtl::getRawValue(uint8_t channel_index)
{
    return this->raw_val[channel_index];
}

void PotiCtl::setMinFromRaw(uint8_t controller_index, double margin)
{
    double with_marging = this->_clip(this->getRawValue(controller_index) + margin, 0, 1500);
#ifdef DEBUG
    printf(
        "setMinFromRaw: INDEX: %d (%d)\n raw: %f, with magin: %f\n",
        controller_index,
        controller_index + this->controller_start_index,
        this->getRawValue(controller_index),
        with_marging);
#endif
    this->min[controller_index] = with_marging;
}

void PotiCtl::setCenterFromRaw(uint8_t controller_index)
{
    this->center[controller_index] = this->getRawValue(controller_index);
}

void PotiCtl::setMaxFromRaw(uint8_t controller_index, double margin)
{
    double with_marging = this->_clip(this->getRawValue(controller_index) - margin, 0., 1500.);
#ifdef DEBUG
    printf(
        "setMaxFromRaw: INDEX: %d (%d)\n raw: %f, with magin: %f\n",
        controller_index,
        controller_index + this->controller_start_index,
        this->getRawValue(controller_index),
        with_marging);
#endif
    this->max[controller_index] = with_marging;
}

void PotiCtl::setCenterFromOutValue(uint8_t controller_index)
{
    int center_val = this->out_val[controller_index];
    center_val = (center_val > 150) ? center_val : 0; // When outval < 150 we do not set a center value
    this->center[controller_index] = center_val;
#ifdef DEBUG
    printf(
        "setCenterFromOutValue: INDEX: %d (%d)\n value: %df\n",
        controller_index,
        controller_index + this->controller_start_index,
        this->center[controller_index]);
#endif
}

double PotiCtl::getMin(uint8_t controller_index)
{
    return this->min[controller_index];
}

double PotiCtl::getCenter(uint8_t controller_index)
{
    return this->center[controller_index];
}

double PotiCtl::getMax(uint8_t controller_index)
{
    return this->max[controller_index];
}


uint8_t PotiCtl::getChannelStartIndex() {
    return this->controller_start_index;
}

// PRIVATE Functions
long PotiCtl::_clip(double x, double min, double max)
{
    long x_rounded = round(x);
    x_rounded = (x_rounded < min) ? min : x_rounded;
    x_rounded = (x_rounded > max) ? max : x_rounded;
    return x_rounded;
}

long PotiCtl::_map(double x, double in_min, double in_max, double out_min, double out_max)
{
    double mapped = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    mapped = round(_clip(mapped, out_min, out_max));
    return mapped;
}

double PotiCtl::_smooth(double new_val, double old_val, double smooth)
{
    // y(k) = (1-b)*x(k) + b*y(k-1)
    return (1 - smooth) * new_val + smooth * old_val;
}