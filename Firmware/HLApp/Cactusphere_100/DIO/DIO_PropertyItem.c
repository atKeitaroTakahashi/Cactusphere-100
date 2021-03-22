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

#include "DIO_ConfigMgr.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "applibs_versions.h"
#include <applibs/log.h>

#include "json.h"
#include "DIO_FetchConfig.h"
#include "DIO_WatchConfig.h"
#include "DIO_PropertyItem.h"
#include "PropertyItems.h"

const char FunctionTypeDIKey[]          = "FunctionType_DI";            // FunctionType_DI
const char IntervalDIKey[]              = "Interval_DI";                // Interval_DI
const char EdgeTriggerDIkey[]           = "EdgeTrigger_DI";             // EdgeTrigger_DI
const char MinPulseWidthDIKey[]         = "MinPulseWidth_DI";           // MinPulseWidth_DI
const char MaxPulseCountDIKey[]         = "MaxPulseCount_DI";           // MaxPulseCount_DI
const char FunctionTypeDOKey[]          = "FunctionType_DO";            // FunctionType_DO
const char IsNotifyDOKey[]              = "IsNotify_DO";                // IsNotify_DO
const char SingleFunctionTypeDOKey[]    = "SingleFunctionType_DO";      // SingleFunctionType_DO
const char RelationPortDOKey[]          = "RelationPort_DO";            // RelationPort_DO
const char RelationFunctionTypeDOKey[]  = "RelationFunctionType_DO";    // RelationFunctionType_DO
const char DIEdgeTriggerDOKey[]         = "DIEdgeTrigger_DO";           // DIEdgeTrigger_DO
const char DIPulseCountTriggerDOKey[]   = "DIPulseCountTrigger_DO";     // DIPulseCountTrigger_DO
const char DriveStatusDOKey[]           = "DriveStatus_DO";             // DriveStatus_DO
const char DriveDelayDOKey[]            = "DriveDelay_DO";              // DriveDelay_DO
const char DriveSecondsDOKey[]          = "DriveSeconds_DO";            // DriveSeconds_DO
const char PulseCycleDOKey[]            = "PulseCycle_DO";              // PulseCycle_DO
const char PWMDutyCycleDOKey[]          = "PWMDutyCycle_DO";            // PWMDutyCycle_DO

#define DIO_PORT_OFFSET 1

static bool DIO_PropertyItem_GetIntValue(const json_value* jsonObj, uint32_t* value, int base, vector item, const char* itemName) {
    bool ret = (json_GetIntValue(jsonObj, value, 10) || (jsonObj->type == json_null)) ? true : false;
    if (jsonObj->type == json_null) {
        PropertyItems_AddItem(item, itemName, TYPE_NULL);
    } else {
        PropertyItems_AddItem(item, itemName, TYPE_NUM, *value);
    }
    return ret;
}

static bool DIO_PropertyItem_GetBoolValue(const json_value* jsonObj, bool* value, vector item, const char* itemName) {
    bool ret = json_GetBoolValue(jsonObj, value) ? true : false;
    PropertyItems_AddItem(item, itemName, TYPE_BOOL, *value);
    return ret;
}

bool DIO_PropertyItem_ParseJson(DIO_PropertyData* data,
    const json_value* json, vector propertyItem, const char* version)
{
    const size_t FunctionTypeDILen          = strlen(FunctionTypeDIKey);            // FunctionType_DI
    const size_t IntervalDILen              = strlen(IntervalDIKey);                // Interval_DI
    const size_t EdgeTriggerDILen           = strlen(EdgeTriggerDIkey);             // EdgeTrigger_DI
    const size_t MinPulseWidthDILen         = strlen(MinPulseWidthDIKey);           // MinPulseWidth_DI
    const size_t MaxPulseCountDILen         = strlen(MaxPulseCountDIKey);           // MaxPulseCount_DI
    const size_t FunctionTypeDOLen          = strlen(FunctionTypeDOKey);            // FunctionType_DO
    const size_t IsNotifyDOLen              = strlen(IsNotifyDOKey);                // IsNotify_DO
    const size_t SingleFunctionTypeDOLen    = strlen(SingleFunctionTypeDOKey);      // SingleFunctionType_DO
    const size_t RelationPortDOLen          = strlen(RelationPortDOKey);            // RelationPort_DO
    const size_t RelationFunctionTypeDOLen  = strlen(RelationFunctionTypeDOKey);    // RelationFunctionType_DO
    const size_t DIEdgeTriggerDOLen         = strlen(DIEdgeTriggerDOKey);           // DIEdgeTrigger_DO
    const size_t DIPulseCountTriggerDOLen   = strlen(DIPulseCountTriggerDOKey);     // DIPulseCountTrigger_DO
    const size_t DriveStatusDOLen           = strlen(DriveStatusDOKey);             // DriveStatus_DO
    const size_t DriveDelayDOLen            = strlen(DriveDelayDOKey);              // DriveDelay_DO
    const size_t DriveSecondsDOLen          = strlen(DriveSecondsDOKey);            // DriveSeconds_DO
    const size_t PulseCycleDOLen            = strlen(PulseCycleDOKey);              // PulseCycle_DO
    const size_t PWMDutyCycleDOLen          = strlen(PWMDutyCycleDOKey);            // PWMDutyCycle_DO

    int pinid;
    for (int i = 0; i < json->u.object.length; i++) {
        char* propertyName = json->u.object.values[i].name;
        json_value* item = json->u.object.values[i].value;

        if (0 == strncmp(propertyName, FunctionTypeDIKey, FunctionTypeDILen)) { // DI Function Type
            if ((pinid = strtol(&propertyName[FunctionTypeDILen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                data->diData[pinid].diFunctionType = value;
            }
        } else if (0 == strncmp(propertyName, IntervalDIKey, IntervalDILen)) { // DI Interval
            if ((pinid = strtol(&propertyName[IntervalDILen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 1;
                }
                data->diData[pinid].intervalSec = value;
            }
        } else if (0 == strncmp(propertyName, EdgeTriggerDIkey, EdgeTriggerDILen)) { // DI EdgeTriggerIsHigh
            if ((pinid = strtol(&propertyName[EdgeTriggerDILen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            bool value;
            if (DIO_PropertyItem_GetBoolValue(item, &value, propertyItem, propertyName)) {
                data->diData[pinid].isEdgeTriggerHigh = value;
            }
        } else if (0 == strncmp(propertyName, MinPulseWidthDIKey, MinPulseWidthDILen)) { // DI minPulseWidth
            if ((pinid = strtol(&propertyName[MinPulseWidthDILen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 200;
                }
                data->diData[pinid].minPulseWidth = value;
            }
        } else if (0 == strncmp(propertyName, MaxPulseCountDIKey, MaxPulseCountDILen)) { // DI maxPulseCount
            if ((pinid = strtol(&propertyName[MaxPulseCountDILen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 0x7FFFFFFF;
                }
                data->diData[pinid].maxPulseCount = value;
            }
        } else if (0 == strncmp(propertyName, FunctionTypeDOKey, FunctionTypeDOLen)) { // DO Function Type
            if ((pinid = strtol(&propertyName[FunctionTypeDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                data->doData[pinid].doFunctionType = value;
            }
        } else if (0 == strncmp(propertyName, IsNotifyDOKey, IsNotifyDOLen)) { // DO IsNotify
            if ((pinid = strtol(&propertyName[IsNotifyDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            bool value;
            if (DIO_PropertyItem_GetBoolValue(item, &value, propertyItem, propertyName)) {
                data->doData[pinid].isDoNotify = value;
            }
        } else if (0 == strncmp(propertyName, SingleFunctionTypeDOKey, SingleFunctionTypeDOLen)) { // DO Function Type(Single)
            if ((pinid = strtol(&propertyName[SingleFunctionTypeDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                data->doData[pinid].singleFunctionType = value;
            }
        } else if (0 == strncmp(propertyName, RelationPortDOKey, RelationPortDOLen)) { // DO RelationDIPort
            if ((pinid = strtol(&propertyName[RelationPortDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                data->doData[pinid].relationDIPort = value;
            }
        } else if (0 == strncmp(propertyName, RelationFunctionTypeDOKey, RelationFunctionTypeDOLen)) { // DO Function Type(Relation)
            if ((pinid = strtol(&propertyName[RelationFunctionTypeDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                data->doData[pinid].relationFunctionType = value;
            }
        } else if (0 == strncmp(propertyName, DIEdgeTriggerDOKey, DIEdgeTriggerDOLen)) { // DO DriveStatusIsHigh
            if ((pinid = strtol(&propertyName[DIEdgeTriggerDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            bool value;
            if (DIO_PropertyItem_GetBoolValue(item, &value, propertyItem, propertyName)) {
                data->doData[pinid].isDIEdgeTriggerHigh = value;
            }
        } else if (0 == strncmp(propertyName, DIPulseCountTriggerDOKey, DIPulseCountTriggerDOLen)) { // DO DIPulseCountTrigger
            if ((pinid = strtol(&propertyName[DIPulseCountTriggerDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 0x7FFFFFFF;
                }
                data->doData[pinid].triggerPulseCount = value;
            }
        } else if (0 == strncmp(propertyName, DriveStatusDOKey, DriveStatusDOLen)) { // DO DriveStatus
            if ((pinid = strtol(&propertyName[DriveStatusDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            bool value;
            if (DIO_PropertyItem_GetBoolValue(item, &value, propertyItem, propertyName)) {
                data->doData[pinid].isDODriveStateHigh = value;
            }
        } else if (0 == strncmp(propertyName, DriveDelayDOKey, DriveDelayDOLen)) { // DO DriveDelay
            if ((pinid = strtol(&propertyName[DriveDelayDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 0;
                }
                data->doData[pinid].doDriveDelay = value;
            }
        } else if (0 == strncmp(propertyName, DriveSecondsDOKey, DriveSecondsDOLen)) { // DO DriveSeconds
            if ((pinid = strtol(&propertyName[DriveSecondsDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 0;
                }
                data->doData[pinid].doDriveSeconds = value;
            }
        } else if (0 == strncmp(propertyName, PulseCycleDOKey, PulseCycleDOLen)) { // DO PulseCycle
            if ((pinid = strtol(&propertyName[PulseCycleDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 1;
                }
                data->doData[pinid].doPulseCycle = value;
            }
        } else if (0 == strncmp(propertyName, PWMDutyCycleDOKey, PWMDutyCycleDOLen)) { // DO PWMDutyCycle
            if ((pinid = strtol(&propertyName[PWMDutyCycleDOLen], NULL, 10) - DIO_PORT_OFFSET) < 0) {
                continue;
            }
            uint32_t value;
            if (DIO_PropertyItem_GetIntValue(item, &value, 10, propertyItem, propertyName)) {
                if(item->type == json_null){
                    value = 50;
                }
                data->doData[pinid].doPwmDutyCycle = value;
            }
        }
    }
    return true;
}

bool DIO_PropertyItem_CheckDOSetting(DIO_PropertyData* data){
    bool ret = true;
    uint32_t pinid;

    for (int i = 0; i < NUM_DIO; i++) {
        switch (data->doData[i].doFunctionType)
        {
        case DOFUNC_TYPE_NOTSELECTED:
            break;
        case DOFUNC_TYPE_SINGLE:
            switch (data->doData[i].singleFunctionType)
            {
            case DOFUNC_SGL_TYPE_NOTSELECTED:
                // Error
                ret = false;
                break;
            case DOFUNC_SGL_TYPE_DRIVE:
                // Option : doDriveDelay, doDriveSeconds
                break;
            case DOFUNC_SGL_TYPE_PULSE:
                // Option : doPulseCycle
                if (!(data->doData[i].doPulseCycle >= 1 && data->doData[i].doPulseCycle <= 100000)) {
                    ret = false;
                }
                break;
            case DOFUNC_SGL_TYPE_PWM:
                // Option : doPulseCycle, doPwmDutyCycle
                if ((!(data->doData[i].doPulseCycle >= 1 && data->doData[i].doPulseCycle <= 100000)) ||
                    (!(data->doData[i].doPwmDutyCycle >= 0 && data->doData[i].doPwmDutyCycle <= 100))) {
                    ret = false;
                }
                break;
            default:
                break;
            }
            break;
        case DOFUNC_TYPE_RELATION:
            if (data->doData[i].relationDIPort == DI_REL_PORT_NOTSELECTED) {
                // Error
                ret = false;
                break;
            }
            pinid = data->doData[i].relationDIPort - DIO_PORT_OFFSET;

            switch (data->doData[i].relationFunctionType)
            {
            case DOFUNC_REL_TYPE_NOTSELECTED:
                // Error
                ret = false;
                break;
            case DOFUNC_REL_TYPE_DRIVE:
                // Relation : PulseCount
                if (data->diData[pinid].diFunctionType != DIFUNC_TYPE_PULSECOUNTER) {
                    ret = false;
                }
                if (!(data->doData[i].triggerPulseCount >= 1 && data->doData[i].triggerPulseCount <= 0x7FFFFFFF)) {
                    ret = false;
                }
                if (data->diData[pinid].maxPulseCount < data->doData[i].triggerPulseCount) {
                    ret = false;
                }
                break;
            case DOFUNC_REL_TYPE_INTERLOCK:
                // Relation : Edge
                if (data->diData[pinid].diFunctionType != DIFUNC_TYPE_EDGE) {
                    ret = false;
                }
                break;
            case DOFUNC_REL_TYPE_INVERT:
                // Relation : Edge
                if (data->diData[pinid].diFunctionType != DIFUNC_TYPE_EDGE) {
                    ret = false;
                }
                break;
            case DOFUNC_REL_TYPE_SHAP:
                // Relation : Edge
                if (data->diData[pinid].diFunctionType != DIFUNC_TYPE_EDGE) {
                    ret = false;
                }
                break;
            case DOFUNC_REL_TYPE_PULSE:
                // Relation : PulseCount or Edge
                if (data->diData[pinid].diFunctionType == DIFUNC_TYPE_PULSECOUNTER) {
                    if (!(data->doData[i].triggerPulseCount >= 1 && data->doData[i].triggerPulseCount <= 0x7FFFFFFF)) {
                        ret = false;
                    }
                    if (data->diData[pinid].maxPulseCount < data->doData[i].triggerPulseCount) {
                        ret = false;
                    }
                } else if(data->diData[pinid].diFunctionType != DIFUNC_TYPE_EDGE){
                    ret = false;
                }
                // Option : doPulseCycle
                if (!(data->doData[i].doPulseCycle >= 1 && data->doData[i].doPulseCycle <= 100000)) {
                    ret = false;
                }
                break;
            case DOFUNC_REL_TYPE_PWM:
                // Relation : PulseCount or Edge
                if (data->diData[pinid].diFunctionType == DIFUNC_TYPE_PULSECOUNTER) {
                    if (!(data->doData[i].triggerPulseCount >= 1 && data->doData[i].triggerPulseCount <= 0x7FFFFFFF)) {
                        ret = false;
                    }
                    if (data->diData[pinid].maxPulseCount < data->doData[i].triggerPulseCount) {
                        ret = false;
                    }
                } else if(data->diData[pinid].diFunctionType != DIFUNC_TYPE_EDGE){
                    ret = false;
                }
                // Option : doPulseCycle, doPwmDutyCycle
                if ((!(data->doData[i].doPulseCycle >= 1 && data->doData[i].doPulseCycle <= 100000)) ||
                    (!(data->doData[i].doPwmDutyCycle >= 0 && data->doData[i].doPwmDutyCycle <= 100))) {
                    ret = false;
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    return ret;
}
