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

#ifndef _LIB_DIO_H_
#define _LIB_DIO_H_

#include <stdbool.h>

#define NUM_DIO	2

// Initialization and cleanup
extern bool DIO_Lib_Initialize(void);
extern void DIO_Lib_Cleanup(void);

// Configure the pulse counter
extern bool DIO_Lib_ConfigPulseCounter(unsigned long pinId,
    bool isPulseHigh, unsigned long minPulseWidth, unsigned long maxPulseCount);

// Reset the pulse counter
extern bool DIO_Lib_ResetPulseCount(unsigned long pinId, unsigned long initVal);

// Read value of the pulse counter
extern bool DIO_Lib_ReadPulseCount(unsigned long pinId, unsigned long* outVal);

// Read on-time integrated value of the pulse (in seconds)
extern bool	DIO_Lib_ReadDutySumTime(unsigned long pinID, unsigned long* outSecs);

// Get input level of all DIO ports/pins
extern bool DIO_Lib_ReadLevels(int outLevels[NUM_DIO]);

// Get input level of specific pin
extern bool DIO_Lib_ReadPinLevel(unsigned long pinId, unsigned int* outVal);

// Get RTApp Version
extern bool DIO_Lib_ReadRTAppVersion(char* rtAppVersion);

#endif  // _LIB_DIO_H_
