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
#include <inttypes.h>
#include "AmVideoDecBase.h"

#define READ_SIZE (512 * 1024)

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif


class vesplayer {
public:
    vesplayer(init_param_t * config);
    virtual ~vesplayer() {
        delete mCallback;
        delete mAmVideoDec;
        if (mFp) {
            fclose(mFp);
        }
        std::map<int32_t, uint8_t*>::iterator iter;
        iter = mInputBuffer.begin();
        while (iter != mInputBuffer.end()) {
            free(iter->second);
            iter->second = NULL;
            iter++;
        }
    }
    void play(const char* iname);

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
    virtual void onEvent(uint32_t event, void* param, uint32_t paramsize);

    class playerCallback : public AmVideoDecCallback {
    public:
        playerCallback(vesplayer *thiz) : mThis(thiz) {}
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
        virtual void onEvent(uint32_t event, void* param, uint32_t paramsize) override {
            mThis->onEvent(event, param, paramsize);
        }

    private:
        vesplayer * const mThis;
    };

    playerCallback* mCallback;
    AmVideoDecBase* mAmVideoDec;
    init_param_t *mConfig;
    std::mutex mInputLock;
    std::map<int32_t, uint8_t*> mInputBuffer;
    std::mutex mEofLock;
    std::condition_variable mEofcv;
    int mEof;
    FILE *mFp;
};

// global
static int axis[8] = {0};
static vesplayer* gEsplayer = nullptr;

static int sysfs_cmd_control(const char *path, const char *cmdstr)
{
    int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);

    if (fd >= 0) {
        write(fd, cmdstr, strlen(cmdstr));
        close(fd);
        return 0;
    }

    return -1;
}

static int parse_para(const char *para, int para_num, int *result)
{
    char *endp;
    const char *startp = para;
    int *out = result;
    int len = 0, count = 0;

    if (!startp) {
        return 0;
    }

    len = strlen(startp);

    do {
        //filter space out
        while (startp && (isspace(*startp) || !isgraph(*startp)) && len) {
            startp++;
            len--;
        }

        if (len == 0) {
            break;
        }

        *out++ = strtol(startp, &endp, 0);

        len -= endp - startp;
        startp = endp;
        count++;

    } while ((endp) && (count < para_num) && (len > 0));

    return count;
}

static int set_display_axis(int recovery)
{
    int fd;
    //char *path = "/sys/class/display/axis";
    char str[128];
    int count;

    fd = open("/sys/class/display/axis", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        if (!recovery) {
            read(fd, str, 128);
            printf("read axis %s, length %zu\n", str, strlen(str));
            count = parse_para(str, 8, axis);
        }
        if (recovery) {
            sprintf(str, "%d %d %d %d %d %d %d %d",
                axis[0], axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        } else {
            sprintf(str, "2048 %d %d %d %d %d %d %d",
                axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        }
        write(fd, str, strlen(str));
        close(fd);
        return 0;
    }

    return -1;
}

static void signal_handler(int signum)
{
    printf("Get signum=%x\n", signum);
    set_display_axis(1);
    sysfs_cmd_control("/sys/class/graphics/fb0/blank", "0");
    sysfs_cmd_control("/sys/class/graphics/fb0/osd_display_debug", "0");
    sysfs_cmd_control("/sys/class/video/disable_video", "1");
    delete gEsplayer;
    signal(signum, SIG_DFL);
    raise(signum);
}

AmVideoDecBase* getAmVideoDec(AmVideoDecCallback* callback) {
    void *libHandle = dlopen("libmediahal_videodec.so", RTLD_NOW);

    if (libHandle == NULL) {
        libHandle = dlopen("libmediahal_videodec.system.so", RTLD_NOW);
        if (libHandle == NULL) {
            //ALOGE("unable to dlopen libmediahal_videodec.so: %s", dlerror());
            printf("unable to dlopen libmediahal_videodec.so: %s", dlerror());
            return nullptr;
        }
    }

    typedef AmVideoDecBase *(*createAmVideoDecFunc)(AmVideoDecCallback* callback);

    createAmVideoDecFunc getAmVideoDec =
        (createAmVideoDecFunc)dlsym(libHandle, "AmVideoDec_create");

    if (getAmVideoDec == NULL) {
        dlclose(libHandle);
        libHandle = NULL;
        //ALOGE("can not create AmVideoDec\n");
        printf("can not create AmVideoDec\n");
        return nullptr;
    }
    AmVideoDecBase* halHanle = (*getAmVideoDec)(callback);
    //ALOGI("getAmVideoDec ok\n");
    printf("getAmVideoDec ok\n");
    return halHanle;
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

vesplayer::vesplayer(init_param_t * config):mConfig(config),mEof(0),mFp(nullptr) {
    mCallback = new playerCallback(this);
    mAmVideoDec = getAmVideoDec(mCallback);
}

void vesplayer::play(const char* iname) {
    uint32_t Readlen = 0;
    //uint32_t isize = 0;
    char buffer[READ_SIZE];
    int end = 0;
    int32_t bitstreamId = 0;
    int64_t timestamp = 0u;
    int ret = 0;

    printf("\n*********CODEC PLAYER DEMO************\n\n");
    printf("file %s to be played\n", iname);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    if ((mFp = fopen(iname, "rb")) == NULL) {
        printf("open file error!\n");
        return;
    }

    ret = mAmVideoDec->initialize(vformat_to_mime(mConfig->vFmt), (uint8_t*)mConfig, sizeof(init_param_t), false, false);
    if (ret) {
        printf("init failed!, ret=%d\n", ret);
    }

    while (!ret) {
        if (!end) {
            Readlen = fread(buffer, 1, READ_SIZE, mFp);
            //printf("Readlen %d\n", Readlen);
            if (Readlen <= 0) {
                printf("read file error!\n");
                rewind(mFp);
            }
        } else
            Readlen = 0;

        if (end) {
            memset(buffer, 0 ,READ_SIZE);
            Readlen = READ_SIZE - 10;
        }

        if (mInputBuffer.size() > 50) {
            usleep(20000);
        }

        uint8_t * newbuf = (uint8_t *)malloc(READ_SIZE);
        memcpy(newbuf, buffer, Readlen);
        mInputBuffer[bitstreamId] = newbuf;
        //printf("queueInputBuffer bitstreamId %d, size %zd\n",
            //bitstreamId,  Readlen);

        int retry = 1000;
        int32_t err = 0;
        do {
            err = mAmVideoDec->queueInputBuffer(bitstreamId, mInputBuffer[bitstreamId], 0, Readlen, timestamp);
            if (err == -EAGAIN)
                usleep(5000);
            else
                break;
        } while(err || retry-- > 0);
        std::lock_guard<std::mutex> lock(mInputLock);
        //mInputWork[bitstreamId] = timestamp;
        bitstreamId++;

        if (feof(mFp))
            end = 1;
        if (end)
            break;
    }

    if (end) {
        std::unique_lock <std::mutex> lock(mEofLock);
        while (!mEof)
            mEofcv.wait(lock);
    }
    if (mFp)
        fclose(mFp);
    mAmVideoDec->destroy();
}

void vesplayer::onOutputFormatChanged(uint32_t requestedNumOfBuffers,
                int32_t width, uint32_t height) {
    printf("%s requestedNumOfBuffers:%d width:%d height:%d\n", __func__,
        requestedNumOfBuffers, width, height);
}

void vesplayer::onOutputBufferDone(int32_t pictureBufferId, int64_t bitstreamId,
                uint32_t width, uint32_t height) {
    printf("%s pictureBufferId:%d bitstreamId:%" PRId64 " width:%d height:%d\n", __func__,
        pictureBufferId, bitstreamId, width, height);
}
void vesplayer::onInputBufferDone(int32_t bitstream_buffer_id) {
    //printf("%s bitstream_buffer_id:%d\n", __func__,
        //bitstream_buffer_id);
    std::lock_guard<std::mutex> lock(mInputLock);
    mInputBuffer.erase(bitstream_buffer_id);
    free(mInputBuffer[bitstream_buffer_id]);
}

void vesplayer::onUpdateDecInfo(const uint8_t* info, uint32_t isize) {
    printf("%s info:%s isize:%d\n", __func__,
        info, isize);
}

void vesplayer::onFlushDone() {

}

void vesplayer::onResetDone() {

}

void vesplayer::onError(int32_t error) {
    printf("%s error%d", __func__, error);
}

void vesplayer::onEvent(uint32_t event, void* param, uint32_t paramsize) {
    UNUSED(param);
    UNUSED(paramsize);
    //printf("%s event:%x, %s %d", __func__, event, (char*)param, paramsize);
    if (10 == event) {
        std::unique_lock <std::mutex> lock(mEofLock);
        mEof = 1;
        mEofcv.notify_all();
    }
}

static void usage(void)
{
    printf("mediahal_esplayer\n");
    printf("Usage: mediahal_esplayer -i <file> -f <format> [-r <framerate>] [-a <addhead>] [-w <width>] [-h <height>] [-y <help>]\n");
    printf("\n");
  printf(" -i, --ifileinput es file\n");
  printf(" -f, --formatvideo format\n");
  printf ("(0:mpeg12\t1:mpeg4\t\t2:h264\t\t3:mjpeg\n\
            5:jpeg\t\t6:vcl\t\t7:avs\t\t11:hevc\n\
            14:vp9\t\t15:avs2)\n");
    printf(" -r, --framerate framerate\n");
    printf(" -a, --addhead add es header \n");
    printf(" -w, --width video width\n");
    printf(" -h, --height video height\n");
    printf(" -y, --help usage\n");
}

int main(int argc, char** argv) {
    int optionChar = 0;
    int optionIndex = 0;
    uint32_t videowidth = 1920;
    uint32_t videoheight = 1080;
    uint32_t framerate = 24;
    uint32_t   vFmt = -1;
    char* iname = nullptr;
    uint32_t addhead = 0;

    if (argc < 2) {
        usage();
        return -1;
    }

    const char *shortOptions = "i:f:r:a:w:h:y";
    struct option longOptions[] = {
        { "ifile", required_argument, nullptr, 'i' },
        { "format", required_argument, nullptr, 'f' },
        { "framerate", required_argument, nullptr, 'r' },
        { "addhead", required_argument, nullptr, 'a' },
        { "width", required_argument, nullptr, 'w' },
        { "height", required_argument, nullptr, 'h' },
        { "help", no_argument, nullptr,  'y'},
        { nullptr, 0, nullptr, 0 },
    };

    while ((optionChar = getopt_long(argc, argv, shortOptions,
            longOptions, &optionIndex)) != -1) {
        switch (optionChar) {
        case 'i':
            iname = optarg;
            break;
        case 'f':
            vFmt = atoi(optarg);
            break;
        case 'r':
            framerate = atoi(optarg);
            break;
        case 'a':
            addhead = atoi(optarg);
            break;
        case 'w':
            videowidth = atoi(optarg);
            break;
        case 'h':
            videoheight = atoi(optarg);
            break;
        case 'y':
            usage();
            exit(-1);
        default:
            break;
        }
    }

    if (iname == nullptr ||
        vFmt == (uint32_t)-1) {
        usage();
        exit(-1);
    }

    sysfs_cmd_control("/sys/class/graphics/fb0/osd_display_debug", "1");
    sysfs_cmd_control("/sys/class/graphics/fb0/blank", "1");
    sysfs_cmd_control("/sys/class/video/path_select", "0 -1");
    sysfs_cmd_control("sys/class/video/video_global_output", "1");
    sysfs_cmd_control("/sys/class/video/disable_video", "2");
    set_display_axis(0);

    init_param_t config = {
    /* video */     256, videowidth, videoheight, framerate, vFmt, 0,
    /* audio */     0, 2, 44100, 2,
    /*pcrid */		0,
    /* display */   1, 0, 0, 0,
                    2, 0, 0, 0, 0, 0, 0};


    gEsplayer = new vesplayer(&config);
    gEsplayer->play(iname);

    sysfs_cmd_control("/sys/class/graphics/fb0/blank", "0");
    sysfs_cmd_control("/sys/class/graphics/fb0/osd_display_debug", "0");
    sysfs_cmd_control("/sys/class/video/disable_video", "1");
    set_display_axis(1);

    delete gEsplayer;

    return 0;
}

