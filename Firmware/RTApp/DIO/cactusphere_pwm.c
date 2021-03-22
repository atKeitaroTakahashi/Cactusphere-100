/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Atmark Techno, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "cactusphere_pwm.h"

#include "mt3620-gpio.h"
#include "libpwm/GPIO.h"
#include "libpwm/mt3620/gpio.h"

int32_t Cactusphere_PWM_ReadOutput(uint32_t pin, bool *state)
{
    mt3620_gpio_block_e block = pinToBlock(pin);

    if ((block < MT3620_GPIO_BLOCK_0) || (block > MT3620_GPIO_BLOCK_2)) {
        return ERROR_PWM_NOT_A_PIN;
    }

    uint32_t pinMask  = getPinMask(pin, block);
    uint32_t pwmBlock = block - MT3620_GPIO_BLOCK_0;

    bool pwm_enable = false;
    uint16_t on_time = 0;
    uint16_t off_time = 0;

    switch (pinMask) {
        case 1:
            pwm_enable = MT3620_PWM_FIELD_READ(pwmBlock, pwm0_ctrl, pwm_clock_en);
            on_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm0_param_s0, pwm_on_time);
            off_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm0_param_s0, pwm_off_time);
            break;
        case 2:
            pwm_enable = MT3620_PWM_FIELD_READ(pwmBlock, pwm1_ctrl, pwm_clock_en);
            on_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm1_param_s0, pwm_on_time);
            off_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm1_param_s0, pwm_off_time);
            break;
        case 4:
            pwm_enable = MT3620_PWM_FIELD_READ(pwmBlock, pwm2_ctrl, pwm_clock_en);
            on_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm2_param_s0, pwm_on_time);
            off_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm2_param_s0, pwm_off_time);
            break;
        case 8:
            pwm_enable = MT3620_PWM_FIELD_READ(pwmBlock, pwm3_ctrl, pwm_clock_en);
            on_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm3_param_s0, pwm_on_time);
            off_time = MT3620_PWM_FIELD_READ(pwmBlock, pwm3_param_s0, pwm_off_time);
            break;
        default:
            break;
    }

    if (!pwm_enable) {
        *state = false;
    } else if (on_time > off_time) {
        *state = true;
    } else {
        *state = false;
    }

    return ERROR_NONE;
}

int32_t Cactusphere_PWM_WriteOutput(uint32_t pin, bool state) {
    PWM_ConfigurePin(pin, MT3620_PWM_CLK_SEL_32K, state, !state);
}