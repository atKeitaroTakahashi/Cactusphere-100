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

#ifndef _DIO_PROPERTYITEM_H_
#define _DIO_PROPERTYITEM_H_

#ifndef _STDBOOL_H
#include <stdbool.h>
#endif
#ifndef _STDIDONT_H
#include <stdint.h>
#endif

#ifndef NUM_DIO
#define NUM_DIO 2
#endif

typedef struct _json_value	json_value;

typedef enum {
    DIFUNC_TYPE_NOTSELECTED = 0,
    DIFUNC_TYPE_PULSECOUNTER,
    DIFUNC_TYPE_EDGE,
    DIFUNC_TYPE_POLLING,
} DIFuncType;

typedef enum {
    DOFUNC_TYPE_NOTSELECTED = 0,
    DOFUNC_TYPE_SINGLE,
    DOFUNC_TYPE_RELATION,
} DOFuncType;

typedef enum {
    DOFUNC_SGL_TYPE_NOTSELECTED = 0,
    DOFUNC_SGL_TYPE_DRIVE,
    DOFUNC_SGL_TYPE_PULSE,
    DOFUNC_SGL_TYPE_PWM,
} DOFuncSingleType;

typedef enum {
    DOFUNC_REL_TYPE_NOTSELECTED = 0,
    DOFUNC_REL_TYPE_DRIVE,
    DOFUNC_REL_TYPE_INTERLOCK,
    DOFUNC_REL_TYPE_INVERT,
    DOFUNC_REL_TYPE_SHAP,
    DOFUNC_REL_TYPE_PULSE,
    DOFUNC_REL_TYPE_PWM,
} DOFuncRelationType;

typedef enum {
    DI_REL_PORT_NOTSELECTED = 0,
    DI_REL_PORT_DI1 = 1,
    DI_REL_PORT_DI2 = 2,
} DIRelationPort;

typedef struct DIO_DIPropertyData {
    DIFuncType diFunctionType;
    uint32_t intervalSec;
    bool     isEdgeTriggerHigh;
    uint32_t minPulseWidth;
    uint32_t maxPulseCount;
} DIO_DIPropertyData;

typedef struct DIO_DOPropertyData {
    DOFuncType doFunctionType;
    bool     isDoNotify;
    DOFuncSingleType singleFunctionType;
    DIRelationPort relationDIPort;
    DOFuncRelationType relationFunctionType;
    bool     isDIEdgeTriggerHigh;
    uint32_t triggerPulseCount;
    bool     isDODriveStateHigh;
    uint32_t doDriveDelay;
    uint32_t doDriveSeconds;
    uint32_t doPulseCycle;
    uint32_t doPwmDutyCycle;
} DIO_DOPropertyData;

typedef struct DIO_PropertyData {
    struct DIO_DIPropertyData diData[NUM_DIO];
    struct DIO_DOPropertyData doData[NUM_DIO];
} DIO_PropertyData;

extern bool DIO_PropertyItem_ParseJson(DIO_PropertyData* data,
    const json_value* json, vector propertyItem, const char* version);

extern bool DIO_PropertyItem_CheckDOSetting(DIO_PropertyData* data);

#endif  // _DIO_PROPERTYITEM_H_
