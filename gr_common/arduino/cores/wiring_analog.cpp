/* wiring_analog.cpp */
/* Copyright (C) 2016 Nozomu Fujita, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "wiring_private.h"
#include "pins_arduino.h"
#include "pwmout_api.h"
#include "RZ_A1_Init.h"
#include "gpio_addrdefine.h"
#ifdef GRPEACH
#include "MBRZA1H.h"
#endif

static uint8_t currentAnalogReference = DEFAULT_0;

void analogReference(uint8_t mode)
{
    currentAnalogReference = mode;
}

int analogRead(PinName pinName)
{
    if (isValidPinName(pinName)) {
        setArduinoPinMode(pinName, ArduinoPinModeAnalogRead);
        int val = analogin_read_u16(&arduinoPinStatus[pinName].adc) >> 4;
        switch (currentAnalogReference) {
        case DEFAULT_0:
            val = val * (1024 * 33) / (4096 * 50);
            break;
        case INTERNAL:
            val = val * (1024 * 33) / (4096 * 11);
            if (val > 1023) {
                val = 1023;
            }
            break;
        case EXTERNAL:
            val = val * 1024 / 4096;
            break;
        case RAW12BIT:
            break;
        }
        return val;
    } else {
        return 0;
    }
}

int analogRead(uint8_t pin)
{
    if (pin < NUM_ANALOG_INPUTS) {
        pin += PIN_AN000;
    }
    PinName pinName = int2PinName(pin);
    if (pinName != NC) {
        return analogRead(pinName);
    } else {
        return 0;
    }
}

void setArduinoPinModeAnalogRead(PinName pinName)
{
    if (isValidPinName(pinName)) {
        analogin_init(&arduinoPinStatus[pinName].adc, pinName);
    }
}

/*
void resetArduinoPinModeAnalogRead(PinName pinName)
{
    if (isValidPinName(pinName)) {
        analogin_free(&arduinoPinStatus[pinName].adc, pinName);
    }
}
*/

#define MaxSoftwarePwmChannels 20
#define SoftwarePWMFreq (125 * 1000)
#define isHardwarePWMPin(pinName) \
( \
       ((pinName) == P4_7) \
    || ((pinName) == P4_6) \
    || ((pinName) == P4_5) \
    || ((pinName) == P4_4) \
    || ((pinName) == P8_13) \
    || ((pinName) == P8_11) \
    || ((pinName) == P8_15) \
    || ((pinName) == P8_14) \
)
void changePinModeSoftwarePWM(PinName pinName, int period, int term, unsigned long length);

void analogWrite(PinName pinName, int val)
{
    if (isValidPinName(pinName)) {
        if (isHardwarePWMPin(pinName)) {
            setArduinoPinMode(pinName, ArduinoPinModeHardwarePWM);
            pwmout_write(&arduinoPinStatus[pinName].pwm, (1.0f / 255.0f) * val);
        } else {
            setArduinoPinMode(pinName, ArduinoPinModeSoftwarePWM);
            changePinModeSoftwarePWM(pinName, 255, val, 0);
        }
    }
}

void analogWrite(uint8_t pin, int val)
{
    PinName pinName = int2PinName(pin);
    if (pinName != NC) {
        analogWrite(pinName, val);
    }
}

void setArduinoPinModeHardwarePWM(PinName pinName)
{
    if (isValidPinName(pinName)) {
        pwmout_init(&arduinoPinStatus[pinName].pwm, pinName);
    }
}

void resetArduinoPinModeHardwarePWM(PinName pinName)
{
    if (isValidPinName(pinName)) {
        pwmout_free(&arduinoPinStatus[pinName].pwm);
    }
}

typedef struct {
    bool valid;
    PinName pinName;
    uint16_t count;
    uint16_t period;
    uint16_t term;
    uint32_t length;
    volatile uint32_t* psr;
    uint32_t set;
    uint32_t reset;
} SoftwarePwm;

static SoftwarePwm softwarePwmTable[MaxSoftwarePwmChannels];
static void setTimer();

void setPinModeSoftwarePWM(PinName pinName, int period, int term, unsigned long length)
{
    if (isValidPinName(pinName)) {
        setTimer();
        SoftwarePwm* p;
        for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
            if (!p->valid || p->pinName == pinName) {
                setArduinoPinModeOutput(pinName);
                p->valid = false;
                p->pinName = pinName;
                p->count = 0;
                p->period = period;
                p->term = term;
                p->length = length;
                p->psr = PSR(PINGROUP(pinName));
                p->set = 0x00010001 << PINNO(pinName);
                p->reset = 0x00010000 << PINNO(pinName);
                p->valid = true;
                break;
            }
        }
    }
}

void changePinModeSoftwarePWM(PinName pinName, int period, int term, unsigned long length)
{
    if (isValidPinName(pinName)) {
        volatile SoftwarePwm* p;
        for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
            if (p->pinName == pinName) {
                p->valid = false;
                p->term = term;
                p->period = period;
                p->length = length;
                p->valid = true;
                break;
            }
        }
    }
}

void resetArduinoPinModeSoftwarePWM(PinName pinName)
{
    SoftwarePwm* p;
    for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
        if (p->valid && p->pinName == pinName) {
            p->pinName = NC;
            p->valid = false;
            digitalWrite(pinName, LOW);
            break;
        }
    }
}

void setArduinoPinModeSoftwarePWM(PinName pinName)
{
    setPinModeSoftwarePWM(pinName, 0, 0, 0);
}

static void timer(void)
{
    SoftwarePwm* p;
    for (p = &softwarePwmTable[0]; p < &softwarePwmTable[MaxSoftwarePwmChannels]; p++) {
        if (p->valid) {
            if (p->count == p->term) {
                *p->psr = p->reset;
            } else if (p->count == 0) {
                *p->psr = p->set;
            }
            if (p->length > 0 && --p->length == 0) {
                p->valid = false;
            }
            if (++p->count >= p->period) {
                p->count = 0;
            }
        }
    }
    MTU2TSR_3 &= ~0b00000001;
}

static void setTimer()
{
    static bool set = false;
    if (!set) {
        const IRQn_Type irqch = TGI3A_IRQn;
        GIC_DisableIRQ(irqch);
        InterruptHandlerRegister(irqch, timer);
        CPGSTBCR3 &= ~(CPG_STBCR3_BIT_MSTP33);
        MTU2TSTR &= ~0b01000000;
        MTU2TCR_3 = 0b00100000;
        MTU2TMDR_3 = 0b00000000;
        MTU2TIORH_3 = 0b00000000;
        MTU2TIORL_3 = 0b00000000;
        MTU2TIER_3 = 0b00000001;
        MTU2TCNT_3 = 0;
        if (RZ_A1_IsClockMode0()) {
            MTU2TGRA_3 = CM0_RENESAS_RZ_A1_P0_CLK / SoftwarePWMFreq - 1;
        } else {
            MTU2TGRA_3 = CM1_RENESAS_RZ_A1_P0_CLK / SoftwarePWMFreq - 1;
        }
        MTU2TSTR |= 0b01000000;
        GIC_EnableIRQ(irqch);
        set = true;
    }
}
