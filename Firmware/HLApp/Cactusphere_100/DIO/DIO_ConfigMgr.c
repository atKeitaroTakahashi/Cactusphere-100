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

typedef struct DIO_ConfigMgr {
    DIO_FetchConfig* fetchConfig;
    DIO_WatchConfig* watchConfig;
    DIO_PropertyData  settingData;
} DIO_ConfigMgr;

#define DIO_PORT_OFFSET 1

static DIO_ConfigMgr sDIO_ConfigMgr;  // singleton

// Initializaition and cleanup
void
DIO_ConfigMgr_Initialize(void)
{
    sDIO_ConfigMgr.fetchConfig = DIO_FetchConfig_New();
    sDIO_ConfigMgr.watchConfig = DIO_WatchConfig_New();

    for (int i = 0; i < NUM_DIO; i ++) {
        // DI
        sDIO_ConfigMgr.settingData.diData[i].diFunctionType = DIFUNC_TYPE_NOTSELECTED;
        sDIO_ConfigMgr.settingData.diData[i].intervalSec = 1;
        sDIO_ConfigMgr.settingData.diData[i].isEdgeTriggerHigh = false;
        sDIO_ConfigMgr.settingData.diData[i].minPulseWidth = 200;
        sDIO_ConfigMgr.settingData.diData[i].maxPulseCount = 0x7FFFFFFF;

        // DO
        sDIO_ConfigMgr.settingData.doData[i].doFunctionType = DOFUNC_TYPE_NOTSELECTED;
        sDIO_ConfigMgr.settingData.doData[i].isDoNotify = false;
        sDIO_ConfigMgr.settingData.doData[i].singleFunctionType = DOFUNC_SGL_TYPE_NOTSELECTED;
        sDIO_ConfigMgr.settingData.doData[i].relationDIPort = DI_REL_PORT_NOTSELECTED;
        sDIO_ConfigMgr.settingData.doData[i].relationFunctionType = DOFUNC_REL_TYPE_NOTSELECTED;
        sDIO_ConfigMgr.settingData.doData[i].isDIEdgeTriggerHigh = false;
        sDIO_ConfigMgr.settingData.doData[i].triggerPulseCount = 0;
        sDIO_ConfigMgr.settingData.doData[i].isDODriveStateHigh = false;
        sDIO_ConfigMgr.settingData.doData[i].doDriveDelay = 0;
        sDIO_ConfigMgr.settingData.doData[i].doDriveSeconds = 0;
        sDIO_ConfigMgr.settingData.doData[i].doPulseCycle = 1;
        sDIO_ConfigMgr.settingData.doData[i].doPwmDutyCycle = 50;
    }
}

void
DIO_ConfigMgr_Cleanup(void)
{
    DIO_FetchConfig_Destroy(sDIO_ConfigMgr.fetchConfig);
    DIO_WatchConfig_Destroy(sDIO_ConfigMgr.watchConfig);
}

// Apply new configuration
SphereWarning
DIO_ConfigMgr_LoadAndApplyIfChanged(const unsigned char* payload,
    unsigned int payloadSize, vector item)
{
    json_value* jsonObj = json_parse(payload, payloadSize);
    json_value* desiredObj = json_GetKeyJson("desired", jsonObj);
    SphereWarning ret = NO_ERROR;

    if (desiredObj) {
        jsonObj = desiredObj;
    }

    DIO_PropertyItem_ParseJson(&sDIO_ConfigMgr.settingData, jsonObj, item, "1.0");

    if (! DIO_FetchConfig_LoadFromJSON(
        sDIO_ConfigMgr.fetchConfig, &sDIO_ConfigMgr.settingData, jsonObj, item, "1.0")) {
        Log_Debug("failed to DIO_FetchConfig_LoadFromJSON().\n");
        ret = ILLEGAL_PROPERTY;
    }

    if (! DIO_WatchConfig_LoadFromJSON(
        sDIO_ConfigMgr.watchConfig, &sDIO_ConfigMgr.settingData, jsonObj, item, "1.0")) {
        Log_Debug("failed to DIO_WatchConfig_LoadFromJSON().\n");
        ret = ILLEGAL_PROPERTY;
    }

    if (! DIO_PropertyItem_CheckDOSetting(&sDIO_ConfigMgr.settingData)) {
        Log_Debug("failed to DIO_PropertyItem_CheckDOSetting().\n");
        ret = ILLEGAL_PROPERTY;
    }

    if ( (jsonObj->u.object.length > 1) && (vector_size(item) < 1)) {
        ret = UNSUPPORTED_PROPERTY;
    }

    return ret;
}

// Get configuratioin
DIO_FetchConfig*
DIO_ConfigMgr_GetFetchConfig()
{
    return sDIO_ConfigMgr.fetchConfig;
}

DIO_WatchConfig*
DIO_ConfigMgr_GetWatchConfig()
{
    return sDIO_ConfigMgr.watchConfig;
}
