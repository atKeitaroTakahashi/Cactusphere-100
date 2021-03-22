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

#include "DIO_FetchConfig.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "json.h"
#include "DIO_FetchItem.h"
#include "TelemetryItems.h"
#include "PropertyItems.h"

struct DIO_FetchConfig {
    vector	mFetchItems;    // vector of DIO pulse conter configuration
    vector	mFetchItemPtrs;	// vector of pointer which points mFetchItem's elem
    char	version[32];	// version string (not using)
};

#define DIO_FETCH_PORT_OFFSET 1

// Initialization and cleanup
DIO_FetchConfig*
DIO_FetchConfig_New(void)
{
    DIO_FetchConfig*	newObj =
        (DIO_FetchConfig*)malloc(sizeof(DIO_FetchConfig));

    if (NULL != newObj) {
        newObj->mFetchItems = vector_init(sizeof(DIO_FetchItem));
        if (NULL == newObj->mFetchItems) {
            free(newObj);
            return NULL;
        }
        newObj->mFetchItemPtrs = vector_init(sizeof(DIO_FetchItem*));
        if (NULL == newObj->mFetchItemPtrs) {
            vector_destroy(newObj->mFetchItems);
            free(newObj);
            return NULL;
        }
        memset(newObj->version, 0, sizeof(newObj->version));
    }

    return newObj;
}

void
DIO_FetchConfig_Destroy(DIO_FetchConfig* me)
{
    vector_destroy(me->mFetchItemPtrs);
    vector_destroy(me->mFetchItems);
    free(me);
}


// Load DIO pulse conter configuration from JSON
bool
DIO_FetchConfig_LoadFromJSON(DIO_FetchConfig* me, DIO_PropertyData* data,
    const json_value* json, vector propertyItem, const char* version)
{
    bool ret = true;

    if (! json) {
        return false;
    }

    DIO_FetchItem currentValue[NUM_DIO] = {
        // telemetryName, intervalSec, pinID, isPulseCounter, isPulseHigh, isCountClear, minPulseWidth, maxPulseCount
        {"", 1, 0, false, false, false, 200, 0x7FFFFFFF},
        {"", 1, 1, false, false, false, 200, 0x7FFFFFFF}
    };

    if (! vector_is_empty(me->mFetchItems)) {
        DIO_FetchItem* tmp = (DIO_FetchItem*)vector_get_data(me->mFetchItems);
        for (int i = 0; i < vector_size(me->mFetchItems); i++) {
            memcpy(&currentValue[tmp->pinID], tmp, sizeof(DIO_FetchItem));
            tmp ++;
        }
    }

    if (0 != vector_size(me->mFetchItems)) {
        DIO_FetchItem*	curs = (DIO_FetchItem*)vector_get_data(me->mFetchItems);

        for (int i = 0, n = vector_size(me->mFetchItems); i < n; ++i) {
            TelemetryItems_RemoveDictionaryElem(curs->telemetryName);
        }
        vector_clear(me->mFetchItemPtrs);
        vector_clear(me->mFetchItems);
    }

    for (uint32_t i = 0; i < NUM_DIO; i++) {
        DIO_FetchItem config =
        // telemetryName, intervalSec, pinID, isPulseCounter, isCountClear, isPulseHigh, minPulseWidth, maxPulseCount
        {"", 1, 0, false, true, false, 200, 0x7FFFFFFF};

        switch(data->diData[i].diFunctionType) {
            case DIFUNC_TYPE_PULSECOUNTER:
                // telemetryName
                sprintf(config.telemetryName, "DI%d_count", i + DIO_FETCH_PORT_OFFSET);
                // intervalSec
                if(data->diData[i].intervalSec >= 1 && data->diData[i].intervalSec <= 86400) {
                    config.intervalSec = data->diData[i].intervalSec;
                } else {
                    ret = false;
                    continue;
                }
                // pinID
                config.pinID = i;

                // isPulseCounter
                config.isPulseCounter = true;

                // isCountClear
                if (currentValue[i].isPulseCounter) {
                    if((currentValue[i].intervalSec == data->diData[i].intervalSec) &&
                       (currentValue[i].isPulseHigh == data->diData[i].isEdgeTriggerHigh) &&
                       (currentValue[i].minPulseWidth == data->diData[i].minPulseWidth) &&
                       (currentValue[i].maxPulseCount == data->diData[i].maxPulseCount)) {
                        config.isCountClear = false;
                    }
                }

                // isPulseHigh
                config.isPulseHigh = data->diData[i].isEdgeTriggerHigh;
                // minPulseWidth
                if (data->diData[i].minPulseWidth >= 1 && data->diData[i].minPulseWidth <= 1000) {
                    config.minPulseWidth = data->diData[i].minPulseWidth;
                } else {
                    ret = false;
                    continue;
                }
                // maxPulseCount
                if (data->diData[i].maxPulseCount >= 1 && data->diData[i].maxPulseCount <= 0x7FFFFFFF) {
                    config.maxPulseCount = data->diData[i].maxPulseCount;
                } else {
                    ret = false;
                    continue;
                }
                break;
            case DIFUNC_TYPE_POLLING:
                // telemetryName
                sprintf(config.telemetryName, "DI%d_PollingStatus", i + DIO_FETCH_PORT_OFFSET);
                // intervalSec
                if(data->diData[i].intervalSec >= 1 && data->diData[i].intervalSec <= 86400) {
                    config.intervalSec = data->diData[i].intervalSec;
                } else {
                    ret = false;
                    continue;
                }
                break;
            case DIFUNC_TYPE_NOTSELECTED:
            case DIFUNC_TYPE_EDGE:
            default:
                continue;
        }
        vector_add_last(me->mFetchItems, &config);
    }

    if (! vector_is_empty(me->mFetchItems)) {
        // store entity's pointers
        DIO_FetchItem* curs = (DIO_FetchItem*)vector_get_data(me->mFetchItems);

        for (int i = 0, n = vector_size(me->mFetchItems); i < n; ++i) {
            vector_add_last(me->mFetchItemPtrs, &curs);
            TelemetryItems_AddDictionaryElem(curs->telemetryName, false);
            ++curs;
        }
    }

    return ret;
}

// Get configuration of DIO pulse conters
vector
DIO_FetchConfig_GetFetchItems(DIO_FetchConfig* me)
{
    return me->mFetchItems;
}

vector
DIO_FetchConfig_GetFetchItemPtrs(DIO_FetchConfig* me)
{
    return me->mFetchItemPtrs;
}

// Get enable port number of DIO pulse counter
int
DIO_FetchConfig_GetFetchEnablePorts(DIO_FetchConfig* me,
    bool* counterStatus, bool* pollingStatus)
{
    DIO_FetchItem* tmp = (DIO_FetchItem*)vector_get_data(me->mFetchItems);
    int enablePort = vector_size(me->mFetchItems);

    for (int i = 0; i < enablePort; i++){
        if (tmp->isPulseCounter) {
            counterStatus[tmp->pinID] = true;
        } else {
            pollingStatus[tmp->pinID] = true;
        }
        tmp++;
    }

    return enablePort;
}
