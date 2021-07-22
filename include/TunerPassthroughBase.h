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

typedef struct DECODERBUFFERSTATUS {
    int64_t nFreePercentage;
    int64_t nSystemTimeNs;
} DECODERBUFSTATUS;

struct TunerPassthroughBase
{

public:
    enum {
        CB_NODIFYDECODERBUFFERSTATUS,
        CB_FUNS_MAX,
    };

    TunerPassthroughBase() {};
    virtual ~TunerPassthroughBase() {};
    virtual int Init(int dmx_id, int video_pid, const char* mime, int hw_sync_id, int secure_mode);
    virtual int RegCallBack(int cb_id, callbackFunc funs, void* obj);
    virtual int GetSyncInstansNo(int *no);
    virtual int Start();
    virtual int Stop();

};

extern "C" TunerPassthroughBase* TunerPassthroughBase_create();

#endif
