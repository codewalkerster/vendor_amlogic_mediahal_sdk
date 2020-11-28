/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef VideoTunnelRenderer_BASE_H_
#define VideoTunnelRenderer_BASE_H_

#include <stdint.h>

typedef int (*callbackFunc)(void*obj, void* args);

class VideoTunnelRendererBase
{

public:

    enum {
        CB_FILLVIDEOFRAME,
        CB_FUNS_MAX,
    };

    VideoTunnelRendererBase() {};
    virtual ~VideoTunnelRendererBase() {};
    virtual bool init(int hwsyncid);
    virtual int getTunnelId();
    virtual bool start();
    virtual bool stop();
    virtual bool sendVideoFrame(int metafd, int64_t timestampNs);
    virtual int regCallBack(int cb_id, callbackFunc funs, void* obj);
    virtual bool flush();

};

extern "C" VideoTunnelRendererBase* VideoTunnelRenderer_create();

#endif