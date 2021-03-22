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

#ifndef _PULSE_COUNTER_H_
#define _PULSE_COUNTER_H_

#ifndef _STDBOOL_H
#include <stdbool.h>
#endif
#ifndef _STDINT_H
#include <stdint.h>
#endif
// ファイル名はDIOControllerとかのほうがいいかもしれない
// !日本語のコメント、TODOは最終的に削除・変更してください!
typedef enum {
    FunctionType_NotSelected,
    FunctionType_Single,
    FunctionType_Pulse,
    FunctionType_Relation
} FunctionType;

typedef enum {
    RelationType_NotSelected,
    RelationType_Drive,
    RelationType_Invert,
    RelationType_Interlock,
    RelationType_Interlock_Invert,
    RelationType_Snap,
    RelationType_Pulse,
    RelationType_PWM
} RelationType;

typedef enum {
    PulseClock_None = 0,
    PulseClock_32KHz = 32768,
    PulseClock_2MHz = 2000000,
    PulseClock_XTAL = 26000000
} PulseClock;

typedef enum {
    FunctionStatus_None,
    FunctionStatus_Enable,
    FunctionStatus_Started,
    FunctionStatus_Disable
} FunctionStatus;

typedef struct PulseCounter {
    int         pinId;             // DIn pin number
    int         pulseCounter;      // pulse counter value
    int         pulseOnTime;       // time integration of pulse [msec]
    int         pulseOnTimeS;      // time integration of pulse [sec]
    int         pulseElapsedTime;  // current pulse's continuation length
    uint32_t    minPulseSetTime;   // minimum length for settlement as pulse
    uint32_t    maxPulseCounter;   // max pulse counter value
    bool        isCountHight;      // whether settlement as pulse when high(:1) or low(:0) level
    bool        prevState;         // previous state of the DIn pin
    bool        currentState;      // state of the DIn pin (After chattering control)
    bool        isSetPulse;        // is settlement have done
    bool        isRising;          // is DIn level rised
    bool        isStart;           // is this counter running
} PulseCounter;

typedef struct DoController
{                                   // !日本語のコメント、TODOは最終的に削除・変更してください!
    int         pinId;              // DOut pin number
    bool        defaultState;       // 0/1のどっちをデフォルトにするか
    bool        driveState;         // next state of DO pin
    bool        driveCertainEnable; // 出力時間(「○秒間パルス出力する」など)の設定を有効にするか
    bool        delayEnable;        // ディレイ(「トリガから○秒後に出力」など)の設定を有効にするか
    bool        flagDriveTime;      // 出力時間の設定が有効のときに、現在出力中かどうか
    bool        flagDelay;          // ディレイの設定が有効のときに、ディレイ中かどうか
    int         driveTime;          // 出力時間
    int         driveElapsedTime;   // 出力時間の経過時間
    int         delayTime;          // ディレイ時間
    int         delayElapsedTime;   // ディレイの経過時間
    bool        triggerActived;     // トリガが有効になったときにtrue
    int         pulseEffectiveTime; // パルスを1(High)にする期間(値×分解能=実時間)
    int         pulsePeriod;        // パルス全体の期間(値×分解能=周期)
    PulseClock  pulseClock;         // PWMコントローラのクロック(32kHz, 2MHzのみ使用可能)
    FunctionStatus functionStatus;  // トリガーが一度有効になったあと、稼働中かどうか。いらないかも
    FunctionType functionType;      // 単純出力, パルス出力, 連動出力(DIポートの)
    RelationType relationType;      // DIと連動して出力する際の出力方法
    int         relationPort;       // どのDIポートと連動するか
    bool        isStart;            // is this counter running
} DoController;                     // TODO: DIの動作をトリガとして設定する項目を作成する

// Initialization
extern void PulseCounter_Initialize(PulseCounter* me, int pin);

// Attribute
extern int  PulseCounter_GetPinId(PulseCounter* me);

// Pulse counter driver operation
extern void PulseCounter_SetConfigCounter(PulseCounter* me,
    bool isCountHigh, int minPulse, int maxPulse);
extern void PulseCounter_Clear(PulseCounter* me, int initValue);
extern int  PulseCounter_GetPulseCount(PulseCounter* me);
extern int  PulseCounter_GetPulseOnTime(PulseCounter* me);
extern bool PulseCounter_GetLevel(PulseCounter* me);
extern bool PulseCounter_GetPinLevel(PulseCounter* me);

// Handle polling based pulse counting task
extern void PulseCounter_Counter(PulseCounter* me);

//DIO
DoController_Controller(DoController* me)

#endif  // _PULSE_COUNTER_H_
