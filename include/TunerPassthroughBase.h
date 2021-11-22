/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef TUNER_PASSTHROUGH_BASE_H_
#define TUNER_PASSTHROUGH_BASE_H_

#include <stdint.h>

typedef int (*callbackFunc)(void*obj, void* args);

typedef struct PASSTHROUGH_INIT_PARAMS
{
    int32_t dmx_id;
    int32_t video_pid;
    int32_t hw_sync_id;
    int32_t secure_mode;
    const char* mime;
    void* tunnel_renderer;
} passthroughInitParams;

struct TunerPassthroughBase
{
public:

    TunerPassthroughBase() {};
    virtual ~TunerPassthroughBase() {};
    virtual int Init(passthroughInitParams* params);
    virtual int RegCallBack(int cb_id, callbackFunc funs, void* obj);
    virtual int GetSyncInstansNo(int *no);
    virtual int Start();
    virtual int Stop();

};

extern "C" TunerPassthroughBase* TunerPassthroughBase_create();

#endif
