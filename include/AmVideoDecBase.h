/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */



#ifndef AM_VIDEO_DEC_BASE_H
#define AM_VIDEO_DEC_BASE_H

#include <stdint.h>

typedef struct {
    /* video */
    uint32_t    vpid;
    uint32_t    nVideoWidth;
    uint32_t    nVideoHeight;
    uint32_t    nFrameRate;
    uint32_t   vFmt;
    uint32_t    drmMode;
    /* audio */
    uint32_t	apid;
    uint32_t    nChannels;
    uint32_t    nSampleRate;
    uint32_t   aFmt;
    /* pcrid */
    uint32_t	pcrid;
    /* display */
    uint32_t dispMode;
    uint32_t    nSidebandType;
    uint32_t    nSidebandId;
    uint32_t    nAvsyncMode;
    int subtitleFlg;
    int32_t  mDemuxType;
    int32_t dmx_dev_id;
    int32_t dmx_player_id;
    unsigned int  stbuf_start;
    unsigned int  stbuf_size;
    uint32_t    nDecType;
} init_param_t;

class AmVideoDecCallback {
public:
    virtual ~AmVideoDecCallback() {};
    virtual void onOutputFormatChanged(uint32_t requested_num_of_buffers,
                int32_t width, uint32_t height);
    virtual void onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
                uint32_t width, uint32_t height);
    virtual void onInputBufferDone(int32_t bitstream_buffer_id);
    virtual void onUpdateDecInfo(const uint8_t* info, uint32_t isize);
    virtual void onFlushDone();
    virtual void onResetDone();
    virtual void onError(int32_t error);
    virtual void onUserdataReady(const uint8_t* userdata, uint32_t usize);
    virtual void onEvent(uint32_t event, void* param, uint32_t paramsize);
};

class AmVideoDecBase {

public:
    AmVideoDecBase(AmVideoDecCallback* callback) { (void)&callback; };
    virtual ~AmVideoDecBase() {};

    virtual int32_t initialize(const char* mime, uint8_t* config, uint32_t configLen,
            bool secureMode, bool useV4l2 = 1);
    virtual int32_t setQueueCount(uint32_t queueCount);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, int ashmemFd, off_t offset,
            uint32_t bytesUsed, uint64_t timestamp);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, int ashmemFd, off_t offset,
            uint32_t bytesUsed, uint64_t timestamp,
            uint8_t* hdrbuf, uint32_t hdrlen);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, uint8_t* pbuf,
            off_t offset, uint32_t bytesUsed, uint64_t timestamp);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, uint8_t* pbuf,
            off_t offset, uint32_t bytesUsed, uint64_t timestamp,
            uint8_t* hdrbuf, uint32_t hdrlen);
    virtual int32_t setupOutputBufferNum(uint32_t numOutputBuffers);
    virtual int32_t createOutputBuffer(uint32_t pictureBufferId,
                    int32_t dmabufFd, bool nv21 = 1, int32_t metaFd = -1);
    virtual int32_t createOutputBuffer(uint32_t pictureBufferId,
                    uint8_t* buf, size_t size, bool nv21 = 1);
    virtual int32_t queueOutputBuffer(int32_t pictureBufferId);
    virtual void flush();
    virtual void reset();
    virtual void destroy();
    virtual int32_t sendCommand(uint32_t index, void* param, uint32_t size);

    /* Ion output for non-bufferqueue */
    virtual int32_t allocIonBuffer(size_t size, void** mapaddr, int* fd = 0);
    virtual int32_t freeIonBuffer(void* mapaddr);
    virtual int32_t freeAllIonBuffer();
};

extern "C" AmVideoDecBase* AmVideoDec_create(AmVideoDecCallback* callback);
extern "C" uint32_t AmVideoDec_getVersion(uint32_t* versionM, uint32_t* verionL);

#endif  // AM_VIDEO_DEC_BASE_H
