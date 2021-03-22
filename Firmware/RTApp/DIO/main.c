/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Copyright (c) 2020 Atmark Techno, Inc.
 * 
 * MIT License
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

#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "mt3620-baremetal.h"
#include "mt3620-intercore.h"
#include "mt3620-gpio.h"
#include "mt3620-timer.h"

#include "InterCoreComm.h"
#include "TimerUtil.h"
#include "PulseCounter.h"


#define NUM_DI	2	// num of DIO ports
#define NUM_DO	2	// num of DIO ports
#define NUM_DIO	(NUM_DI + NUM_DO)

#define OK	1
#define NG	-1

// DIO gpio pin number/ID
const int DIPIN_0 = 12;
const int DIPIN_1 = 15;
const int DOPIN_0 = 0;
const int DOPIN_1 = 8;
static const int periodMs = 1;  // 1[ms] (for polling DIn pin's input level)
static PulseCounter sPulseCounter[NUM_DI];
static DoController sDoController[NUM_DO];


extern uint32_t StackTop; // &StackTop == end of TCM

static _Noreturn void DefaultExceptionHandler(void);
static _Noreturn void RTCoreMain(void);

// 1ms timer's interrupt handler
static void
Handle1msIrq(void)
{
    for (int i = 0; i < NUM_DI; i++) {
        if (sPulseCounter[i].isStart) {
            PulseCounter_Counter(&sPulseCounter[i]);
        }
    }
    for (int i = 0; i < NUM_DO; i++) {
        if (sDoController[i].isStart) {
            DoController_Controller(&sDoController[i]);
        }
    }
    Gpt_LaunchTimerMs(TimerGpt1, periodMs, Handle1msIrq);
}

static PulseCounter*
GetTargetPt(int pinId)
{
    // return PulseCounter which specified with the DIn pin ID
    for (int i = 0; i < NUM_DI; i++) {
        if (pinId == PulseCounter_GetPinId(&sPulseCounter[i])) {
            return &sPulseCounter[i];
        }
    }
    return NULL;
}

static DoController*
GetTargetPt_DO(int pinId)
{
    // return DoController which specified with the DOut pin ID
    for (int i = 0; i < NUM_DO; i++) {
        if (pinId == DoController_GetPinId(&sDoController[i])) {
            return &sDoController[i];
        }
    }
    return NULL;
}

// ARM DDI0403E.d SB1.5.2-3
// From SB1.5.3, "The Vector table must be naturally aligned to a power of two whose alignment
// value is greater than or equal to (Number of Exceptions supported x 4), with a minimum alignment
// of 128 bytes.". The array is aligned in linker.ld, using the dedicated section ".vector_table".

// The exception vector table contains a stack pointer, 15 exception handlers, and an entry for
// each interrupt.
#define INTERRUPT_COUNT 100 // from datasheet
#define EXCEPTION_COUNT (16 + INTERRUPT_COUNT)
#define INT_TO_EXC(i_) (16 + (i_))
const uintptr_t ExceptionVectorTable[EXCEPTION_COUNT] __attribute__((section(".vector_table")))
__attribute__((used)) = {
    [0] = (uintptr_t)&StackTop,                // Main Stack Pointer (MSP)
    [1] = (uintptr_t)RTCoreMain,               // Reset
    [2] = (uintptr_t)DefaultExceptionHandler,  // NMI
    [3] = (uintptr_t)DefaultExceptionHandler,  // HardFault
    [4] = (uintptr_t)DefaultExceptionHandler,  // MPU Fault
    [5] = (uintptr_t)DefaultExceptionHandler,  // Bus Fault
    [6] = (uintptr_t)DefaultExceptionHandler,  // Usage Fault
    [11] = (uintptr_t)DefaultExceptionHandler, // SVCall
    [12] = (uintptr_t)DefaultExceptionHandler, // Debug monitor
    [14] = (uintptr_t)DefaultExceptionHandler, // PendSV
    [15] = (uintptr_t)DefaultExceptionHandler, // SysTick

    [INT_TO_EXC(0)] = (uintptr_t)DefaultExceptionHandler,
    [INT_TO_EXC(1)] = (uintptr_t)Gpt_HandleIrq1,
    [INT_TO_EXC(2)... INT_TO_EXC(INTERRUPT_COUNT - 1)] = (uintptr_t)DefaultExceptionHandler };

static _Noreturn void
DefaultExceptionHandler(void)
{
    for (;;) {
        // do nothing
    }
}

static _Noreturn void
RTCoreMain(void)
{
    // SCB->VTOR = ExceptionVectorTable
    WriteReg32(SCB_BASE, 0x08, (uint32_t)ExceptionVectorTable);

    // initialization
    if (! TimerUtil_Initialize()) {
        goto err;
    }
    if (! InterCoreComm_Initialize()) {
        goto err;
    }

    // for debugger connection (wait 3[sec])
    {
        uint32_t	prevTickCount = TimerUtil_GetTickCount();
        uint32_t	tickCount     = prevTickCount;

        while (3000 > tickCount - prevTickCount) {
            TimerUtil_SleepUntilIntr();
            tickCount = TimerUtil_GetTickCount();
        }
    }

    // GPIO setting
    static const GpioBlock grp0 = {
        .baseAddr = 0x38010000,.type = GpioBlock_PWM,.firstPin = 0,.pinCount = 4
    };
    Mt3620_Gpio_AddBlock(&grp0);
    Mt3620_Gpio_ConfigurePinForInput(DIPIN_0);
    Mt3620_Gpio_ConfigurePinForInput(DIPIN_1);
    // Mt3620_Gpio_ConfigurePinForOutput(DOPIN_0);
    // Mt3620_Gpio_ConfigurePinForOutput(DOPIN_1);

    // initialize pulse counters and start the polling timer
    PulseCounter_Initialize(&sPulseCounter[0], DIPIN_0);
    PulseCounter_Initialize(&sPulseCounter[1], DIPIN_1);
    DoController_Initialize(&sDoController[0], DOPIN_0);
    DoController_Initialize(&sDoController[1], DOPIN_1);
    Gpt_LaunchTimerMs(TimerGpt1, periodMs, Handle1msIrq);

    // main loop
    for (;;) {
        // wait and receive a request message from HLApp and process it
        const DIO_DriverMsg* msg = InterCoreComm_WaitAndRecvRequest();

        if (msg != NULL) {
            PulseCounter*   targetP_DI = NULL;
            DoController*   targetP_DO = NULL;
            DIO_ReturnMsg    retMsg;
            int val;

            switch (msg->header.requestCode) {
            case DI_SET_CONFIG_AND_START:
                targetP_DI = GetTargetPt(msg->body.setConfig.pinId);
                if (targetP_DI == NULL) {
                    InterCoreComm_SendIntValue(NG);
                    continue;
                }
                PulseCounter_SetConfigCounter(targetP_DI,
                    msg->body.setConfig.isPulseHigh,
                    msg->body.setConfig.minPulseWidth,
                    msg->body.setConfig.maxPulseCount
                );
                if (InterCoreComm_SendIntValue(OK)) {
                    // int i = 0;
                }
                break;
            case DO_SET_CONFIG:
                targetP_DO = GetTargetPt_DO(msg->body.setDOConfig.pinId);
                if (targetP_DO == NULL) {
                    InterCoreComm_SendIntValue(NG);
                    continue;
                }
                break;
            case DI_PULSE_COUNT_RESET:
                targetP_DI = GetTargetPt(msg->body.resetPulseCount.pinId);
                if (targetP_DI == NULL) {
                    InterCoreComm_SendIntValue(NG);
                    continue;
                }
                PulseCounter_Clear(targetP_DI, msg->body.resetPulseCount.initVal);
                val = 1;
                if (InterCoreComm_SendIntValue(val)) {
                    // int i = 0;
                }
                break;
            case DI_READ_PULSE_COUNT:
                targetP_DI = GetTargetPt(msg->body.pinId.pinId);
                if (targetP_DI == NULL) {
                    InterCoreComm_SendIntValue(NG);
                    continue;
                }
                val = PulseCounter_GetPulseCount(targetP_DI);

                if (InterCoreComm_SendIntValue(val)) {
                    // int i = 0;
                }
                break;
            case DI_READ_DUTY_SUM_TIME:
                targetP_DI = GetTargetPt(msg->body.pinId.pinId);
                if (targetP_DI == NULL) {
                    InterCoreComm_SendIntValue(NG);
                    continue;
                }
                val = PulseCounter_GetPulseOnTime(targetP_DI);
                if (InterCoreComm_SendIntValue(val)) {
                    // int i = 0;
                }
                break;
            case DI_READ_PULSE_LEVEL:
                for (int i = 0; i < NUM_DI; i++) {
                    retMsg.message.levels[i] = PulseCounter_GetLevel(&sPulseCounter[i]);
                }
                retMsg.returnCode = OK;
                retMsg.messageLen = sizeof(retMsg.message.levels);
                if (InterCoreComm_SendReadData((uint8_t*)&retMsg, sizeof(DIO_ReturnMsg))) {
                    // int i = 0;
                }
                break;
            case DI_READ_PIN_LEVEL:
                targetP_DI = GetTargetPt(msg->body.pinId.pinId);
                if (targetP_DI == NULL) {
                    InterCoreComm_SendIntValue(NG);
                    continue;
                }
                val = (int)PulseCounter_GetPinLevel(targetP_DI);

                if (InterCoreComm_SendIntValue(val)) {
                    // int i = 0;
                }
                break;
            case DIO_READ_VERSION:
                memset(retMsg.message.version, 0x00, sizeof(retMsg.message.version));
                strncpy(retMsg.message.version, RTAPP_VERSION, strlen(RTAPP_VERSION) + 1);
                retMsg.returnCode = OK;
                retMsg.messageLen = strlen(RTAPP_VERSION);
                if (InterCoreComm_SendReadData((uint8_t*)&retMsg, sizeof(DIO_ReturnMsg))) {
                    // int i = 0;
                }
                break;
            default:
                InterCoreComm_SendIntValue(NG);
                break;
            }
        }
    }

err:
    // error
    DefaultExceptionHandler();
}
