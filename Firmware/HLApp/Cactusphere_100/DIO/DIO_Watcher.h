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

#ifndef _DIO_WATCHER_H_
#define _DIO_WATCHER_H_

#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

#ifndef CONTAINERS_VECTOR_H
#include <vector.h>
#endif

typedef struct DIO_WatchItem	DIO_WatchItem;
typedef struct DIO_Watcher	DIO_Watcher;

// status of contact input monitoring target
typedef struct DIO_WatchItemStat {
    const DIO_WatchItem*	watchItem;  // watching specification
    unsigned long	prevPulseCount; // previous counter value
    unsigned long	currPulseCount; // last counter value
} DIO_WatchItemStat;

// Initialization and cleanup
extern DIO_Watcher*	DIO_Watcher_New(void);
extern void	DIO_Watcher_Init(DIO_Watcher* me, vector watchItems);
extern void	DIO_Watcher_Destroy(DIO_Watcher* me);

// Check update
extern bool	DIO_Watcher_DoWatch(DIO_Watcher* me);
extern const vector	DIO_Watcher_GetLastChanges(DIO_Watcher* me);

#endif  // _DIO_WATCHER_H_
