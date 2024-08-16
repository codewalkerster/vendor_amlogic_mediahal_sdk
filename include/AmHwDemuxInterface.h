/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef AMHWDEMUX_INTERFACE_H
#define AMHWDEMUX_INTERFACE_H

/*Player working mode*/
typedef enum {
   STREAM_CONTROL = 0,
   GET_ES_MODE = 1,
} hw_demux_work_mode;


/*Function return type*/
typedef enum {
    AM_DEMUX_OK  = 0,                      // OK
    AM_DEMUX_ERROR_INVALID_PARAMS = -1,    // Parameters invalid
    AM_DEMUX_ERROR_INVALID_OPERATION = -2, // Operation invalid
    AM_DEMUX_ERROR_INVALID_OBJECT = -3,    // Object invalid
    AM_DEMUX_ERROR_RETRY = -4,             // Retry
    AM_DEMUX_ERROR_BUSY = -5,              // Device busy
    AM_DEMUX_ERROR_END_OF_STREAM = -6,     // End of stream
    AM_TDEMUX_ERROR_IO            = -7,     // Io error
    AM_DEMUX_ERROR_WOULD_BLOCK   = -8,     // Blocking error
    AM_DEMUX_ERROR_MAX = -254
} am_demux_result;



struct AmDemuxControlInfo{
    int demuxId;
    int mediasyncId;
    //int aPid;
    //int vPid;
};

typedef struct {
    int videoPid;
    int numAudioPids;
    int audioPids[4];
} StreamPidInfo;

typedef struct {
    void* arg;
    uint64_t writeTsSize;
    StreamPidInfo *pidInfo;
    size_t pidInfoSize;
    bool bypassStreamControl;
    int timeout;
} StreamControlArgs;

extern void* AmHwDemux_Create(int mode,void* arg);
extern void AmHwDemux_Destroy(void* handle);
extern int AmHwDemux_Init(void* handle,int mode,void* arg);
extern int AmHwDemux_Flush(void *handle);
extern int AmHwDemux_ResetStatus(void* handle);
extern int AmHwDemux_SetParams(void* handle,int type, void* arg);
extern int AmHwDemux_GetParams(void* handle,int type, void* arg);
extern am_demux_result AmHwDemux_GetStreamControlStatus(void* handle,void* arg,uint64_t WriteTsSize,int vPid,int aPid);
extern am_demux_result AmHwDemux_GetMultiStreamControlStatus(void* handle, const StreamControlArgs& args);

#endif

