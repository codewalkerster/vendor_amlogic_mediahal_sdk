/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */



#include <string>
#include <vector>
#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <sys/time.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <thread>
#include <sys/mman.h>
#include "AmVideoDecBase.h"
#include <inttypes.h>
static uint32_t kDefaultQueueCount = 32;
using namespace std::chrono_literals;
#define MAX_INSTANCE_MUN  9
#define InputBufferMaxSize (1024 * 1024)
#define ALIGN(x, align) ((x) + (align -1) & (~(align -1)))

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

typedef enum {
    VFORMAT_UNKNOWN = -1,
    VFORMAT_MPEG12 = 0,
    VFORMAT_MPEG4,
    VFORMAT_H264,
    VFORMAT_MJPEG,
    VFORMAT_REAL,
    VFORMAT_JPEG,
    VFORMAT_VC1,
    VFORMAT_AVS,
    VFORMAT_SW,
    VFORMAT_H264MVC,
    VFORMAT_H264_4K2K,
    VFORMAT_HEVC,
    VFORMAT_H264_ENC,
    VFORMAT_JPEG_ENC,
    VFORMAT_VP9,
    VFORMAT_AVS2,
    VFORMAT_DVES_AVC,
    VFORMAT_DVES_HEVC,
    VFORMAT_MPEG2TS,
    VFORMAT_UNSUPPORT,
    VFORMAT_MAX
} vformat_t;


const uint32_t crc32_tab[] = { /* CRC32 feedback table. */
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static uint32_t crc32_gen(uint32_t crc, const void *buf, size_t size)
{
    const uint8_t *p;

    p = (const uint8_t *)buf;

    while (size--)
        crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

    return crc;
}

const char* vformat_to_mime(uint32_t vformat) {
    switch (vformat) {
        case 2:
            return "video/avc";
        case 11:
            return "video/hevc";
        case 14:
            return "video/x-vnd.on2.vp9";
        case 0:
            return "video/mpeg2";
        case 1:
            return "video/mp4v-es";
        case 3:
            return "video/x-motion-jpeg";
        case 6:
            return "video/vc1";
        case 7:
            return "video/avs";
        case 15:
            return "video/avs2";
        default:
            return "";
    }
}

static int sysfs_cmd_control(const char *path, const char *cmdStr)
{
    int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);

    if (fd >= 0) {
        write(fd, cmdStr, strlen(cmdStr));
        close(fd);
        return 0;
    }

    return -1;
}

class VideoDecPlayerExample {
public:
    VideoDecPlayerExample();
    virtual ~VideoDecPlayerExample();
    int init(uint32_t vFmt, uint32_t width = 1920, uint32_t height = 1080, uint32_t framerate = 24);
    void setQueueCount(int32_t queueCount);
    void play(const char* iname, const char* sname,const char* oName, int num);
    void outputBuffer(const char* oName, int num);
    int inputBuffer(uint8_t* buf, uint32_t size, uint64_t timestamp = 0);
    void dumpData(char* buf, uint32_t width, uint32_t height, int32_t bitstreamId);

    /* Implement callback */
    virtual void onOutputFormatChanged(uint32_t requestedNumOfBuffers,
    int32_t width, uint32_t height);
    virtual void onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
    uint32_t width, uint32_t height);
    virtual void onInputBufferDone(int32_t bitstream_buffer_id);
    virtual void onUpdateDecInfo(const uint8_t* info, uint32_t isize);
    virtual void onFlushDone();
    virtual void onResetDone();
    virtual void onError(int32_t error);
    virtual void onEvent(uint32_t event, void* param, uint32_t paramSize);

    class playerCallback : public AmVideoDecCallback {
    public:
        playerCallback(VideoDecPlayerExample *thiz) : mThis(thiz) {}
        virtual ~playerCallback() = default;
        virtual void onOutputFormatChanged(uint32_t requestedNumOfBuffers,
        int32_t width, uint32_t height) override {
            mThis->onOutputFormatChanged(requestedNumOfBuffers, width, height);
        }
        virtual void onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
        uint32_t width, uint32_t height) override {
            mThis->onOutputBufferDone(pictureBufferId, bitstreamId, width, height);
        }
        virtual void onInputBufferDone(int32_t bitstream_buffer_id) override {
            mThis->onInputBufferDone(bitstream_buffer_id);
        }
        virtual void onUserdataReady(const uint8_t* userdata, uint32_t usize) override {
            UNUSED(userdata);
            UNUSED(usize);
        }
        virtual void onUpdateDecInfo(const uint8_t* info, uint32_t isize) override {
            mThis->onUpdateDecInfo(info, isize);
        }
        virtual void onFlushDone() override {
            mThis->onFlushDone();
        }
        virtual void onResetDone() override {
            mThis->onResetDone();
        }
        virtual void onError(int32_t error) override {
            mThis->onError(error);
        }
        virtual void onEvent(uint32_t event, void* param, uint32_t paramSize) override {
            mThis->onEvent(event, param, paramSize);
        }

        private:
        VideoDecPlayerExample * const mThis;
    };

    playerCallback* mCallback;
    AmVideoDecBase* mAmVideoDec;

    std::mutex mFlushedLock;
    std::condition_variable mFlushedCondition;
    bool mReady;

    bool out_flag;

    FILE *miFp;
    FILE *msFp;
    FILE *moFp;
    FILE *mcFp;

    struct dispWork {
        int64_t bitstreamId;
        int64_t timestamp;
        int32_t pictureBufferId;
        uint32_t width;
        uint32_t height;
    };

    /*Ion output for non-bufferQueue*/
    struct mapInfo {
        void* map_addr;
        size_t size;
    };
    std::vector<struct mapInfo> mmapBuf;


private:
    uint64_t getTimeUs(void);
    uint64_t getTimeUsFromStart(void);

    /* Map: bitstreamId -> timestamp */
    std::map<int32_t, uint64_t> mInputWork;

    std::map<int32_t, uint8_t*> mInputBuffer;

    /* Map: slot -> pictureBufferId */
    std::map<int32_t, int32_t> mOutputBufferPictureId;

    /* Queue: bistreamId -> struct dispWork */
    std::queue<dispWork> mDisplayWork;
    /* output buffer mmap addr */
    std::vector<void*>mMmapAddr;

    std::mutex mInputLock;
    std::mutex mOutputLock;
    std::mutex mDumpLock;

    uint32_t mDqWidth;
    uint32_t mDqHeight;
    uint32_t mInputDoneCount;
    uint32_t mOutputDoneCount;
    uint32_t mOutputBufferNum;
    uint32_t mDumpNum;
    uint64_t mStartTime;
    uint32_t mBitStreamId;
    uint32_t mBufferWidth;
    uint32_t mBufferHeight;
};

uint64_t VideoDecPlayerExample::getTimeUs(void) {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1e6 + time.tv_usec;
}

uint64_t VideoDecPlayerExample::getTimeUsFromStart(void) {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1e6 + time.tv_usec - mStartTime;
}

int VideoDecPlayerExample::init(uint32_t vFmt, uint32_t width, uint32_t height, uint32_t framerate) {
    int ret = 0;

    init_param_t config = {
    /* video */256, width, height, framerate, vFmt, 0,
    /* audio */0, 2, 44100, 2,
    /*pcrid */ 0,
    /* display */1, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 1};

    mAmVideoDec->setQueueCount(kDefaultQueueCount);
    ret = mAmVideoDec->initialize(vformat_to_mime(vFmt), (uint8_t*)&config, sizeof(init_param_t), false, true);
    if (ret) {
        printf("init failed!, ret=%d\n", ret);
        return -2;
    }

    return 0;
}

int VideoDecPlayerExample::inputBuffer(uint8_t* buf, uint32_t size, uint64_t timestamp) {
    if (buf == NULL || size == 0) {
        printf("please input valid data\n");
        return -1;
    }

    mInputBuffer[mBitStreamId] = buf;

    int retry = 500;
    int32_t err = 0;
    do {
        err = mAmVideoDec->queueInputBuffer(mBitStreamId, mInputBuffer[mBitStreamId], 0, size, timestamp);
        if (err == -EAGAIN) {
            usleep(10000);
        } else {
            break;
        }
    } while(err || retry-- > 0);
    std::lock_guard<std::mutex> lock(mInputLock);
    mInputWork[mBitStreamId] = timestamp;
    mBitStreamId++;

    return 0;
}

void VideoDecPlayerExample::dumpData(char* buf, uint32_t width, uint32_t height, int32_t bitstreamId) {
    std::lock_guard<std::mutex> lock(mDumpLock);
    uint32_t y_size = (mBufferHeight* mBufferWidth);
    int i = 0;

    if (moFp) {
        /* save output yuv data */
        for (i = 0 ; i < (int)height; i++) {
            fwrite(buf + i * mBufferWidth, width, 1, moFp);
        }
        for (i = 0 ; i < (int)height / 2; i++) {
            fwrite(buf + y_size + i * mBufferWidth, width, 1, moFp);
        }

        fflush(moFp);
        fsync(fileno(moFp));

        printf("Dump YUV y_size %d, bitstreamId %d\n",
        y_size, bitstreamId);
    }
    if (mcFp) {
        /* save output crc data */
        int crc_y = 0,crc_uv = 0;

        for (i = 0 ; i < (int)height; i++) {
            crc_y = crc32_gen(crc_y, (unsigned char const *)(buf + i * mBufferWidth), width);
        }
        for (i = 0 ; i < (int)height / 2; i++) {
            crc_uv = crc32_gen(crc_uv, (unsigned char const *)(buf + y_size + i * mBufferWidth), width);
        }

        fprintf(mcFp, "%08d: %08x %08x\n", mDumpNum, crc_y, crc_uv);
        fflush(mcFp);
        fsync(fileno(mcFp));
        printf("%08d: %08x %08x\n", mDumpNum, crc_y, crc_uv);
    }
    mDumpNum++;
}


void VideoDecPlayerExample::outputBuffer(const char* oName, int num) {
    int32_t bitstreamId = 0;
    int64_t timestamp = 0;
    int32_t pictureBufferId = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    if (mDisplayWork.empty()) {
        usleep(5000);
        return;
    }
    std::lock_guard<std::mutex> lock(mOutputLock);

    dispWork* work = &mDisplayWork.front();
    if (work == NULL) {
        return ;
    }

    bitstreamId = work->bitstreamId;
    timestamp = work->timestamp;
    pictureBufferId = work->pictureBufferId;
    width = work->width;
    height = work->height;

    if (oName != NULL && moFp == NULL && mcFp == NULL) {
        char newOName[128];
        char newCName[128];

        sprintf(newOName, "%s_%d_%d_%d.yuv", oName, num, width, height);
        sprintf(newCName, "%s_%d_%d_%d.crc", oName, num, width, height);
        moFp = fopen(newOName, "wb");
        if (!moFp) {
            printf("Unable to open output YUV file\n");
            return;
        }
        setbuf(moFp,NULL);

        mcFp = fopen(newCName, "w");
        if (!mcFp) {
            printf("Unable to open output crc file\n");
            return;
        }
        setbuf(mcFp,NULL);
    }

    char* vaddr = (char *)mMmapAddr[pictureBufferId];

    dumpData(vaddr, width, height, bitstreamId);

    mAmVideoDec->queueOutputBuffer(pictureBufferId);

    mDisplayWork.pop();
}

AmVideoDecBase* getAmVideoDec(AmVideoDecCallback* callback) {
    void *libHandle = dlopen("libmediahal_videodec.so", RTLD_NOW);

    if (libHandle == NULL) {
        libHandle = dlopen("libmediahal_videodec.system.so", RTLD_NOW);
        if (libHandle == NULL) {
            printf("unable to dlopen libmediahal_videodec.so: %s\n", dlerror());
            return nullptr;
        }
    }

    typedef AmVideoDecBase *(*createAmVideoDecFunc)(AmVideoDecCallback* callback);

    createAmVideoDecFunc getAmVideoDec =
    (createAmVideoDecFunc)dlsym(libHandle, "AmVideoDec_create");

    if (getAmVideoDec == NULL) {
        dlclose(libHandle);
        libHandle = NULL;
        printf("can not create AmVideoDec\n");
        return nullptr;
    }
    AmVideoDecBase* halHanle = (*getAmVideoDec)(callback);
    printf("getAmVideoDec ok\n");
    return halHanle;
}

VideoDecPlayerExample::VideoDecPlayerExample() {
    moFp = nullptr;
    miFp = nullptr;
    mcFp = nullptr;
    msFp = nullptr;
    mStartTime = getTimeUs();
    mInputDoneCount = 0;
    mOutputDoneCount = 0;
    mOutputBufferNum = 0;
    mDumpNum = 0;
    mReady = false;
    out_flag = false;
    mBitStreamId = 0;
    mCallback = new playerCallback(this);
    mAmVideoDec = getAmVideoDec(mCallback);
}

VideoDecPlayerExample::~VideoDecPlayerExample() {
    delete mCallback;
    delete mAmVideoDec;

    if (miFp) {
        fclose(miFp);
    }
    if (msFp) {
        fclose(msFp);
    }
    if (moFp) {
        fclose(moFp);
    }
    if (mcFp) {
        fclose(mcFp);
    }

    std::map<int32_t, uint8_t*>::iterator iter;
    iter = mInputBuffer.begin();
    while (iter != mInputBuffer.end()) {
        free(iter->second);
        iter->second = NULL;
        iter++;
    }
}


void VideoDecPlayerExample::play(const char* iname, const char* sname, const char* oName, int num) {
    int end = 0;
    int ret = 0;

    if (iname == NULL) {
        printf("iname is null\n");
        return ;
    } else {
    miFp = fopen(iname, "rb");
    if (!miFp) {
        printf("open input file error %s!\n",iname);
        return;
    }
    }

    if (sname == NULL) {
        printf("sname is null\n");
        return ;
    } else {
        msFp = fopen(sname, "rb");
        if (!msFp) {
            printf("open frame size file error!\n");
            return;
        }
    }

    printf("\n*********VideoDec PLAYER DEMO************\n\n");
    printf("file %s to be played\n", iname);

    std::atomic_bool running(true);
    std::thread outputThread([this, &running, oName, num]() {
        while (running) {
            outputBuffer(oName, num);
        }
    });

    while (1) {
        char frame_size_str[32];
        char *s_rt;
        uint32_t frame_size;
        uint8_t * newbuf;

        if (mInputBuffer.size() > 50) {
            printf("sleep 20000\n");
            usleep(20000);
        }

        memset(frame_size_str, 0, sizeof(frame_size_str));
        s_rt = fgets(frame_size_str, 32, msFp);

        if (s_rt == NULL) {
            break;
        }
        frame_size = atoi(frame_size_str);

        if (frame_size <= InputBufferMaxSize) {
            newbuf = (uint8_t *)malloc(frame_size);
            if (newbuf == NULL) {
                printf("malloc frame size %d fail\n", frame_size);
            }
            memset(newbuf, 0, frame_size);
            ret = fread(newbuf, 1, frame_size, miFp);
            printf("read size %d, %x %x %x %x %x %x %x %x ...\n", frame_size,
            newbuf[0], newbuf[1], newbuf[2], newbuf[3], newbuf[4], newbuf[5], newbuf[6], newbuf[7]);

            if (ret < (int)frame_size) {
                printf("read back size %d, less than frame size %d\n", ret, frame_size);
                return ;
            }
        } else {
            printf("Error:input frame size(%d) is error!\n",frame_size);
            return;
        }

        ret = inputBuffer(newbuf, frame_size, 0);
        if (ret) {
            printf("Error:input buffer is error!\n");
            return;
        }

        if (feof(miFp) || feof(msFp)) {
            end = 1;
        }
        if (end) {
            break;
        }
    }

    mAmVideoDec->flush();

    std::unique_lock <std::mutex> l(mFlushedLock);
    while (!mReady) {
        mFlushedCondition.wait(l);
    }

    while ((mDisplayWork.size() > 0)) {
        usleep(10000);
    }

    running.store(false);
    outputThread.join();
    mAmVideoDec->destroy();
}

void VideoDecPlayerExample::onOutputFormatChanged(uint32_t requestedNumOfBuffers,
                                                  int32_t width,
                                                  uint32_t height) {

    printf("onOutputFormatChanged bufnum %d, width %d, height %d\n",
                requestedNumOfBuffers,
                width,
                height);

    /* resolution change clean resource */
    if (mOutputBufferNum > 0) {
        printf("resolution change free all buffer, mOutputBufferNum %d\n", mOutputBufferNum);
#if ANDROID_PLATFORM_SDK_VERSION >= 30
        mAmVideoDec->freeUvmBuffers();
#else
        mAmVideoDec->freeAllIonBuffer();
#endif
    }


#if ANDROID_PLATFORM_SDK_VERSION >= 30
    mOutputBufferNum = requestedNumOfBuffers;
    mDqWidth = width;
    mDqHeight = height;
    mBufferWidth = ALIGN(width, 64);
    mBufferHeight = ALIGN(height, 64);
    uint32_t imagesize = (mBufferHeight * mBufferWidth * 3) / 2;
    mAmVideoDec->setupOutputBufferNum(mOutputBufferNum);
    for (uint32_t i = 0; i < mOutputBufferNum; i++) {
        uint8_t* vaddr;
        int fd;
        mAmVideoDec->allocUvmBuffer(mBufferWidth, mBufferHeight, (void**)&vaddr, i, &fd);
        mMmapAddr.push_back((void*)vaddr);
        printf("allocUvmBuffer fd %d, size = %d\n", fd, imagesize);
        mAmVideoDec->createOutputBuffer(i, fd);
    }
#else
    mOutputBufferNum = requestedNumOfBuffers;
    mDqWidth = width;
    mDqHeight = height;
    uint32_t imagesize = (width * height * 3) / 2;
    mAmVideoDec->setupOutputBufferNum(mOutputBufferNum);

    for (uint32_t i = 0; i < mOutputBufferNum; i++) {
        uint8_t* vaddr;
        int fd;

        mAmVideoDec->allocIonBuffer(imagesize, (void**)&vaddr, &fd);
        mMmapAddr.push_back((void*)vaddr);
        printf("allocIonBuffer fd %d, size = %d\n", fd, imagesize);
        mAmVideoDec->createOutputBuffer(i, fd);
    }
#endif

    printf("onOutputFormatChanged out timeUs %" PRId64 "\n", getTimeUsFromStart());
}


void VideoDecPlayerExample::onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
                                               uint32_t width, uint32_t height) {
    printf("onOutputBufferDone this %p, pictureBufferId %d, bitstreamId %" PRId64 ", mInputWork.size %d, In %d, Out %d\n",
                        this, pictureBufferId, bitstreamId, (int)mInputWork.size(), mInputDoneCount, mOutputDoneCount);
    std::lock_guard<std::mutex> lock(mOutputLock);
    int64_t timestamp = mInputWork[bitstreamId];
    printf("onOutputBufferDone this %p, timestamp %" PRId64 ", width %d, height %d\n",this, timestamp, width, height);

    mDisplayWork.push({bitstreamId, timestamp, pictureBufferId, width, height});
    mInputWork.erase(bitstreamId);
    mOutputDoneCount++;
}

void VideoDecPlayerExample::onInputBufferDone(int32_t bitstreamId) {
    printf("%s bitstream_buffer_id:%d\n", __func__, bitstreamId);
    std::lock_guard<std::mutex> lock(mInputLock);
    mInputBuffer.erase(bitstreamId);
    free(mInputBuffer[bitstreamId]);
    mInputDoneCount++;
}

void VideoDecPlayerExample::onUpdateDecInfo(const uint8_t* info, uint32_t isize) {
    printf("%s info:%s isize:%d\n", __func__, info, isize);
}

void VideoDecPlayerExample::onFlushDone() {
    printf("onFlushDone\n");
    std::unique_lock <std::mutex> l(mFlushedLock);
    mReady = true;
    mFlushedCondition.notify_all();
}

void VideoDecPlayerExample::onResetDone() {
    printf("onResetDone\n");
}

void VideoDecPlayerExample::onError(int32_t error) {
    printf("%s error%d\n", __func__, error);
}

void VideoDecPlayerExample::onEvent(uint32_t event, void* param, uint32_t paramSize) {
    UNUSED(param);
    UNUSED(paramSize);
    printf("%s event:%x, %s %d\n", __func__, event, (char*)param, paramSize);
}

static void usage(void)
{
    printf("VideoDecPlayer\n");
    printf("Usage: VideoDecPlayerExample -i <file> -f <format> -s <frame size> [-n <number>] [-o <out yuv and crc>] [-h <help>]\n");
    printf("\n");
    printf(" -i, --ifile		input es file\n");
    printf(" -f, --format		video format\n");
    printf ("\t\t0:mpeg12\t1:mpeg4\t\t2:h264\t\t3:mjpeg\n\
5:jpeg\t\t6:vcl\t\t7:avs\t\t11:hevc\n\
14:vp9\t\t15:avs2\t\t16:av1\n");
    printf(" -s, --size 	frame size file\n");
    printf(" -n, --number	instance number\n");
    printf(" -o, --output	output yuv and crc\n");
    printf(" -h, --help	usage\n");
}


int main(int argc, char** argv) {
    int optionChar = 0;
    int optionIndex = 0;
    int32_t   vFmt = -1;
    char* iname = nullptr;
    char* sname = nullptr;
    char* oName = nullptr;
    int num = 1;

    if (argc < 7) {
        usage();
        return -1;
    }

    const char *shortOptions = "i:f:s:n:o:h";
    struct option longOptions[] = {
        { "ifile", required_argument, nullptr, 'i' },
        { "format", required_argument, nullptr, 'f' },
        { "size", required_argument, nullptr, 's' },
        { "number", required_argument, nullptr, 'n' },
        { "output", required_argument, nullptr, 'o' },
        { "help", no_argument, nullptr,  'h'},
        { nullptr, 0, nullptr, 0 },
    };

    while ((optionChar = getopt_long(argc, argv, shortOptions,longOptions, &optionIndex)) != -1) {
        switch (optionChar) {
            case 'i':
                iname = optarg;
                break;
            case 'f':
                vFmt = atoi(optarg);
                break;
            case 's':
                sname = optarg;
                break;
            case 'n':
                num = atoi(optarg);
                break;
            case 'o':
                oName = optarg;
                break;
            case 'h':
                usage();
                exit(-1);
            default:
                break;
        }
    }

    if (iname == nullptr || vFmt == -1 || sname == nullptr) {
        usage();
        exit(-1);
    }

    if (num > MAX_INSTANCE_MUN) {
        printf("input instance num is more than 9\n");
        exit(-1);
    }

    sysfs_cmd_control("/sys/module/amvdec_ports/parameters/bypass_vpp", "1");

    std::vector<std::thread> threads;

    for (int i = 1 ; i <= num ; i++) {
        threads.push_back(std::thread([iname, sname, oName, vFmt, i]() {
            printf("start player %d\n", i);
            VideoDecPlayerExample player;
            player.init(vFmt);
            printf("iname %s,sname %s,vFmt = %d\n",iname,sname,vFmt);
            player.play(iname,sname,oName,i);
        }));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    printf("VideoDecPlayerExample exit\n");
    sysfs_cmd_control("/sys/module/amvdec_ports/parameters/bypass_vpp", "0");

    return 0;
}

