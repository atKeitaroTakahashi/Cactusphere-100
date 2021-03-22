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

#include "PulseCounter.h"

#include "mt3620-gpio.h"
#include "libpwm/GPIO.h"

//
// Initialization
//
void 
PulseCounter_Initialize(PulseCounter* me, int pin)
{
    me->pinId = pin;
    me->isCountHight = true;
    me->prevState = false;
    me->currentState = false;
    me->pulseCounter = 0;
    me->pulseOnTime = 0;
    me->isSetPulse = false;
    me->minPulseSetTime = 0;
    me->pulseElapsedTime = 0;
    me->isRising = false;
    me->maxPulseCounter = 0;
    me->isStart = false;
    me->pulseOnTimeS = 0;
}

//
// Attribute
//
int
PulseCounter_GetPinId(PulseCounter* me)
{
    return me->pinId;
}

//
// Pulse counter driver operation
//
void 
PulseCounter_SetConfigCounter(PulseCounter* me,
    bool isCountHight, int minPulse, int maxPulse)
{
    me->isCountHight    = isCountHight;
    me->minPulseSetTime = minPulse;
    me->maxPulseCounter = maxPulse;
    me->prevState       = isCountHight;
    me->isRising        = !(isCountHight);
    me->isStart         = true;
}

void
PulseCounter_Clear(PulseCounter* me, int initValue)
{
    bool prevIsStart     = me->isStart;
    
    me->isStart          = false; // stop
    me->pulseCounter     = initValue;
    me->prevState        = me->isCountHight;
    me->isRising         = !(me->isCountHight);
    me->pulseOnTime      = 0;
    me->pulseOnTimeS     = 0;
    me->pulseElapsedTime = 0;
    me->isSetPulse       = false;
    if (prevIsStart) {
        me->isStart      = true; // restart
    }
}

int
PulseCounter_GetPulseCount(PulseCounter* me)
{
    return me->pulseCounter;
}

int 
PulseCounter_GetPulseOnTime(PulseCounter* me)
{
    return me->pulseOnTimeS;
}

bool 
PulseCounter_GetLevel(PulseCounter* me)
{
    return me->prevState;
}

bool 
PulseCounter_GetPinLevel(PulseCounter* me)
{
    return me->currentState;
}

//
// Handle polling based pulse counting task
//
void
PulseCounter_Counter(PulseCounter* me)
{
    bool newState;

    // check DIn pin's input level and do pulse counting task as state machine
    Mt3620_Gpio_Read(me->pinId, &newState);
    if (newState != me->prevState) {
        me->pulseElapsedTime = 0;
        me->isSetPulse = false;
        if (!me->prevState) {
            me->isRising = true;
        } else {
            me->isRising = false;
        }
        me->prevState = newState;
    } else {
        if (! me->isSetPulse) {
            me->pulseElapsedTime++;
            if (me->pulseElapsedTime > me->minPulseSetTime) {
                me->currentState = me->prevState;
                if ((me->isRising && me->isCountHight)
                ||  (!me->isRising && !me->isCountHight)) {
                    if (me->pulseCounter >= me->maxPulseCounter) {
                        me->pulseCounter = 0;
                    }
                    me->pulseCounter++;
                }
                me->pulseElapsedTime = 0;
                me->isSetPulse = true;
            }
        } else if (me->isRising) {
            me->pulseOnTime++;
            if (me->pulseOnTime > 1000) {
                me->pulseOnTimeS += me->pulseOnTime / 1000;
                me->pulseOnTime = me->pulseOnTime % 1000;
            }
        }
    }
}

void
DoController_Initialize(DoController* me, int pin, bool defaultState) {
    me->pinId = pin;
    me->defaultState = defaultState;
    me->triggerActived = false;
    me->driveState = false;
    me->pulseClock = PulseClock_32KHz;
    me->pulsePeriod = 0;
    me->pulseEffectiveTime = 0;
    me->delayEnable = false;
    me->flagDelay = false;
    me->delayTime = 0;
    me->delayElapsedTime = 0;
    me->driveCertainEnable = false;
    me->flagDriveTime = false;
    me->driveTime = 0;
    me->driveElapsedTime = 0;
    me->functionType = FunctionType_NotSelected;
    me->functionStatus = FunctionStatus_None;
    me->relationType = RelationType_NotSelected;
    me->relationPort = -1;
}

DoController_GetPinId(DoController* me)
{
    return me->pinId;
}

// !日本語のコメント、TODOは最終的に削除・変更してください!
// TODO: DOの動作に必要なパラメータをセットする処理を追加
// PulseCounter_SetConfigCounterをコピーしていますが、引数は適宜変更してください
void
DoController_SetConfig(DoController* me,
    bool isCountHight, int minPulse, int maxPulse)
{
    ;
}

// !日本語のコメント、TODOは最終的に削除・変更してください!
// DIとのトリガ条件が満たされた場合や即時実行のコマンドを受け取った際に実行する想定↓
void
DoController_Trigger_Active(DoController* me)
{
    me->triggerActived = true;
}

// DIと同じで1ms毎に実行します
void
DoController_Controller(DoController* me) {
    bool newState;

    if (me->triggerActived == true) {
        if (me->delayEnable == true) {
            me->flagDelay = true;
            me->delayElapsedTime = 0;
            me->triggerActived = false;
        }
        if (me->driveCertainEnable == true) {
            me->flagDriveTime = true;
            me->driveElapsedTime = 0;
            me->triggerActived = false;
        }
    }

    if (me->flagDelay == true) {
        if (me->delayElapsedTime >= me->delayTime) {
            me->flagDelay = false;
        } else {
            me->delayElapsedTime++;
        }
    } else if ((me->driveCertainEnable == false && me->triggerActived == true) ||
               (me->driveCertainEnable == true && me->flagDriveTime == true)) {

        me->triggerActived = false;

        if (me->functionStatus == FunctionStatus_Started) {
            if (me->functionType == FunctionType_Relation) {
                switch (me->relationType)
                {
                case RelationType_Interlock:
                    Mt3620_Gpio_Read(me->relationPort, &newState);
                    Cactusphere_PWM_WriteOutput(me->pinId, newState);
                    break;
                case RelationType_Interlock_Invert:
                    Mt3620_Gpio_Read(me->relationPort, &newState);
                    Cactusphere_PWM_WriteOutput(me->pinId, !newState);
                    break;
                default:
                    break;
                }
            }
        } else {
            if (me->functionType == FunctionType_Relation) {
                switch (me->relationType)
                {
                case RelationType_Drive:
                    Cactusphere_PWM_WriteOutput(me->pinId, me->driveState);
                    break;
                case RelationType_Invert:
                    Mt3620_Gpio_Read(me->relationPort, &newState);
                    Cactusphere_PWM_WriteOutput(me->pinId, !newState);
                    break;
                case RelationType_Interlock:
                    Mt3620_Gpio_Read(me->relationPort, &newState);
                    Cactusphere_PWM_WriteOutput(me->pinId, newState);
                    break;
                case RelationType_Interlock_Invert:
                    Mt3620_Gpio_Read(me->relationPort, &newState);
                    Cactusphere_PWM_WriteOutput(me->pinId, !newState);
                    break;
                case RelationType_Snap:
                    // TODO: 処理を追加する
                    break;
                case RelationType_Pulse:
                case RelationType_PWM:
                    PWM_ConfigurePin(me->pinId, me->pulseClock,
                    me->pulseEffectiveTime, me->pulsePeriod - me->pulseEffectiveTime);
                    break;
                default:
                    break;
                }
            } else if (me->functionType == FunctionType_Single) {
                Cactusphere_PWM_WriteOutput(me->pinId, me->driveState);
            } else if (me->functionType == FunctionType_Pulse) {
                PWM_ConfigurePin(me->pinId, me->pulseClock,
                 me->pulseEffectiveTime, me->pulsePeriod - me->pulseEffectiveTime);
            }
            me->functionStatus = FunctionStatus_Started;
        }
        if (me->flagDriveTime == true) {
            if (me->driveElapsedTime >= me->driveTime) {
                Cactusphere_PWM_WriteOutput(me->pinId, me->defaultState);
                me->flagDriveTime = false;
            } else {
                me->driveElapsedTime++;
            }
        }
    }
}
