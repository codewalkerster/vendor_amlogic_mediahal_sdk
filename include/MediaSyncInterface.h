#ifndef MEDIA_SYNC_INTERFACE_H_
#define MEDIA_SYNC_INTERFACE_H_
#include <stdint.h>

typedef enum {
    MEDIA_SYNC_VMASTER = 0,
    MEDIA_SYNC_AMASTER = 1,
    MEDIA_SYNC_PCRMASTER = 2,
    MEDIA_SYNC_MODE_MAX = 255,
}sync_mode;

typedef enum {
    MEDIA_VIDEO = 0,
    MEDIA_AUDIO = 1,
    MEDIA_OTHER = 2,
    MEDIA_TYPE_MAX = 255,
}sync_stream_type;

typedef enum {
    MEDIA_SYNC_NOSYNC = 0,
    MEDIA_SYNC_PAUSED = 1,
    MEDIA_SYNC_DISCONTINUE = 2,
    MEDIA_SYNC_STATUS_MAX = 255,
}sync_status;


typedef enum {
    AM_MEDIASYNC_OK  = 0,                      // OK
    AM_MEDIASYNC_ERROR_INVALID_PARAMS = -1,    // Parameters invalid
    AM_MEDIASYNC_ERROR_INVALID_OPERATION = -2, // Operation invalid
    AM_MEDIASYNC_ERROR_INVALID_OBJECT = -3,    // Object invalid
    AM_MEDIASYNC_ERROR_RETRY = -4,             // Retry
    AM_MEDIASYNC_ERROR_BUSY = -5,              // Device busy
    AM_MEDIASYNC_ERROR_END_OF_STREAM = -6,     // End of stream
    AM_MEDIASYNC_ERROR_IO            = -7,     // Io error
    AM_MEDIASYNC_ERROR_WOULD_BLOCK   = -8,     // Blocking error
    AM_MEDIASYNC_ERROR_MAX = -254
} mediasync_result;

extern void* MediaSync_create();

extern mediasync_result MediaSync_allocInstance(void* handle, int32_t DemuxId,
                                                              int32_t PcrPid,
                                                              int32_t *SyncInsId);

extern mediasync_result MediaSync_bindInstance(void* handle, uint32_t SyncInsId,
                                                             sync_stream_type streamtype);
extern mediasync_result MediaSync_setSyncMode(void* handle, sync_mode mode);

extern mediasync_result MediaSync_getSyncMode(void* handle, sync_mode *mode);
extern mediasync_result MediaSync_setPause(void* handle, bool pause);
extern mediasync_result MediaSync_getPause(void* handle, bool *pause);
extern mediasync_result MediaSync_setStartingTimeMedia(void* handle, int64_t startingTimeMediaUs);
extern mediasync_result MediaSync_clearAnchor(void* handle);
extern mediasync_result MediaSync_updateAnchor(void* handle, int64_t anchorTimeMediaUs,
                                                                int64_t anchorTimeRealUs,
                                                                int64_t maxTimeMediaUs);
extern mediasync_result MediaSync_setPlaybackRate(void* handle, float rate);
extern mediasync_result MediaSync_getPlaybackRate(void* handle, float *rate);
extern mediasync_result MediaSync_getMediaTime(void* handle, int64_t realUs,
                                                                int64_t *outMediaUs,
                                                                bool allowPastMaxTime);
extern mediasync_result MediaSync_getRealTimeFor(void* handle, int64_t targetMediaUs, int64_t *outRealUs);
extern mediasync_result MediaSync_getRealTimeForNextVsync(void* handle, int64_t *outRealUs);
extern mediasync_result MediaSync_reset(void* handle);
extern void MediaSync_destroy(void* handle);


#endif  // MEDIA_CLOCK_H_
