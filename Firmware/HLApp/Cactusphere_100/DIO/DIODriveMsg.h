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

#ifndef _DIO_DRIVER_MSG_H_
#define _DIO_DRIVER_MSG_H_

#ifndef _STDIDONT_H
#include <stdint.h>
#endif

// request code
enum {  // !日本語のコメント、TODOは最終的に削除・変更してください!
    DI_SET_CONFIG_AND_START = 1,  // setting up a pulse conter
    DI_PULSE_COUNT_RESET    = 2,  // reset a pulse counter
    DI_READ_PULSE_COUNT     = 3,  // read the counter value
    DI_READ_DUTY_SUM_TIME   = 4,  // read the time integration of pulse
    DI_READ_PULSE_LEVEL     = 5,  // read the input level of all DIO pin
    DI_READ_PIN_LEVEL       = 6,  // read the input level of specific DIO pin
    DO_SET_CONFIG           = 7,  // 動作タイプを設定するやつ
    DO_SET_TRIGGER          = 8,  // トリガ条件を設定するやつ
    DO_TRIGGER_IMMEDIATE    = 9,  // DO動作を即時実行するやつ
    DIO_READ_VERSION        = 255,// read the RTApp version
}; // Firmware/RTApp/DIO/DIODriveMsg.hと揃える

//
// message structure
//
// header
typedef struct DIO_DriverMsgHdr
{
    uint32_t requestCode;
    uint32_t messageLen;
} DIO_DriverMsgHdr;

// body
// setting config
typedef struct DI_MsgSetConfig
{
    uint32_t pinId;
    uint32_t minPulseWidth;
    uint32_t maxPulseCount;
    bool isPulseHigh;
} DI_MsgSetConfig;
// sizeof(DI_MsgSetConfig) == messageLen

// DO_SET_CONFIG_AND_START
typedef struct DO_MsgSetConfig
{
    // !日本語のコメント、TODOは最終的に削除・変更してください!
    // TODO: Firmware/RTApp/DIO/DIODriveMsg.hと揃える
} DO_MsgSetConfig;
// sizeof(DO_MsgSetConfig) == messageLen

// pulse reset
typedef struct DIO_MsgResetPulseCount
{
    uint32_t pinId;
    uint32_t initVal;
} DIO_MsgResetPulseCount;
// sizeof(DIO_MsgResetPulseCount) == messageLen

// pinID
typedef struct DIO_MsgPinId
{
    uint32_t pinId;
} DIO_MsgPinId;
// sizeof(DIO_MsgPinId) == messageLen

// message
typedef struct DIO_DriverMsg
{
    DIO_DriverMsgHdr header;
    union
    {
        DI_MsgSetConfig setConfig;
        DO_MsgSetConfig setDOConfig;
        DIO_MsgResetPulseCount resetPulseCount;
        DIO_MsgPinId pinId;
    } body;
} DIO_DriverMsg;

// return message
typedef struct DIO_ReturnMsg
{
    uint32_t returnCode;
    uint32_t messageLen;
    union
    {
        bool levels[4];
        char version[256];
    } message;
} DIO_ReturnMsg;

#endif // _DIO_DRIVER_MSG_H_
