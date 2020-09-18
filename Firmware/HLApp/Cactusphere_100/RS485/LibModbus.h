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

#ifndef _LIBMODBUS_H_
#define _LIBMODBUS_H_

#include <stdbool.h>

#include "ModbusDev.h"
#include "json.h"

// Initialization and destroy
extern void Libmodbus_ModbusDevInitialize(void);
extern void Libmodbus_ModbusDevDestroy(void);

// Clear
extern void Libmodbus_ModbusDevClear(void);

// Regist
extern bool Libmodbus_LoadFromJSON(const json_value* json);

// Connect
extern ModbusDev* Libmodbus_GetAndConnectLib(int devID);

// Read/Write register
extern bool Libmodbus_ReadRegister(ModbusDev* me, int regAddr, int funcCode, unsigned short* dst, int regCount);
extern bool Libmodbus_WriteRegister(ModbusDev* me, int regAddr, int funcCode, unsigned short* data);

// Get RTApp Version
extern bool Libmodbus_GetRTAppVersion(char* rtAppVersion);

#endif  // _LIBMODBUS_H_
