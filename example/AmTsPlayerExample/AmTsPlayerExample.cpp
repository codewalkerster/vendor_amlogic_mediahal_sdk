/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */



#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include <cstdlib>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <sys/time.h>
#include <memory>
#include <getopt.h>
#include <chrono>
#include <AmTsPlayer.h>
#include <termios.h>
#include <pthread.h>

#ifdef SYSTEMLIB

#if (ANDROID_PLATFORM_SDK_VERSION == 30) || (ANDROID_PLATFORM_SDK_VERSION == 28)
#include <amlogic/am_gralloc_ext.h>
#endif

#include <gui/IProducerListener.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#if ANDROID_PLATFORM_SDK_VERSION <= 30
#include <ui/DisplayInfo.h>
#endif

using namespace android;
#endif
using namespace std;

/*TS Playback Switch*/
typedef enum {
    TS_PLAYBACK_DISABLE = 0,    // Not playback when file eof
    TS_PLAYBACK_ENABLE = 1,     // Playback when file eof
} am_tsplayer_playback_type;

am_tsplayer_handle session;
const int kRwSize = 188*300;
const int kRwTimeout = 30000;

#define DEBUG_FLAG 1
#define TEST_FLOW 0
bool enable_thread;

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

/*
ANDROID_PLATFORM_SDK_VERSION = 30 --> Android R
ANDROID_PLATFORM_SDK_VERSION = 29 --> Android Q
ANDROID_PLATFORM_SDK_VERSION = 28 --> Android P
*/

#ifdef SYSTEMLIB

//system lib

android::sp<SurfaceComposerClient> mComposerClient = NULL;
android::sp<SurfaceControl> mControl = NULL;
android::sp<Surface> mSurface = NULL;

bool CreateSurface(void) {
    int x = 0;
    int y = 0;
    int w = 1920 / 2;
    int h = 1080 / 2;
    mComposerClient = new SurfaceComposerClient;
    if (mComposerClient->initCheck() != 0) {
        return false;
    }
    mControl = mComposerClient->createSurface(
           String8("testSurface"),
            w,
            h,
            HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);
    if (mControl == NULL) {
        return false;
    }
    if (!mControl->isValid()) {
        return false;
    }
    SurfaceComposerClient::Transaction{}
            .setLayer(mControl, 0)
            .show(mControl)
            .setPosition(mControl, x, y)
            .apply();
    mSurface = mControl->getSurface();
    if (mSurface == NULL) {
        return false;
    }
    return true;
}

#if (ANDROID_PLATFORM_SDK_VERSION == 30) || (ANDROID_PLATFORM_SDK_VERSION == 28)

sp<IProducerListener> mProducerListener = NULL;
sp<IGraphicBufferProducer> mProducer = NULL;
sp<NativeHandle> mSourceHandle = NULL;
native_handle_t * mNative_handle = NULL;

bool CreateVideoTunnelId(int* id) {
    int x = 0, y = 0, w = 0, h = 0;
    int tunnelid = 0;
    x = 0;
    y = 0;
    w = 960;
    h = 540;

    if (mSurface == NULL) {
        mComposerClient = new SurfaceComposerClient;
        if (mComposerClient->initCheck() != 0) {
            //printf("mSurface == NULL in line 79");
            return false;
        }
        mProducerListener = new DummyProducerListener;
        char test[20];
        sprintf(test,"testSurface_%d",tunnelid);
        printf("CreateVideoTunnelId name:%s \n",test);
        mControl = mComposerClient->createSurface(String8(test),
                                                  w,
                                                  h,
                                                  HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED);
        if (mControl == NULL) {
            printf("mControl == NULL");
            return false;
        }
        if (!mControl->isValid()) {
            printf("! mControl->isValid  no ");
            return false;
        }
        SurfaceComposerClient::Transaction{}
        .setLayer(mControl, 0)
        .setFlags(mControl, android::layer_state_t::eLayerOpaque, android::layer_state_t::eLayerOpaque)
        .show(mControl)
        .setPosition(mControl, x, y)
        .apply();
        mSurface = mControl->getSurface();
        if (mSurface == NULL) {
            printf("mSurface == NULL");
            return false;
        }

        mSurface->connect(NATIVE_WINDOW_API_CPU, mProducerListener);

        if (mSurface) {
            mProducer = mSurface->getIGraphicBufferProducer();
            if (mNative_handle == NULL) {
                mNative_handle = am_gralloc_create_sideband_handle(AM_FIXED_TUNNEL, tunnelid);
                // printf("mNative_handle:%p\n",mNative_handle);
            }
            if (mNative_handle != NULL) {
                mSourceHandle = NativeHandle::create(mNative_handle, false);
            //  printf("mSourceHandle:%p\n",mSourceHandle);
            }
            if (mProducer != NULL && mSourceHandle != NULL) {
                mProducer->setSidebandStream(mSourceHandle);
                //printf("line:%d\n",__LINE__);
            }
            printf("----->tunnelid:%d\n",tunnelid);
            *id = tunnelid;
        }
    }
    return true;
}

#endif

#endif

void video_callback(void *user_data, am_tsplayer_event *event)
{
    UNUSED(user_data);
    printf("video_callback type %d\n", event? event->type : 0);
    switch (event->type) {
        case AM_TSPLAYER_EVENT_TYPE_VIDEO_CHANGED:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_CHANGED: %d x %d @%d [%d]\n",
                event->event.video_format.frame_width,
                event->event.video_format.frame_height,
                event->event.video_format.frame_rate,
                event->event.video_format.frame_aspectratio);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_AUDIO_CHANGED:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_CHANGED: ch=%u ch_mask=%u samplerate=%u\n",
                event->event.audio_format.channels ,
                event->event.audio_format.channel_mask,
                event->event.audio_format.sample_rate);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_USERDATA_AFD:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_USERDATA_AFD\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_USERDATA_CC:
        {
            uint8_t* pbuf = event->event.mpeg_user_data.data;
            uint32_t size = event->event.mpeg_user_data.len;
            printf("[evt] USERDATA [%d] : %x-%x-%x-%x %x-%x-%x-%x ,size %d\n",
                event->type, pbuf[0], pbuf[1], pbuf[2], pbuf[3],
                pbuf[4], pbuf[5], pbuf[6], pbuf[7], size);
            UNUSED(pbuf);
            UNUSED(size);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_FIRST_FRAME:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_FIRST_FRAME\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_VIDEO:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_VIDEO\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_AUDIO:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_AUDIO\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_AV_SYNC_DONE:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_AV_SYNC_DONE\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_INPUT_VIDEO_BUFFER_DONE:
        {
        //    printf("[evt] AM_TSPLAYER_EVENT_TYPE_INPUT_VIDEO_BUFFER_DONE,%p\n",event->event.ptr);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_VIDEO_OVERFLOW:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_OVERFLOW video_overflow_num %u\n",
                event->event.av_flow_cnt.video_overflow_num);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_VIDEO_UNDERFLOW:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_UNDERFLOW video_underflow_num %u\n",
                event->event.av_flow_cnt.video_underflow_num);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_AUDIO_OVERFLOW:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_OVERFLOW audio_overflow_num %u\n",
                event->event.av_flow_cnt.audio_overflow_num);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_AUDIO_UNDERFLOW:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_UNDERFLOW audio_underflow_num %u\n",
                event->event.av_flow_cnt.audio_underflow_num);
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_TIMESTAMP:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_TIMESTAMP\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_DATA:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_VIDEO_INVALID_DATA\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_TIMESTAMP:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_TIMESTAMP\n");
            break;
        }
        case AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_DATA:
        {
            printf("[evt] AM_TSPLAYER_EVENT_TYPE_AUDIO_INVALID_DATA\n");
            break;
       }
        default:
            break;
    }
}

static int set_osd_blank(int blank)
{
    const char *path1 = "/sys/class/graphics/fb0/blank";
#if (ANDROID_PLATFORM_SDK_VERSION < 30)
    const char *path2 = "/sys/class/graphics/fb0/osd_display_debug";
#endif

#if (ANDROID_PLATFORM_SDK_VERSION >= 30)
    const char *path2 = "/sys/kernel/debug/dri/0/vpu/blank";
#endif

    const char *path3 = " /sys/class/video/disable_video";
    const char *path4 = "/sys/class/video/video_global_output";
    int fd;
    char cmd[128] = {0};

    fd = open(path2,O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0)
    {
        sprintf(cmd,"%d",blank);
        write (fd,cmd,strlen(cmd));
        close(fd);
    }
    fd = open(path1,O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0)
    {
        sprintf(cmd,"%d",blank);
        write (fd,cmd,strlen(cmd));
        close(fd);
    }

    fd = open(path3,O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0)
    {
        if (blank == 1) {
            sprintf(cmd,"2");
        } else {
            sprintf(cmd,"1");
        }
        write (fd,cmd,strlen(cmd));
        close(fd);
    }

    fd = open(path4,O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0)
    {
        sprintf(cmd,"%d",blank);
        write (fd,cmd,strlen(cmd));
        close(fd);
    }
    return 0;
}

static int amsysfs_set_sysfs_str(const char *path, const char *val) {
    int fd;
    int bytes;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        bytes = write(fd, val, strlen(val));
        close(fd);
        return 0;
    }
    return -1;
}

static int set_dmx_source()
{
    amsysfs_set_sysfs_str("/sys/class/stb/source", "dmx0");
    amsysfs_set_sysfs_str("/sys/class/stb/demux0_source", "hiu");
    return 0;
}

void signHandler(int iSignNo)
{
    UNUSED(iSignNo);
    enable_thread = false;
    AmTsPlayer_stopVideoDecoding(session);
    AmTsPlayer_stopAudioDecoding(session);
    AmTsPlayer_release(session);
    //Turn on the osd layer.
    set_osd_blank(0);
    printf("signHandler:%d\n",iSignNo);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

static void usage(char **argv)
{
    printf("Usage: %s\n", argv[0]);
    printf("Version 0.1\n");
    printf("[options]:\n");
    printf("-i | --in           Ts file path\n");
    printf("-t | --tstype       demod:0, memory:1[default]\n");
    printf("-y | --avsync       amaster:0[default], vmaster:1, pcrmaster:2, nosync:3\n");
    printf("-c | --vtrick       none:0[default], pause:1, pause next:2, Ionly:3\n");
    printf("-v | --vcodec       unknown:0, mpeg1:1, mpeg2:2, h264:3[default], h265:4, vp9:5 avs:6 mpeg4:7\n");
    printf("-a | --acodec       unknown:0, mp2:1, mp3:2, ac3:3, eac3:4, dts:5, aac:6[default], latm:7, pcm:8\n");
    printf("-V | --vpid         video pid,default:0x100\n");
    printf("-A | --apid         audio pid,default:0x101\n");
    printf("-p | --playback     disable:0[default], enable:1\n");
    printf("-h | --help         print this usage\n");
}

int GetPid(char* pid) {
    if (*pid == '0' && (*(pid + 1) == 'x' || *(pid + 1) == 'X')) {
        return strtol(pid,NULL,16);
    }
    return strtol(pid,NULL,10);
}

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (!initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

int main(int argc, char **argv)
{
    int optionChar = 0;
    int optionIndex = 0;
    const char *shortOptions = "i:t:b:y:c:v:a:V:A:p:h";
    struct option longOptions[] = {
        { "in",             required_argument,  NULL, 'i' },
        { "tstype",         required_argument,  NULL, 't' },
        { "buftype",        required_argument,  NULL, 'b' },
        { "avsync",         required_argument,  NULL, 'y' },
        { "videotrick",     required_argument,  NULL, 'c' },
        { "vcodec",         required_argument,  NULL, 'v' },
        { "acodec",         required_argument,  NULL, 'a' },
        { "vpid",           required_argument,  NULL, 'V' },
        { "apid",           required_argument,  NULL, 'A' },
        { "playback",       required_argument,  NULL, 'p' },
        { "help",           no_argument,        NULL, 'h' },
        { NULL,             0,                  NULL,  0  },
    };

    std::string inputTsName("/data/1.ts");
    am_tsplayer_input_source_type tsType = TS_MEMORY;
    am_tsplayer_input_buffer_type drmmode = TS_INPUT_BUFFER_TYPE_NORMAL;
    am_tsplayer_avsync_mode avsyncMode= TS_SYNC_AMASTER;
    am_tsplayer_video_trick_mode vTrickMode = AV_VIDEO_TRICK_MODE_NONE;
    am_tsplayer_video_codec vCodec = AV_VIDEO_CODEC_H264;
    am_tsplayer_audio_codec aCodec = AV_AUDIO_CODEC_AAC;
    am_tsplayer_playback_type emPlaybackType = TS_PLAYBACK_DISABLE;
    int32_t vPid = 0x100;
    int32_t aPid = 0x101;

    while ((optionChar = getopt_long(argc, argv, shortOptions,
                                    longOptions, &optionIndex)) != -1) {
        switch (optionChar) {
            case 'i':
                inputTsName.assign((const char*)optarg);
                break;
            case 't':
                tsType = static_cast<am_tsplayer_input_source_type>(atoi(optarg));
                break;
            case 'y':
                avsyncMode = static_cast<am_tsplayer_avsync_mode>(atoi(optarg));
                break;
            case 'c':
                vTrickMode = static_cast<am_tsplayer_video_trick_mode>(atoi(optarg));
                break;
            case 'v':
                vCodec = static_cast<am_tsplayer_video_codec>(atoi(optarg));
                break;
            case 'a':
                aCodec = static_cast<am_tsplayer_audio_codec>(atoi(optarg));
                break;
            case 'V':
                vPid = GetPid(optarg); //char* --> int
                break;
            case 'A':
                aPid = GetPid(optarg); //char* --> int
                break;
            case 'p':
                emPlaybackType = static_cast<am_tsplayer_playback_type>(atoi(optarg));
                break;
            case 'h':
                usage(argv);
                exit(-1);
            default:
                break;
        }
    }

    signal(SIGINT, signHandler);

    //Turn off the osd layer.
    set_osd_blank(1);

    char* buf = new char[kRwSize];
    uint64_t fsize = 0;
    ifstream file(inputTsName.c_str(), ifstream::binary);
    if (tsType) {
       set_dmx_source();
       file.seekg(0, file.end);
       fsize = file.tellg();
       if (fsize <= 0) {
           printf("file %s size %lld return\n", inputTsName.c_str(),(long long)fsize);
           return 0;
       }
       file.seekg(0, file.beg);
    }
    printf("file name = %s, is_open %d, size %lld, tsType %d\n",
                inputTsName.c_str(), file.is_open(),(long long) fsize, tsType);

    //am_tsplayer_handle session;
    am_tsplayer_init_params parm = {tsType, drmmode, 0, 0};
    AmTsPlayer_create(parm, &session);
    #ifdef SYSTEMLIB
    //system lib
    #if (ANDROID_PLATFORM_SDK_VERSION == 29)
    //Android Q TsPlayer NonTunnelMode
    //need set setprop vendor.amtsplayer.pipeline 1
    printf("\n");
    printf("\n");
    printf("Android Q System TsPlayer NonTunnelMode\n");
    printf("need to set:\n");
    printf("setprop vendor.amtsplayer.pipeline 1 \n");
    printf("\n");
    printf("\n");
    if (CreateSurface() == true) {
        AmTsPlayer_setSurface(session,mSurface.get());
    }
    #endif

    #if (ANDROID_PLATFORM_SDK_VERSION == 30) || (ANDROID_PLATFORM_SDK_VERSION == 28)
    //android P android R
    #if (ANDROID_PLATFORM_SDK_VERSION == 28)
        printf("Android p system, platform demux:AmHwMultiDemux \n");
        printf("Need to set:\n");
        printf("setprop vendor.amtsplayer.pipeline 1\n");
        printf("setprop vendor.dtv.audio.skipamadec true\n");
    #endif

    if (access("/sys/module/dvb_demux/",F_OK) == 0) {
        //X4,Y4 need set VideoTunnelId
        printf("Android R system, platform demux:AmHwMultiDemux \n");
        printf("Set VideoTunnelId \n");
        int VideoTunnelId = 0;
        if (CreateVideoTunnelId(&VideoTunnelId) == true) {
            AmTsPlayer_setSurface(session,(void*)&VideoTunnelId);
        } else {
            printf("CreateVideoTunnelId error \n");
            return 0;
        }
    }
    #endif
    #endif

    #ifndef SYSTEMLIB
    #if (ANDROID_PLATFORM_SDK_VERSION == 30)
    if (access("/sys/module/dvb_demux/",F_OK) == 0) {
        //X4,Y4 need set VideoTunnelId
        printf("\n");
        printf("\n");
        printf("Android R vendor,platform X4(SC2) Y4 \n");
        printf("Run the test example in Android R vendor,\nthe example does not have permission to create surface and convert videotunnel id.\n");
        printf("If you need to test with AmTsPlayerExample,\n you can force the setting to use the tsync module for audio and video synchronization.\n");
        printf("Need to set:\n");
        printf("setprop vendor.amtsplayer.pipeline 0\n");
        printf("setprop vendor.dtv.audio.skipamadec false\n");
        printf("\n");
        printf("\n");
    }
    #endif
    #endif
    uint32_t versionM, versionL;
    AmTsPlayer_getVersion(&versionM, &versionL);
    uint32_t instanceno;
    AmTsPlayer_getInstansNo(session, &instanceno);
    AmTsPlayer_setWorkMode(session, TS_PLAYER_MODE_NORMAL);
    AmTsPlayer_registerCb(session, video_callback, NULL);
    AmTsPlayer_setSyncMode(session, avsyncMode);

    am_tsplayer_video_params vparm;
    vparm.codectype = vCodec;
    vparm.pid = vPid;
    AmTsPlayer_setVideoParams(session, &vparm);
    AmTsPlayer_startVideoDecoding(session);

    am_tsplayer_audio_params aparm;
    aparm.codectype = aCodec;
    aparm.pid = aPid;
    AmTsPlayer_setAudioParams(session, &aparm);
    AmTsPlayer_startAudioDecoding(session);

    #if (ANDROID_PLATFORM_SDK_VERSION == 30)
    //
    am_tsplayer_audio_patch_manage_mode FORCE_ENABLE = AUDIO_PATCH_MANAGE_FORCE_ENABLE;
    AmTsPlayer_setParams(session,AM_TSPLAYER_KEY_SET_AUDIO_PATCH_MANAGE_MODE,(void*)&FORCE_ENABLE);
    #endif

    AmTsPlayer_showVideo(session);
    AmTsPlayer_setTrickMode(session, vTrickMode);

    am_tsplayer_input_buffer ibuf = {TS_INPUT_BUFFER_TYPE_NORMAL, (char*)buf, 0};
    long pos = 0;
    int ch = 0;
    while (tsType)
    {
        if (file.eof()) {
            if (emPlaybackType == TS_PLAYBACK_ENABLE) {
                printf("file read eof will playback soon \n");
                file.clear();
                file.seekg(0, file.beg);
            } else {
                printf("file read eof will stop playing soon \n");
                break;
            }
        }
        file.read(buf, (int)kRwSize);
        ibuf.buf_size = kRwSize;
        pos += kRwSize;

        int retry = 100;
        am_tsplayer_result res;
        do {
            if (pos > kRwSize * 10 && TEST_FLOW) {
                printf ("pos =%ld sleep 10s for test underflow event\n",pos);
                usleep(10000000);
                pos = 0;
            }
            res = AmTsPlayer_writeData(session, &ibuf, kRwTimeout);
            if (res == AM_TSPLAYER_ERROR_RETRY) {
                usleep(50000);
            } else
                break;
        } while(res || retry-- > 0);
        if (_kbhit()) {
            ch = getchar();
            printf("----key input : %d quit:q\n",ch);
            if (ch == 113) {
                printf("----break\n");
                break;
            }
            if (ch == 10) {
                am_tsplayer_state_t state;
                state.data_len = 1024;
                state.av_flag = AM_TSPLAYER_AV_INFO;
                state.data = (uint8_t *)malloc(state.data_len);
                memset(state.data, 0x0, state.data_len);
                AmTsPlayer_getState(session, &state);
                printf("----json[%d]:\n%s\n", (int)state.actual_len, state.data);
                free(state.data);
            }
        }
    }
    while (tsType == TS_DEMOD) {
        usleep(1000000);
    }

    if (ch != 113)
        std::this_thread::sleep_for(std::chrono::seconds(10));


    delete [](buf);

    if (file.is_open())
        file.close();

    #ifdef SYSTEMLIB
    if (mSurface && mComposerClient && mControl) {
        mSurface.clear();
        mSurface = nullptr;
        mControl.clear();
        mControl = nullptr;
        mComposerClient.clear();
        mComposerClient = nullptr;
    }
    #endif

    //Turn on the osd layer.
    set_osd_blank(0);

    AmTsPlayer_stopVideoDecoding(session);

    AmTsPlayer_stopAudioDecoding(session);

    AmTsPlayer_release(session);
    printf("exit\n");
    return 0;
}

