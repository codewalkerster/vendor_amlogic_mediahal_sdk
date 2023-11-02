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
#include <sys/utsname.h>
#include <string.h>
#include <dlfcn.h>

#ifdef SYSTEMLIB

#if (ANDROID_PLATFORM_SDK_VERSION >= 30) || (ANDROID_PLATFORM_SDK_VERSION == 28)
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
const int kRwTimeout = 500;

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
#if ANDROID_PLATFORM_SDK_VERSION >= 30 || defined(__linux__)
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define DUMMY_LIB_NAME "libdummy_wrap.so"
#define VMX_LIB_NAME_IPTV "libvmx_iptv_wrap.so"
#define VMX_LIB_NAME_DVB "libvmx_dvb_wrap.so"

typedef enum {
    CAS_UNKNOWN = -1,
    CAS_DUMMY = 0,
    CAS_VMX_IPTV = 1,
    CAS_VMX_DVB = 2,
    CAS_OTHER = 3,
    CAS_UNSUPPORT,
    CAS_MAX
} castype_t;
uint32_t casType = CAS_DUMMY;
#define TS_PACKET_SIZE 188
uint8_t CASECM[TS_PACKET_SIZE];
#define CAS_RETRY_NUM 10000
#define FIND_FIRST_ECM    (1)
uint32_t find_first_ecm = 0;

typedef struct iptvseverinfo {
     char * storepath;
     char * serveraddr;
     char * serverport;
     int    enablelog;
} iptvseverinfo_t;

typedef size_t am_casiptv_wrapper_handle;
typedef int32_t cas_create(am_casiptv_wrapper_handle *pHandle);
static cas_create* amcas_create = NULL;
typedef int cas_setprivatedata(am_casiptv_wrapper_handle pHandle ,void * date, int size);
static cas_setprivatedata* amcas_setprivatedata = NULL;
typedef int cas_setinstanceid(am_casiptv_wrapper_handle pHandle, unsigned int casid);
static cas_setinstanceid* amcas_setinstanceid = NULL;
typedef int cas_provision(am_casiptv_wrapper_handle pHandle);
static cas_provision* amcas_provision = NULL;
typedef int cas_setpids(am_casiptv_wrapper_handle pHandle, uint32_t vpid, uint32_t apid);
static cas_setpids* amcas_setpids = NULL;
typedef int cas_opensession(am_casiptv_wrapper_handle pHandle, uint8_t* sessionId);
static cas_opensession* amcas_opensession = NULL;
typedef int cas_processecm (am_casiptv_wrapper_handle pHandle, int isSection,int isvecm ,int vpid, int apid, unsigned char *pBuffer,int iBufferLength);
static cas_processecm* amcas_processecm = NULL;
typedef int cas_processemm (am_casiptv_wrapper_handle pHandle, int isSection,int pid,unsigned char *pBuffer,int iBufferLength);
static cas_processemm* amcas_processemm = NULL;
typedef int cas_selecttrack(am_casiptv_wrapper_handle pHandle, int trackType, int trackPid, int trackFormat);
static cas_selecttrack* amcas_selecttrack = NULL;
typedef int cas_closesession(am_casiptv_wrapper_handle pHandle, uint8_t* sessionId);
static cas_closesession* amcas_closesession = NULL;
am_casiptv_wrapper_handle casHandle = 0;

int casplugin_register (char * casTypeStr)
{
   int ret = -1 ;
   uint32_t type;
   char * p = casTypeStr;
   char * libPath = NULL;
   void * libHandle = NULL;

    if (p && strlen(p) < 4)
        return false;
    type = MKTAG(*p,*(p+1),*(p+2),*(p+3));
    switch (type) {
       case (MKTAG('d','u','m','m')):
           casType = CAS_DUMMY;
           libPath = strdup(DUMMY_LIB_NAME);
           break;
       case (MKTAG('v','m','x','i')):
           casType = CAS_VMX_IPTV;
           libPath = strdup(VMX_LIB_NAME_IPTV);
           break;
       case (MKTAG('v','m','x','d')):
           casType = CAS_VMX_DVB;
           libPath = strdup(VMX_LIB_NAME_DVB);
           break;
       case (MKTAG('o','t','h','r')):
           casType = CAS_OTHER;
           break;
       default:
           break;
   }

   if (libPath)
   {
       if (libHandle == NULL) {
           libHandle = dlopen(libPath, RTLD_LAZY);
           if (libHandle == NULL) {
               printf("unable to dlopen %s : %s",libPath, dlerror());
               return false;
           }
       }
       if (libHandle)
       {
           amcas_create = (cas_create*)dlsym(libHandle, "AmCasIPTVCreate");
           if (amcas_create == NULL)
               printf("unable to dlopen %s : %s",libPath, dlerror());
           amcas_setprivatedata = (cas_setprivatedata*)dlsym(libHandle, "AmCasIPTVSetPrivateData");
           if (amcas_setprivatedata == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_provision = (cas_provision*)dlsym(libHandle, "AmCasIPTVProvision");
           if (amcas_provision == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_setinstanceid = (cas_setinstanceid*)dlsym(libHandle, "AmCasIPTVSetInstanceId");
           if (amcas_setinstanceid == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_processecm = (cas_processecm*)dlsym(libHandle, "AmCasIPTVProcessEcm");
           if (amcas_processecm == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_processemm = (cas_processemm*)dlsym(libHandle, "AmCasIPTVProcessEmm");
           if (amcas_processemm == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_setpids = (cas_setpids*)dlsym(libHandle, "AmCasIPTVSetPids");
           if (amcas_setpids == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_opensession = (cas_opensession*)dlsym(libHandle, "AmCasIPTVOpenSession");
           if (amcas_opensession == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_selecttrack = (cas_selecttrack*)dlsym(libHandle, "AmCasIPTVSelectTrack");
           if (amcas_selecttrack == NULL)
               printf("dlsym fail %s", dlerror());
           amcas_closesession = (cas_closesession*)dlsym(libHandle, "AmCasIPTVCloseSession");
           if (amcas_closesession == NULL)
               printf("dlsym fail %s", dlerror());
       }
    }
   return ret;
}

int casplugin_provision()
{
    int ret = -1 ;
    iptvseverinfo_t sevinfo;

    if (amcas_create && amcas_setprivatedata && amcas_provision) {
        amcas_create(&casHandle);
        sevinfo.serveraddr = strdup("client-test-3.verimatrix.com");
        sevinfo.storepath = strdup("/data/mediadrm");
        sevinfo.serverport = strdup("12686");
        sevinfo.enablelog = 0;
        amcas_setprivatedata(casHandle, (void *)&sevinfo, sizeof(iptvseverinfo_t));
        ret = amcas_setinstanceid(casHandle, 0);
        ret = amcas_provision(casHandle);
    }
    return ret;
}

int casplugin_opensession(uint8_t* sessionId, int vpid, int apid)
{
    int ret = -1 ;

    if (amcas_setpids && amcas_opensession ) {
        amcas_setpids(casHandle,vpid,apid);
        ret = amcas_opensession(casHandle, sessionId);
    }
    return ret;
}

int casplugin_closesession(uint8_t* sessionId)
{
    int ret = -1 ;

    if (amcas_closesession)
        ret = amcas_closesession(casHandle, sessionId);
    return ret;
}

static am_tsplayer_result  check_ecm_inject(am_tsplayer_handle session, am_tsplayer_input_buffer *buf, int32_t timeout_ms , uint32_t vecm_pid, uint32_t aecm_pid)
{
    am_tsplayer_result ret;
    uint32_t pid = 0;
    int nSize = buf->buf_size;
    unsigned int rem = nSize;
    int retry_count = 0, send = 0;
    uint8_t * psync = (uint8_t *)buf->buf_data;
    uint8_t * rembufferpos = (uint8_t *)buf->buf_data;
    uint8_t * current = NULL;
    am_tsplayer_input_buffer ibuf = {TS_INPUT_BUFFER_TYPE_NORMAL, NULL, 0};

    while (rem >= TS_PACKET_SIZE)
    {
        if (*psync != 0x47)
        {
            ++psync;
            --rem;
            printf("check_ecm_inject ts not match rem%d\n",rem);
            if (rem <= TS_PACKET_SIZE)
                return AM_TSPLAYER_ERROR_RETRY;
            else
                continue;
        }
        if ((*(psync) == 0x47) && ((rem == TS_PACKET_SIZE) || (*(psync+TS_PACKET_SIZE) == 0x47)))
        {
            current = psync;
            pid = (( current[1] << 8 | current[2]) & 0x1FFF);
            if ((pid == vecm_pid || pid == aecm_pid ) && !find_first_ecm) {
                memcpy(CASECM, psync, TS_PACKET_SIZE);
                if (amcas_processecm && pid != 0x1fff)
                {
                    if (pid == vecm_pid)
                        amcas_processecm(casHandle,0,1,vecm_pid,aecm_pid, CASECM, TS_PACKET_SIZE);
                    else
                        amcas_processecm(casHandle,0,0,vecm_pid,aecm_pid, CASECM, TS_PACKET_SIZE);
                    find_first_ecm = 1;
                }
            }
            if ((pid == vecm_pid || pid == aecm_pid ) && (memcmp(CASECM + 4,psync + 4,TS_PACKET_SIZE- 4)))
            {
                send = psync - rembufferpos;
                if (send)
                {
                    ibuf.buf_data = rembufferpos;
                    ibuf.buf_size = send;
                    ret = AmTsPlayer_writeData(session,&ibuf,timeout_ms);
                    retry_count = 0;
                    while (ret == AM_TSPLAYER_ERROR_RETRY && retry_count < CAS_RETRY_NUM)
                    {
                        if (!enable_thread)
                        {
                            printf("%s, line %d quit\n",__FUNCTION__,__LINE__);
                            break;
                        }
                        retry_count++;
                        ibuf.buf_data = rembufferpos;
                        ibuf.buf_size = send;
                        if ((send % 188) != 0)
                        {
                        /*Remove PKCS7 padding at the end */
                            printf("%s, line %d process padding inject_len %d\n",__FUNCTION__,__LINE__,send);
                        }
                        ret = AmTsPlayer_writeData(session,&ibuf,timeout_ms);
                        if (ret == AM_TSPLAYER_ERROR_RETRY)
                        {
                            //DVBTRACE("[%d]%s at %d ret == AM_TSPLAYER_ERROR_RETRY retry_count %d\n",instanceID,__func__,__LINE__,retry_count);
                            usleep(50*1000);
                        }
                    }
                    if (ret)
                        return AM_TSPLAYER_OK;
                    if (send != (psync - rembufferpos))
                        printf("send %d is not match \n", send);
                    rembufferpos = psync;
                    send = 0;
                }
                if (memcmp(CASECM + 4, psync + 4, TS_PACKET_SIZE - 4))
                {
                    memcpy(CASECM, psync, TS_PACKET_SIZE);
                    if (amcas_processecm && pid != 0x1fff)
                    {
                        if (pid == vecm_pid)
                            amcas_processecm(casHandle,0,1,vecm_pid,aecm_pid, CASECM, TS_PACKET_SIZE);
                        else
                            amcas_processecm(casHandle,0,0,vecm_pid,aecm_pid, CASECM, TS_PACKET_SIZE);
                    }
               }
            }
        }
        psync += TS_PACKET_SIZE;
        rem -= TS_PACKET_SIZE;
    }
    if (send == 0)
    {
        send =  (uint8_t *)buf->buf_data + nSize - rembufferpos;
        //DVBTRACE("[%d]%s, line %d need send %d\n",instanceID, __FUNCTION__,__LINE__,send);
        if (send)
        {
            ibuf.buf_data = rembufferpos;
            ibuf.buf_size = send;
            ret = AmTsPlayer_writeData(session,&ibuf,timeout_ms);
        }
        retry_count = 0;
        while (ret == AM_TSPLAYER_ERROR_RETRY && retry_count < CAS_RETRY_NUM)
        {
            if (!enable_thread)
            {
                printf("%s, line %d quit\n",__FUNCTION__,__LINE__);
                break;
            }
            retry_count++;
            ibuf.buf_data = rembufferpos;
            if ((send % 188) != 0)
            {
            /*Remove PKCS7 padding at the end */
                printf("%s, line %d process padding inject_len %d\n", __FUNCTION__,__LINE__,send);
            }
            ibuf.buf_size = send;
            ret = AmTsPlayer_writeData(session,&ibuf,timeout_ms);

            if (ret == AM_TSPLAYER_ERROR_RETRY)
            {
                //DVBTRACE("[%d]%s at %d ret == AM_TSPLAYER_ERROR_RETRY retry_count %d\n",instanceID,__func__,__LINE__,retry_count);
                usleep(50*1000);
            }
       }
   }
#ifdef  VMXM9D4
   usleep(40*1000); /*we enable audio secure , aucpu has limit*/
#endif
   return ret;
}
#endif

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

#if (ANDROID_PLATFORM_SDK_VERSION >= 30) || (ANDROID_PLATFORM_SDK_VERSION == 28)

sp<IProducerListener> mProducerListener = NULL;
sp<IGraphicBufferProducer> mProducer = NULL;
sp<NativeHandle> mSourceHandle = NULL;
native_handle_t * mNative_handle = NULL;

bool CreateVideoTunnelId(int* id) {
    int x = 0, y = 0, w = 0, h = 0;
    int tunnelId = 0;
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
        #if (ANDROID_PLATFORM_SDK_VERSION > 30)
            mProducerListener = new StubProducerListener;
        #else
            mProducerListener = new DummyProducerListener;
        #endif
        char test[20];
        sprintf(test,"testSurface_%d",tunnelId);
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
                mNative_handle = am_gralloc_create_sideband_handle(AM_FIXED_TUNNEL, tunnelId);
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
            printf("----->tunnelId:%d\n",tunnelId);
            *id = tunnelId;
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
    printf("-i | --in              Ts file path\n");
    printf("-t | --tstype          demod:0, memory:1[default]\n");
    printf("-y | --avsync          amaster:0[default], vmaster:1, pcrmaster:2, nosync:3\n");
    printf("-c | --vtrick          none:0[default], pause:1, pause next:2, Ionly:3\n");
    printf("-v | --vcodec          unknown:0, mpeg1:1, mpeg2:2, h264:3[default], h265:4, vp9:5 avs:6 mpeg4:7, avs2:8, avs3:12\n");
    printf("-a | --acodec          unknown:0, mp2:1, mp3:2, ac3:3, eac3:4, dts:5, aac:6[default], latm:7, pcm:8\n");
    printf("-V | --vpid            video pid,default:0x100\n");
    printf("-A | --apid            audio pid,default:0x101\n");
    printf("-b | --buffertype      input buffer type ,default:0\n");
    printf("-e | --encryption type encryption type string, vmxiptv:vmxi, vmxdvb:vmxd\n");
    printf("-d | --vecmpid         video Ecm pid,default:0x1001\n");
    printf("-D | --aecmpid         audio Ecm pid,default:0x1001\n");
    printf("-p | --playback        disable:0[default], enable:1\n");
    printf("-h | --help            print this usage\n");
}

int GetPid(char* pid) {
    if (*pid == '0' && (*(pid + 1) == 'x' || *(pid + 1) == 'X')) {
        return strtol(pid,NULL,16);
    }
    return strtol(pid,NULL,10);
}

int keyboardHit() {
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
    const char *shortOptions = "i:t:b:y:c:v:a:V:A:p:e:d:D:h";
#if ANDROID_PLATFORM_SDK_VERSION >= 30 || defined(__linux__)
    char * amcasTypeStr = NULL;
    uint8_t sessionId [8]= {0};
    int ret = 0;
#endif
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
    int32_t vEcmPid = 0x1001;
    int32_t aEcmPid = 0x1001;

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
            case 'b':
                drmmode = static_cast<am_tsplayer_input_buffer_type>(atoi(optarg));
                printf("drmmode %d\n", drmmode);
                break;
            case 'e':
                if (optarg && strlen(optarg))
                    amcasTypeStr = strdup(optarg);
                break;
            case 'd':
                vEcmPid = GetPid(optarg);
                printf("vEcmPid 0x%x\n", vEcmPid);
                break;
            case 'D':
                aEcmPid = GetPid(optarg);
                printf("aEcmPid 0x%x\n", aEcmPid);
                break;
            case 'h':
                usage(argv);
                exit(-1);
            default:
                break;
        }
    }

#if ANDROID_PLATFORM_SDK_VERSION >= 30 || defined(__linux__)
    if (drmmode == TS_INPUT_BUFFER_TYPE_TVP && amcasTypeStr) {
        casplugin_register(amcasTypeStr);
        if (casplugin_provision( ))
            printf("cas provesion fail\n");
        ret = casplugin_opensession(sessionId, vPid, aPid);
        if (ret)
            printf("cas opensession fail\n");
        find_first_ecm = 0;
        memset(CASECM,0,TS_PACKET_SIZE);
    }
#endif
    signal(SIGINT, signHandler);
    enable_thread = true;

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

    struct utsname kernel_msg;
    bool isTsyncNonTunelflag = false;
    uname(&kernel_msg);
    if (strstr(kernel_msg.release, "5.15") != NULL) {
        printf("t5d nontunelmode need set VideoTunnelId\n");
        isTsyncNonTunelflag = true;
    }

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

    #if (ANDROID_PLATFORM_SDK_VERSION >= 30) || (ANDROID_PLATFORM_SDK_VERSION == 28)
    //android P android R
    #if (ANDROID_PLATFORM_SDK_VERSION == 28)
        printf("Android p system, platform demux:AmHwMultiDemux \n");
        printf("Need to set:\n");
        printf("setprop vendor.amtsplayer.pipeline 1\n");
        printf("setprop vendor.dtv.audio.skipamadec true\n");
    #endif

    if (access("/sys/class/stb/demux0_source",F_OK) != 0 ||
        (access("/sys/class/stb/demux0_source",F_OK) == 0 &&
        isTsyncNonTunelflag)) {
        //X4,Y4 need set VideoTunnelId
        printf("Android R system, platform demux:AmHwMultiDemux \n");
        printf("Set VideoTunnelId \n");
        static int VideoTunnelId = 0;
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
        printf("Run the test example in Android R vendor,\n the example does not have permission to create surface and convert videotunnel id.\n");
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
    uint32_t instanceNo;
    AmTsPlayer_getInstansNo(session, &instanceNo);
    AmTsPlayer_setWorkMode(session, TS_PLAYER_MODE_NORMAL);
    AmTsPlayer_registerCb(session, video_callback, NULL);
    AmTsPlayer_setSyncMode(session, avsyncMode);
#if ANDROID_PLATFORM_SDK_VERSION >= 30 || defined(__linux__)
    if (drmmode == TS_INPUT_BUFFER_TYPE_TVP && amcasTypeStr) {
        int32_t video_seclevel = AM_TSPLAYER_DMX_FILTER_SEC_LEVEL2;
        AmTsPlayer_setParams(session,AM_TSPLAYER_KEY_VIDEO_SECLEVEL,(void*)&video_seclevel);
        int32_t audio_seclevel = AM_TSPLAYER_DMX_FILTER_SEC_LEVEL2;
        AmTsPlayer_setParams(session,AM_TSPLAYER_KEY_AUDIO_SECLEVEL,(void*)&audio_seclevel);
    }
#endif

    am_tsplayer_video_params vparam;
    vparam.codectype = vCodec;
    vparam.pid = vPid;
    AmTsPlayer_setVideoParams(session, &vparam);
    AmTsPlayer_startVideoDecoding(session);

    am_tsplayer_audio_params aparam;
    aparam.codectype = aCodec;
    aparam.pid = aPid;
    AmTsPlayer_setAudioParams(session, &aparam);
    AmTsPlayer_startAudioDecoding(session);

    #if (ANDROID_PLATFORM_SDK_VERSION >= 30)
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
#if ANDROID_PLATFORM_SDK_VERSION >= 30 || defined(__linux__)
            if (drmmode == TS_INPUT_BUFFER_TYPE_TVP && amcasTypeStr)
                res = check_ecm_inject(session, &ibuf, kRwTimeout , vEcmPid, aEcmPid);
            else
#endif
                res = AmTsPlayer_writeData(session, &ibuf, kRwTimeout);
            if (res == AM_TSPLAYER_ERROR_RETRY) {
                usleep(50000);
            } else
                break;
        } while(res || retry-- > 0);
        if (keyboardHit()) {
            ch = getchar();
            printf("----key input : %d quit:q\n",ch);
            if (ch == 'q') {
                printf("----break\n");
                break;
            }
            if (ch == 'p') {
                printf("====>stop video \n");
                AmTsPlayer_stopVideoDecoding(session);
            }
            if (ch == 'r') {
                am_tsplayer_video_params vparam;
                vparam.codectype = vCodec;
                vparam.pid = vPid;
                printf("====>start video \n");
                AmTsPlayer_setVideoParams(session, &vparam);
                AmTsPlayer_startVideoDecoding(session);
            }
            if (ch == 'g') {
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

#if ANDROID_PLATFORM_SDK_VERSION >= 30 || defined(__linux__)
    if (drmmode == TS_INPUT_BUFFER_TYPE_TVP && amcasTypeStr) {
        ret = casplugin_closesession(sessionId);
        if (ret)
            printf("cas close session fail\n");
    }
#endif
    printf("exit\n");
    return 0;
}

