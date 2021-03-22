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

#ifndef _DIO_WATCH_CONFIG_H_
#define _DIO_WATCH_CONFIG_H_

#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

#ifndef CONTAINERS_VECTOR_H
#include <vector.h>
#endif

#include "DIO_PropertyItem.h"

typedef struct DIO_WatchConfig	DIO_WatchConfig;
typedef struct _json_value	json_value;

#ifndef NUM_DIO
#define NUM_DIO 2
#endif

// Initialization and cleanup
extern DIO_WatchConfig*	DIO_WatchConfig_New(void);
extern void	DIO_WatchConfig_Destroy(DIO_WatchConfig* me);

// Load DIO contact input watcher configuration from JSON
extern bool	DIO_WatchConfig_LoadFromJSON(DIO_WatchConfig* me, DIO_PropertyData* data,
    const json_value* json, vector propertyItem, const char* version);

// Get configuration of DIO contact input watchers
extern vector	DIO_WatchConfig_GetFetchItems(DIO_WatchConfig* me);

// Get enable port number of DIO contact input
int DIO_WatchConfig_GetWatchEnablePorts(DIO_WatchConfig* me, bool* status);

#endif  // _DIO_WATCH_CONFIG_H_
