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
#include "AmlMessageBase.h"

typedef int (*callbackFunc)(void*obj, void* args);

struct renderTime
{
    int64_t mediaUs;
    int64_t renderUs;
};

struct fillVideoFrame2
{
   int fd;
   bool rendered;
};

struct renderframe
{
    int fd;
    int64_t timestampUs;
    bool renderAtonce;
    int64_t bitstreamId;
    int64_t reserved[4];
};

struct tunnelEventParam
{
    uint32_t type;
    void* data;
    uint32_t paramSize;
};

typedef enum {
        AM_VT_PARAM_MUTE,
        AM_VT_PARAM_SCREEN_COLOR,
        AM_VT_PARAM_TRANSITION_MODE_BEFORE,
        AM_VT_PARAM_TRANSITION_MODE_AFTER,
        AM_VT_PARAM_TRANSITION_PREROLL_RATE,
        AM_VT_PARAM_TRANSITION_PREROLL_AV_TOLERANCE,
        AM_VT_PARAM_PAUSE_RESUME,
    } renderParamsType;

struct renderParams
{
    int64_t param1;
    int64_t param2;
    int64_t param3;
    char reserved[64];
};

struct playerInfo {
    int32_t instID;
    int32_t decoderID;
};

class VideoTunnelRendererBase
{

public:

    enum {
        CB_FILLVIDEOFRAME,
        CB_NODIFYRENDERTIME,
        CB_FILLVIDEOFRAME2,
        CB_EVENT,
        CB_FUNS_MAX,
    };

    VideoTunnelRendererBase() {};
    virtual ~VideoTunnelRendererBase() {};
    virtual bool init(int hwSyncId);
    virtual int getTunnelId();
    virtual bool start();
    virtual bool stop();
    virtual bool sendVideoFrame(int metaFd, int64_t timestampNs, bool renderAtonce);
    virtual int regCallBack(int cb_id, callbackFunc func, void* obj);
    virtual bool flush();
    virtual bool flushSeekTrickMode();
    virtual bool setFrameRate(int32_t framerate);
    virtual bool peekFirstFrame();
    virtual void onVideoSyncQueueVideoFrame(int64_t timestampUs, uint32_t size);
    virtual void setTrickMode(uint32_t trickmode);
    virtual void startFast(float scale);
    virtual bool getDisplayFrameFlag(int frame);
    virtual void setDisplayFrameFlag(int frame, bool value);
    virtual void setRenderParams(int32_t type, renderParams* params);
    virtual void setWorkMode(uint32_t workmode);
    virtual bool postAndReplyMsg(AmlMessageBase *msg);
};

extern "C" VideoTunnelRendererBase* VideoTunnelRenderer_create();
extern "C" AmlMessageBase* VideoTunnelRenderer_getAmlMessage();

#endif
