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
#include "AmlMessageBase.h"

#define AM_VIDEO_DEC_INIT_FLAG_DEFAULT              0
#define AM_VIDEO_DEC_INIT_FLAG_CODEC2               1
#define AM_VIDEO_DEC_INIT_FLAG_STREAMMODE           (1ul << 1)
#define AM_VIDEO_DEC_INIT_FLAG_DMXDATA_SOURCE       (1ul << 2)
#define AM_VIDEO_DEC_INIT_FLAG_TSPLAYER             (1ul << 3)
#define AM_VIDEO_DEC_INIT_FLAG_VIDEO_PASSTHROUGH    (1ul << 4)

#define AM_VIDEO_DEC_INIT_FLAG_USE_INPUT_BUFFER_POOL (1ul << 4)
#define AM_VIDEO_DEC_INIT_FLAG_USE_DYNAMIC_CAPTURE_BUFFER (1ul << 5)
#define AM_VIDEO_DEC_INIT_FLAG_USE_UVM               (1ul << 6)
#define AM_VIDEO_DEC_INIT_FLAG_USE_PTS_CALCULATOR    (1ul << 7)
#define AM_VIDEO_DEC_INIT_FLAG_USE_SECURE_VDEC_HAL   (1ul << 8)
#define AM_VIDEO_DEC_INIT_FLAG_USE_HI_PRIO           (1ul << 9)
#define AM_VIDEO_DEC_INIT_FLAG_USE_LOW_LATENCY_MODE  (1ul << 10)
enum class InputCodec {
  H264,
  H265,
  VP9,
  AV1,
  DVHE,
  DVAV,
  DVAV1,
  MP2V,
  MP4V,
  MJPG,
  AVS3,
  AVS2,
  AVS,
  VC1,
  UNKNOWN = 0xff,
};


typedef enum {
    GET_DECODER_FEATURE_LIST_SIZE = 0,
    GET_DECODER_FEATURE_LIST = 1,
    GET_DECODER_INFO_MAX = 255,
} decoder_info_parameter;

typedef struct {
    uint8_t *data;
    size_t data_len;
    size_t actual_len;
} decoder_feature_info;



typedef struct {
    /* video */
    uint32_t    vpid;
    uint32_t    nVideoWidth;
    uint32_t    nVideoHeight;
    uint32_t    nFrameRate;
    uint32_t    vFmt;
    uint32_t    drmMode;
    /* audio */
    uint32_t	apid;
    uint32_t    nChannels;
    uint32_t    nSampleRate;
    uint32_t    aFmt;
    /* pcrid */
    uint32_t	pcrid;
    /* display */
    uint32_t    dispMode;
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
    int32_t nVideoRecoveryValue;
    int32_t unStablePts;
    int32_t sourceType;
} init_param_t;

typedef struct {
    /*inherit old interface*/
    char mime[64];
    uint8_t* config;
    uint32_t configLen;
    bool secureMode;
    bool useV4l2;
    bool isTunnelMode;
    int32_t flags;
    /*decode info*/
    int32_t dispMode;
    int32_t pipelineMode;
    int32_t extDecInfo;//reserved
    int32_t resInfo1;
    int32_t resInfo2;
    /*resource manage info*/
    char resAppName[64];
    void (* resCallback)(void * resOpaque);
    void* resOpaque;
    void (* resReport)(void * resOpaque);
}video_dec_init_params;

enum PictureFlag {
  PICTURE_FLAG_NONE = 0,
  PICTURE_FLAG_KEYFRAME = 0x0001,
  PICTURE_FLAG_PFRAME = 0x0002,
  PICTURE_FLAG_BFRAME = 0x0004,
};

enum ResetFlag {
  RESET_FLAG_NONE = 0,
  RESET_FLAG_NOWAIT = 0x0001,
};

typedef struct {
  int32_t pictureBufferId;
  int64_t bitstreamId;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  int32_t flags;
  uint64_t timestamp;
} output_buf_param_t;

class AmVideoDecCallback {
public:
    virtual ~AmVideoDecCallback() {};
    virtual void onOutputFormatChanged(uint32_t requested_num_of_buffers,
                int32_t width, uint32_t height);
    virtual void onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
                uint32_t width, uint32_t height);
    virtual void onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
                uint32_t width, uint32_t height, int32_t flags) {
        (void)flags;
        onOutputBufferDone(pictureBufferId, bitstreamId, width, height);
    }
    virtual void onOutputBufferDone(output_buf_param_t* params) {
       (void)params;
    }

    virtual void onInputBufferDone(int32_t bitstream_buffer_id);
    virtual void onUpdateDecInfo(const uint8_t* info, uint32_t isize);
    virtual void onFlushDone();
    virtual void onResetDone();
    virtual void onError(int32_t error);
    virtual void onUserdataReady(const uint8_t* userdata, uint32_t usize);
    virtual void onEvent(uint32_t event, void* param, uint32_t paramSize);
    virtual void onInputBufferInfo(int32_t bitstream_buffer_id, uint32_t bytesUsed,
                        uint64_t timestamp) {
        (void)bitstream_buffer_id;
        (void)bytesUsed;
        (void)timestamp;
    }
};

class AmVideoDecBase {

public:
    AmVideoDecBase(AmVideoDecCallback* callback) { (void)&callback; };
    virtual ~AmVideoDecBase() {};

    virtual int32_t initialize(const char* mime, uint8_t* config, uint32_t configLen,
            bool secureMode, bool useV4l2 = 1, int32_t flags = 0);
    virtual int32_t setQueueCount(uint32_t queueCount);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, int ashmemFd, off_t offset,
            uint32_t bytesUsed, uint64_t timestamp, int32_t flags = 0);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, int ashmemFd, off_t offset,
            uint32_t bytesUsed, uint64_t timestamp,
            uint8_t* hdrBuf, uint32_t hdrlen, int32_t flags = 0);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, uint8_t* pbuf,
            off_t offset, uint32_t bytesUsed, uint64_t timestamp, int32_t flags = 0);
    virtual int32_t queueInputBuffer(int32_t bitstreamId, uint8_t* pbuf,
            off_t offset, uint32_t bytesUsed, uint64_t timestamp,
            uint8_t* hdrBuf, uint32_t hdrlen, int32_t flags = 0);
    virtual int32_t setupOutputBufferNum(uint32_t numOutputBuffers);
    virtual int32_t createOutputBuffer(uint32_t pictureBufferId,
                    int32_t dmabufFd, bool nv21 = 1, int32_t metaFd = -1);
    virtual int32_t createOutputBuffer(uint32_t pictureBufferId,
                    uint8_t* buf, size_t size, bool nv21 = 1);
    virtual int32_t queueOutputBuffer(int32_t pictureBufferId);
    virtual void flush();
    virtual void reset(uint32_t flags = 0);
    virtual void destroy();
    virtual int32_t sendCommand(uint32_t index, void* param, uint32_t size);
    virtual bool getDecoderMessage(uint32_t type, void *data);
    virtual bool sendMessagetoDecoder(uint32_t type, void *data);

    /* Ion output for non-bufferQueue */
    virtual int32_t allocIonBuffer(size_t size, void** mapAddress, int* fd = 0);
    virtual int32_t freeIonBuffer(void* mapAddress);
    virtual int32_t freeAllIonBuffer();

    /* uvm output for non-bufferQueue */
    virtual int32_t allocUvmBuffer(uint32_t width, uint32_t height, void** mapAddress, unsigned int i,
        int* fd = 0);
    virtual int32_t freeUvmBuffers();

    /* tunnel mode outbuffer */
    virtual int32_t allocTunnelBuffer(int usage, uint32_t format, int stride, uint32_t width, uint32_t height, bool secure, int* fd);
    virtual int32_t freeTunnelBuffer(int fd);

    /* post and reply message */
    virtual bool postAndReplyMsg(AmlMessageBase *msg);

    /*new interface for resman*/
    virtual int32_t initialize(video_dec_init_params* initParams);
    virtual int32_t getWorkMode();
    virtual int32_t setWorkMode(uint32_t mode);
    virtual void setSyncPlayerInstanceNo(int32_t syncPlayerInstanceNo);
};

extern "C" AmVideoDecBase* AmVideoDec_create(AmVideoDecCallback* callback);

extern "C" uint32_t AmVideoDec_getVersion(uint32_t* versionM, uint32_t* versionL);
extern "C" AmlMessageBase* AmVideoDec_getAmlMessage();
extern "C" uint32_t AmVideoDec_getVersionString(char** data);
extern "C" uint32_t  AmVideoDec_getVideoDecoderInfo(decoder_info_parameter type, void* arg);


#endif  // AM_VIDEO_DEC_BASE_H
