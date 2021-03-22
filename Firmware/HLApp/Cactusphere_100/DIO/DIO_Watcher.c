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

#include "DIO_Watcher.h"

#include "DIO_WatchItem.h"
#include "LibDIO.h"

// DIO_Watcher data members
struct DIO_Watcher {
    vector	mBody;         // vector of DIO_WatchItemStat
    vector	mLastChanges;  // pointer vector of changed DIO_WatchItemStat
};

// Initialization and cleanup
DIO_Watcher*
DIO_Watcher_New(void)
{
    DIO_Watcher*	newObj = (DIO_Watcher*)malloc(sizeof(DIO_Watcher));

    if (NULL != newObj) {
        newObj->mBody = vector_init(sizeof(DIO_WatchItemStat));
        newObj->mLastChanges = vector_init(sizeof(DIO_WatchItemStat*));
        if (NULL == newObj->mBody || NULL == newObj->mLastChanges) {
            if (NULL != newObj->mBody) {
                vector_destroy(newObj->mBody);
            }
            if (NULL != newObj->mLastChanges) {
                vector_destroy(newObj->mLastChanges);
            }
            free(newObj);
            newObj = NULL;
        }
    }

    return newObj;
}

void
DIO_Watcher_Init(DIO_Watcher* me, vector watchItems)
{
    // clean up old configuration and setting up monitoring with new configuration
    const DIO_WatchItem*	curs;

    if (0 != vector_size(me->mBody)) {
        vector_clear(me->mBody);
        vector_clear(me->mLastChanges);
    }

    curs = (const DIO_WatchItem*)vector_get_data(watchItems);
    for (int i = 0, n = vector_size(watchItems); i < n; ++i) {
        // configure pulse counter for monitoring contact input
        DIO_WatchItemStat	pseudo;

        DIO_Lib_ResetPulseCount(curs->pinID, 0);
        if (! DIO_Lib_ConfigPulseCounter(curs->pinID, curs->notifyChangeForHigh,
                200, 0xFFFFFFFF)) {
            // error !
            continue;  // ignore that target
        }
        pseudo.watchItem      = curs++;
        pseudo.prevPulseCount = pseudo.currPulseCount = 0;
        vector_add_last(me->mBody, &pseudo);
    }
}

void
DIO_Watcher_Destroy(DIO_Watcher* me)
{
    vector_destroy(me->mBody);
    vector_destroy(me->mLastChanges);
    free(me);
}

// Check update
bool
DIO_Watcher_DoWatch(DIO_Watcher* me)
{
    // Find state changed contact inputs and store them to the vector.
    // Return whether it has changed.
    DIO_WatchItemStat*	curs;

    if (0 != vector_size(me->mLastChanges)) {
        for (int i = 0, n = vector_size(me->mLastChanges); i < n; ++i) {
            DIO_WatchItemStat*	changed;

            vector_get_at(&changed, me->mLastChanges, i);
            changed->prevPulseCount = changed->currPulseCount;
        }
        vector_clear(me->mLastChanges);
    }

    curs = (DIO_WatchItemStat*)vector_get_data(me->mBody);
    for (int i = 0, n = vector_size(me->mBody); i < n; ++i) {
        // Check status change of contact input from the pulse counter value
        unsigned long	counterVal;

        if (! DIO_Lib_ReadPulseCount(curs->watchItem->pinID, &counterVal)) {
            // error!!
            continue;  // ignore that contact input
        }

        if (curs->prevPulseCount != counterVal) {
            curs->currPulseCount = counterVal;
            vector_add_last(me->mLastChanges, &curs);
        }
        curs++;
    }

    return (0 != vector_size(me->mLastChanges));
}

const vector
DIO_Watcher_GetLastChanges(DIO_Watcher* me)
{
    return me->mLastChanges;
}
