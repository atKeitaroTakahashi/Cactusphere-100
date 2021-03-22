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

#include "DIO_DataFetchScheduler.h"

#include "DIO_FetchItem.h"
#include "DIO_FetchTargets.h"
#include "DIO_Watcher.h"
#include "DIO_WatchItem.h"
#include "LibCloud.h"
#include "LibDIO.h"
#include "StringBuf.h"
#include "TelemetryItems.h"

typedef struct DIO_DataFetchScheduler {
    DataFetchSchedulerBase	Super;

// data member
    DIO_FetchTargets*    mFetchTargets;  // acquisition targets of pulse conter
    DIO_Watcher*         mWatcher;       // contact input watch targets
} DIO_DataFetchScheduler;

//
// DIO_DataFetchScheduler's private procedure/method
//
// Callback procedure of FetchTimers
static void
DIO_FetchTimerCallback(void* arg, const FetchItemBase* fetchTarget)
{
    // This procedure called against the acquisition target which
    // timer expired
    DIO_DataFetchScheduler* scheduler = (DIO_DataFetchScheduler*)arg;

    DIO_FetchTargets_Add(
        scheduler->mFetchTargets, (const DIO_FetchItem*)fetchTarget);
}

// Virtual method
static void
DIO_DataFetchScheduler_DoDestroy(DataFetchSchedulerBase* me)
{
    // cleanup own member
    DIO_DataFetchScheduler* self = (DIO_DataFetchScheduler*)me;

    DIO_FetchTargets_Destroy(self->mFetchTargets);
    DIO_Watcher_Destroy(self->mWatcher);
}

static void
DIO_DataFetchScheduler_ClearFetchTargets(DataFetchSchedulerBase* me)
{
    DIO_DataFetchScheduler* self = (DIO_DataFetchScheduler*)me;

    DIO_FetchTargets_Clear(self->mFetchTargets);
}

static void
DIO_DataFetchScheduler_DoSchedule(DataFetchSchedulerBase* me)
{
    // acquire telemetry value from the pulse conter which timer expired and
    // the contact input which input signal changed
    DIO_DataFetchScheduler* self = (DIO_DataFetchScheduler*)me;
    vector	items;

    // pulse conters & polling
    items = DIO_FetchTargets_GetFetchItems(self->mFetchTargets);
    if (! vector_is_empty(items)) {
        const DIO_FetchItem** itemsCurs = (const DIO_FetchItem**)vector_get_data(items);

        for (int i = 0, n = vector_size(items); i < n; i++) {
            const DIO_FetchItem* item = *itemsCurs++;

            if (item->isPulseCounter) {
                unsigned long pulseCount = 0;

                if (! DIO_Lib_ReadPulseCount(item->pinID, &pulseCount)) {
                    continue;
                };
                StringBuf_AppendByPrintf(me->mStringBuf, "%lu", pulseCount);
            } else {
                unsigned int currentStatus = 0;

                if (! DIO_Lib_ReadPinLevel(item->pinID, &currentStatus)) {
                    continue;
                };
                StringBuf_AppendByPrintf(me->mStringBuf, "%ld", currentStatus);
            }

            TelemetryItems_Add(me->mTelemetryItems,
                item->telemetryName, StringBuf_GetStr(me->mStringBuf));
            StringBuf_Clear(me->mStringBuf);
        }
    }

    // contact inputs
    if (DIO_Watcher_DoWatch(self->mWatcher)) {
        const vector	lastChanges = DIO_Watcher_GetLastChanges(self->mWatcher);

        for (int i = 0, n = vector_size(lastChanges); i < n; ++i) {
            DIO_WatchItemStat* wiStat;

            vector_get_at(&wiStat, lastChanges, i);

            StringBuf_AppendByPrintf(me->mStringBuf, "%ld", 1);
            TelemetryItems_Add(me->mTelemetryItems,
                wiStat->watchItem->telemetryName, StringBuf_GetStr(me->mStringBuf));
            StringBuf_Clear(me->mStringBuf);
        }
    }
}

DataFetchScheduler*
DIO_DataFetchScheduler_New(void)
{
    // initialize own menber and setup virtual method
    DIO_DataFetchScheduler* newObj =
        (DIO_DataFetchScheduler*)malloc(sizeof(DIO_DataFetchScheduler));
    DataFetchSchedulerBase* super;

    if (NULL == newObj) {
        goto err_malloc;
    }

    super = &newObj->Super;
    if (NULL == DataFetchScheduler_InitOnNew(
        super, DIO_FetchTimerCallback, DIO)) {
        goto err;
    }
    newObj->mFetchTargets = DIO_FetchTargets_New();
    if (NULL == newObj->mFetchTargets) {
        goto err_delete_super;
    }
    newObj->mWatcher = DIO_Watcher_New();
    if (NULL == newObj->mWatcher) {
        goto err_delete_fetchTargets;
    }

    super->DoDestroy = DIO_DataFetchScheduler_DoDestroy;
//	super->DoInit    = DIO_DataFetchScheduler_DoInit;  // don't override
    super->ClearFetchTargets = DIO_DataFetchScheduler_ClearFetchTargets;
    super->DoSchedule        = DIO_DataFetchScheduler_DoSchedule;

    return super;
err_delete_fetchTargets:
    DIO_FetchTargets_Destroy(newObj->mFetchTargets);
err_delete_super:
    DataFetchScheduler_Destroy(super);
err:
    free(newObj);
err_malloc:
    return NULL;
}

void
DIO_DataFetchScheduler_Init(DataFetchScheduler* me,
    vector fetchItemPtrs, vector watchItems)
{
    // reinitialize pulse count acquisition and contact inpput monitoring targes
    DIO_DataFetchScheduler* self = (DIO_DataFetchScheduler*)me;

    DataFetchScheduler_Init(me, fetchItemPtrs);
    DIO_Watcher_Init(self->mWatcher, watchItems);
}
