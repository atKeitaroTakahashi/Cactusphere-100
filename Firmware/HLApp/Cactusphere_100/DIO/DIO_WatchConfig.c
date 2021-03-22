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

#include "DIO_WatchConfig.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "json.h"
#include "DIO_WatchItem.h"
#include "TelemetryItems.h"
#include "PropertyItems.h"

struct DIO_WatchConfig {
    vector	mWatchItems;	// vector of DIO contact input configuration
    char	version[32];	// version string (not using)
};

#define DIO_WATCH_PORT_OFFSET 1

// Initialization and cleanup
DIO_WatchConfig*
DIO_WatchConfig_New(void)
{
    DIO_WatchConfig*	newObj =
        (DIO_WatchConfig*)malloc(sizeof(DIO_WatchConfig));

    if (NULL != newObj) {
        newObj->mWatchItems = vector_init(sizeof(DIO_WatchItem));
        if (NULL == newObj->mWatchItems) {
            free(newObj);
            return NULL;
        }
        memset(newObj->version, 0, sizeof(newObj->version));
    }

    return newObj;
}

void
DIO_WatchConfig_Destroy(DIO_WatchConfig* me)
{
    vector_destroy(me->mWatchItems);
    free(me);
}

// Load DIO contact input watcher configuration from JSON
bool
DIO_WatchConfig_LoadFromJSON(DIO_WatchConfig* me, DIO_PropertyData* data,
    const json_value* json, vector propertyItem, const char* version)
{
    bool ret = true;

    if (! json) {
        return false;
    }

    if (0 != vector_size(me->mWatchItems)) {
        DIO_WatchItem*	curs = (DIO_WatchItem*)vector_get_data(me->mWatchItems);

        for (int i = 0, n = vector_size(me->mWatchItems); i < n; ++i) {
            TelemetryItems_RemoveDictionaryElem(curs->telemetryName);
        }
        vector_clear(me->mWatchItems);
        memset(me->version, 0, sizeof(me->version));
    }

    for (uint32_t i = 0; i < NUM_DIO; i++){
        DIO_WatchItem config =
        // telemetryName, pinID, notifyChangeForHigh, isCountClear
        { "", 0, false, false};

        switch(data->diData[i].diFunctionType) {
            case DIFUNC_TYPE_EDGE:
                // telemetryName
                sprintf(config.telemetryName, "DI%d_EdgeEvent", i + DIO_WATCH_PORT_OFFSET);
                // pinID
                config.pinID = i;
                // notifyChangeForHigh
                config.notifyChangeForHigh = data->diData[i].isEdgeTriggerHigh;
                break;
            case DIFUNC_TYPE_NOTSELECTED:
            case DIFUNC_TYPE_PULSECOUNTER:
            default:
                continue;
        }

        vector_add_last(me->mWatchItems, &config);
    }

    if (! vector_is_empty(me->mWatchItems)) {
        // store entity's pointers
        DIO_WatchItem*	curs = (DIO_WatchItem*)vector_get_data(me->mWatchItems);

        for (int i = 0, n = vector_size(me->mWatchItems); i < n; ++i) {
            TelemetryItems_AddDictionaryElem(curs->telemetryName, false);
            ++curs;
        }
    }

    return ret;
}

// Get configuration of DIO contact input watchers
vector
DIO_WatchConfig_GetFetchItems(DIO_WatchConfig* me)
{
    return me->mWatchItems;
}

// Get enable port number of DIO contact input
int
DIO_WatchConfig_GetWatchEnablePorts(DIO_WatchConfig* me, bool* status)
{
    DIO_WatchItem* tmp;
    int enablePort = vector_size(me->mWatchItems);

    for (int i = 0; i < enablePort; i++) {
        tmp = (DIO_WatchItem*)vector_get_data(me->mWatchItems);
        status[tmp->pinID] = true;
    }

    return enablePort;
}
