/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */



#ifndef AMTSPLAYER_H
#define AMTSPLAYER_H


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef uint8_t         bool_t;
/*Call back event type*/
typedef enum {
    AM_TSPLAYER_EVENT_TYPE_PTS = 0,        // pts in for some stream
    AM_TSPLAYER_EVENT_TYPE_DTV_SUBTITLE,   // external subtitle of dtv
    AM_TSPLAYER_EVENT_TYPE_USERDATA_AFD,   // user data (afd)
    AM_TSPLAYER_EVENT_TYPE_USERDATA_CC,    // user data (cc)
    AM_TSPLAYER_EVENT_TYPE_VIDEO_CHANGED,  // video format changed
    AM_TSPLAYER_EVENT_TYPE_AUDIO_CHANGED,  // audio format changed
    AM_TSPLAYER_EVENT_TYPE_DATA_LOSS,      // demod data loss
    AM_TSPLAYER_EVENT_TYPE_DATA_RESUME,    // demod data resume
    AM_TSPLAYER_EVENT_TYPE_SCRAMBLING,     // scrambling status changed
    AM_TSPLAYER_EVENT_TYPE_FIRST_FRAME,     // first video frame showed
    AM_TSPLAYER_EVENT_TYPE_STREAM_MODE_EOF, //endof stream mode
    AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_VIDEO, //The video decoder outputs the first frame.
    AM_TSPLAYER_EVENT_TYPE_DECODE_FIRST_FRAME_AUDIO, //The audio decoder outputs the first frame.
    AM_TSPLAYER_EVENT_TYPE_AV_SYNC_DONE,     //av sync done
    AM_TSPLAYER_EVENT_TYPE_INPUT_VIDEO_BUFFER_DONE,  // input video buffer done
    AM_TSPLAYER_EVENT_TYPE_INPUT_AUDIO_BUFFER_DONE  // input audio buffer done
} am_tsplayer_event_type;

/*Call back event mask*/
#define AM_TSPLAYER_EVENT_TYPE_PTS_MASK            (1 << AM_TSPLAYER_EVENT_TYPE_PTS)
#define AM_TSPLAYER_EVENT_TYPE_DTV_SUBTITLE_MASK   (1 << AM_TSPLAYER_EVENT_TYPE_DTV_SUBTITLE)
#define AM_TSPLAYER_EVENT_TYPE_USERDATA_AFD_MASK   (1 << AM_TSPLAYER_EVENT_TYPE_USERDATA_AFD)
#define AM_TSPLAYER_EVENT_TYPE_USERDATA_CC_MASK    (1 << AM_TSPLAYER_EVENT_TYPE_USERDATA_CC)
#define AM_TSPLAYER_EVENT_TYPE_VIDEO_CHANGED_MASK  (1 << AM_TSPLAYER_EVENT_TYPE_VIDEO_CHANGED)
#define AM_TSPLAYER_EVENT_TYPE_AUDIO_CHANGED_MASK  (1 << AM_TSPLAYER_EVENT_TYPE_AUDIO_CHANGED)
#define AM_TSPLAYER_EVENT_TYPE_DATA_LOSS_MASK      (1 << AM_TSPLAYER_EVENT_TYPE_DATA_LOSS)
#define AM_TSPLAYER_EVENT_TYPE_DATA_RESUME_MASK    (1 << AM_TSPLAYER_EVENT_TYPE_DATA_RESUME)
#define AM_TSPLAYER_EVENT_TYPE_SCRAMBLING_MASK     (1 << AM_TSPLAYER_EVENT_TYPE_SCRAMBLING)
#define AM_TSPLAYER_EVENT_TYPE_FIRST_FRAME_MASK    (1 << AM_TSPLAYER_EVENT_TYPE_FIRST_FRAME)

/*Function return type*/
typedef enum {
    AM_TSPLAYER_OK  = 0,                      // OK
    AM_TSPLAYER_ERROR_INVALID_PARAMS = -1,    // Parameters invalid
    AM_TSPLAYER_ERROR_INVALID_OPERATION = -2, // Operation invalid
    AM_TSPLAYER_ERROR_INVALID_OBJECT = -3,    // Object invalid
    AM_TSPLAYER_ERROR_RETRY = -4,             // Retry
    AM_TSPLAYER_ERROR_BUSY = -5,              // Device busy
    AM_TSPLAYER_ERROR_END_OF_STREAM = -6,     // End of stream
    AM_TSPLAYER_ERROR_IO            = -7,     // Io error
    AM_TSPLAYER_ERROR_WOULD_BLOCK   = -8,     // Blocking error
    AM_TSPLAYER_ERROR_MAX = -254
} am_tsplayer_result;

/*Data input source type*/
typedef enum
{
    TS_DEMOD = 0,                          // TS Data input from demod
    TS_MEMORY = 1,                         // TS Data input from memory
    ES_MEMORY = 2,                         // ES Data input from memory
} am_tsplayer_input_source_type;

/*Input buffer type*/
typedef enum {
    TS_INPUT_BUFFER_TYPE_NORMAL = 0,       // Input buffer is normal buffer
    TS_INPUT_BUFFER_TYPE_SECURE = 1,       // Input buffer is secure buffer
    TS_INPUT_BUFFER_TYPE_TVP = 2           // Input buffer is normal but tvp enable
} am_tsplayer_input_buffer_type;

/*Ts stream type*/
typedef enum {
    TS_STREAM_VIDEO = 0,                   // Video
    TS_STREAM_AUDIO = 1,                   // Audio
    TS_STREAM_AD = 2,                      // Audio description
    TS_STREAM_SUB = 3,                     // Subtitle
} am_tsplayer_stream_type;

/*Avsync mode*/
typedef enum {
    TS_SYNC_VMASTER = 0,                   // Video Master
    TS_SYNC_AMASTER = 1,                   // Audio Master
    TS_SYNC_PCRMASTER = 2,                 // PCR Master
    TS_SYNC_NOSYNC = 3                     // Free run
} am_tsplayer_avsync_mode;

/*Player working mode*/
typedef enum {
    TS_PLAYER_MODE_NORMAL = 0,             // Normal mode
    TS_PLAYER_MODE_CACHING_ONLY = 1,       // Only caching data, do not decode. Used in FCC
    TS_PLAYER_MODE_DECODE_ONLY = 2         // Decode data but do not output
} am_tsplayer_work_mode;

/*Audio stereo output mode*/
typedef enum {
    AV_AUDIO_STEREO = 0,                   // Stereo mode
    AV_AUDIO_LEFT = 1,                     // Output left channel
    AV_AUDIO_RIGHT = 2,                    // Output right channel
    AV_AUDIO_SWAP = 3,                     // Swap left and right channel
    AV_AUDIO_LRMIX = 4                     // mix left and right channel
} am_tsplayer_audio_stereo_mode;

/*Audio Output mode*/
typedef enum {
    AV_AUDIO_OUT_PCM = 0,                  // PCM out
    AV_AUDIO_OUT_PASSTHROUGH = 1,          // Passthrough out
    AV_AUDIO_OUT_AUTO = 2,                 // Auto
} am_tsplayer_audio_out_mode;

/*Video decoder trick mode*/
typedef enum {
    AV_VIDEO_TRICK_MODE_NONE = 0,          // Disable trick mode
    AV_VIDEO_TRICK_MODE_PAUSE = 1,         // Pause the video decoder
    AV_VIDEO_TRICK_MODE_PAUSE_NEXT = 2,    // Pause the video decoder when a new frame dispalyed
    AV_VIDEO_TRICK_MODE_IONLY = 3          // Decoding and Out I frame only
} am_tsplayer_video_trick_mode;

/*Video display match mode*/
typedef enum {
    AV_VIDEO_MATCH_MODE_NONE = 0,          // Keep original
    AV_VIDEO_MATCH_MODE_FULLSCREEN = 1,    // Strech the video to the full window
    AV_VIDEO_MATCH_MODE_LETTER_BOX = 2,    // Letter box match mode
    AV_VIDEO_MATCH_MODE_PAN_SCAN = 3,      // Pan scan match mode
    AV_VIDEO_MATCH_MODE_COMBINED = 4,      // Combined pan scan and letter box
    AV_VIDEO_MATCH_MODE_WIDTHFULL = 5,     // Strech the video width to the full window
    AV_VIDEO_MATCH_MODE_HEIGHFULL = 6      // Strech the video height to the full window
} am_tsplayer_video_match_mode;

/*Video decoder type*/
typedef enum {
    AV_VIDEO_CODEC_AUTO = 0,               // Unkown video type
    AV_VIDEO_CODEC_MPEG1 = 1,              // MPEG1
    AV_VIDEO_CODEC_MPEG2 = 2,              // MPEG2
    AV_VIDEO_CODEC_H264 = 3,               // H264
    AV_VIDEO_CODEC_H265 = 4,               // H265
    AV_VIDEO_CODEC_VP9 = 5,               // VP9
    AV_VIDEO_CODEC_AVS = 6,               // AVS
    AV_VIDEO_CODEC_MPEG4 = 7,              // MPEG4
} am_tsplayer_video_codec;

/*Audio decoder type*/
typedef enum {
    AV_AUDIO_CODEC_AUTO = 0,               // Unkown video type
    AV_AUDIO_CODEC_MP2 = 1,                // MPEG audio
    AV_AUDIO_CODEC_MP3 = 2,                // MP3
    AV_AUDIO_CODEC_AC3 = 3,                // AC3
    AV_AUDIO_CODEC_EAC3 = 4,               // DD PLUS
    AV_AUDIO_CODEC_DTS = 5,                // DD PLUS
    AV_AUDIO_CODEC_AAC = 6,                // AAC
    AV_AUDIO_CODEC_LATM = 7,               // AAC LATM
    AV_AUDIO_CODEC_PCM = 8,                // PCM
//    AV_AUDIO_CODEC_HEAAC = 8,            // HEAAC
//    AV_AUDIO_CODEC_AAC_ADTS = 9,         // AAC_ADTS
//    AV_AUDIO_CODEC_HEAACV2 = 10          // HEAAC VERSION2
} am_tsplayer_audio_codec;

/*AmTsPlayer handle*/
typedef size_t am_tsplayer_handle;

/*AmTsPlayer init parameters*/
typedef struct {
    am_tsplayer_input_source_type source;  // Input source type
    am_tsplayer_input_buffer_type drmmode; // Input buffer type (normal, secure, tvp)
    int32_t dmx_dev_id;                    // Demux device id
    int32_t event_mask;                    // Mask the event type need by caller
} am_tsplayer_init_params;

/*AmTsPlayer input buffer type*/
typedef struct {
    am_tsplayer_input_buffer_type buf_type;// Input buffer type (secure/no secure)
    void *buf_data;                        // Input buffer addr
    int32_t buf_size;                      // Input buffer size
} am_tsplayer_input_buffer;

/*AmTsPlayer input buffer type*/
typedef struct {
    am_tsplayer_input_buffer_type buf_type;// Input buffer type (secure/no secure)
    void *buf_data;                        // Input buffer addr
    int32_t buf_size;                      // Input buffer size
    uint64_t pts;                          //frame pts,using only for frame mode
    int32_t isvideo;
} am_tsplayer_input_frame_buffer;

/*AmTsPlayer video init parameters*/
typedef struct {
    am_tsplayer_video_codec codectype;     // Video codec type
    int32_t pid;                           // Video pid in ts
} am_tsplayer_video_params;

/*AmTsPlayer audio init parameters*/
typedef struct {
    am_tsplayer_audio_codec codectype;     // Audio codec type
    int32_t pid;                           // Audio pid in ts
    int32_t seclevel;                      // Audio security seclevel
} am_tsplayer_audio_params;

/*AmTsPlayer stream buffer status*/
typedef struct {
    int32_t size;                          // Buffer size
    int32_t data_len;                      // The len of data in buffer
    int32_t free_len;                      // The len of free in buffer
} am_tsplayer_buffer_stat;

/*Video basic information*/
typedef struct {
    uint32_t width;                        // Video frame width
    uint32_t height;                       // Video frame height
    uint32_t framerate;                    // Video frame rate
    uint32_t bitrate;                      // Video bit rate
    uint64_t ratio64;                      // Video aspect ratio
} am_tsplayer_video_info;

/*Video qos information*/
typedef struct {
    uint32_t num;
    uint32_t type;
    uint32_t size;
    uint32_t pts;
    uint32_t max_qp;
    uint32_t avg_qp;
    uint32_t min_qp;
    uint32_t max_skip;
    uint32_t avg_skip;
    uint32_t min_skip;
    uint32_t max_mv;
    uint32_t min_mv;
    uint32_t avg_mv;
    uint32_t decode_buffer;
} am_tsplayer_video_qos;

/*Video decoder real time information*/
typedef struct {
    am_tsplayer_video_qos qos;
    uint32_t  decode_time_cost;/*us*/
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t frame_rate;
    uint32_t bit_depth_luma;//original bit_rate;
    uint32_t frame_dur;
    uint32_t bit_depth_chroma;//original frame_data;
    uint32_t error_count;
    uint32_t status;
    uint32_t frame_count;
    uint32_t error_frame_count;
    uint32_t drop_frame_count;
    uint64_t total_data;
    uint32_t double_write_mode;//original samp_cnt;
    uint32_t offset;
    uint32_t ratio_control;
    uint32_t vf_type;
    uint32_t signal_type;
    uint32_t pts;
    uint64_t pts_us64;
} am_tsplayer_vdec_stat;

/*Audio basic information*/
typedef struct {
    uint32_t sample_rate;                  // Audio sample rate
    uint32_t channels;                     // Audio channels
    uint32_t channel_mask;                 // Audio channel mask
    uint32_t bitrate;                      // Audio bit rate
} am_tsplayer_audio_info;

/*Audio decoder real time information*/
typedef struct {
    uint32_t frame_count;
    uint32_t error_frame_count;
    uint32_t drop_frame_count;
} am_tsplayer_adec_stat;

typedef struct {
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t frame_rate;
    uint32_t frame_aspectratio;
} am_tsplayer_video_format_t;

typedef struct {
    uint32_t sample_rate;
    uint32_t channels;
} am_tsplayer_audio_format_t;

typedef struct {
    am_tsplayer_stream_type stream_type;
    uint64_t  pts;
} am_tsplayer_pts_t;

typedef struct {
    uint8_t  *data;
    size_t   len;
} mpeg_user_data_t;

typedef struct {
    am_tsplayer_stream_type stream_type;
    bool_t  scramling;
} scamling_t;

/*AmTsPlayer call back event*/
typedef struct {
    am_tsplayer_event_type type;           // Call back event type
    union {
        /*If type is VIDEO_CHANGED send new video basic info*/
        am_tsplayer_video_format_t video_format;
        /*If type is AUDIO_CHANGED send new video basic info*/
        am_tsplayer_audio_format_t audio_format;
        /*Audio /Video/Subtitle pts after pes parser*/
        am_tsplayer_pts_t pts;
        /*User data send cc /afd /dvb subtitle to caller*/
        mpeg_user_data_t mpeg_user_data;
        /*Scrambling status changed send scramling info to caller*/
        scamling_t scramling;
        /*callback audio/video input buffer ptr*/
        void* bufptr;
    } event;
}am_tsplayer_event;

/*Event call back function ptr*/
typedef void (*event_callback) (void *user_data, am_tsplayer_event *event);


/**
 *\brief:        Create AmTsPlayer instance.
 *               Set inputmode demux_id and event mask to AmTsPlayer.
 *\inparam:      Init params with input mode demux_id and event mask.
 *\outparam:     AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_create(am_tsplayer_init_params Params, am_tsplayer_handle *pHadl);
/**
 *\brief:        Get AmTsPlayer interface version inforamtion.
 *\outparam:     AmTsPlayer interface version.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getVersion(uint32_t *versionM,
                                          uint32_t *VersionL);
/**
 *\brief:        Get the instance number of specified AmTsPlayer .
 *\inparam:      AmTsPlayer handle.
 *\outparam:     AmTsPlayer instance number.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getInstansNo(am_tsplayer_handle Hadl, uint32_t *Numb);
/**
 *\brief:        Register event callback to specified AmTsPlayer
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Event callback function ptr.
 *\inparam:      Extra data ptr.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_registerCb(am_tsplayer_handle Hadl, event_callback pfunc, void *param);
/**
 *\brief:        Get event callback to specified AmTsPlayer
 *\inparam:      AmTsPlayer handle.
 *\inparam:      ptr of Event callback function ptr.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getCb(am_tsplayer_handle Hadl, event_callback *pfunc, void* *ppParam);
/**
 *\brief:        Release specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_release(am_tsplayer_handle Hadl);
/**
 *\brief:        Write Frame data to specified AmTsPlayer instance.
 *               It will only work when TS input's source type is TS_MEMORY.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Input buffer struct (1.Buffer type:secrue/no
 *               2.secure buffer ptr 3.buffer len).
 *\inparam:      Time out limit .
 *\inparam:      AV_type(0 means audio, 1 means video)
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_writeFrameData(am_tsplayer_handle Hadl,
                                                   am_tsplayer_input_frame_buffer *buf,
                                                   uint64_t timeout_ms);
/**
 *\brief:        Write data to specified AmTsPlayer instance.
 *               It will only work when TS input's source type is TS_MEMORY.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Input buffer struct (1.Buffer type:secrue/no
 *               2.secure buffer ptr 3.buffer len).
 *\inparam:      Time out limit .
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_writeData(am_tsplayer_handle Hadl, am_tsplayer_input_buffer *buf, uint64_t timeout_ms);
/**
 *\brief:        Set work mode to specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The enum of work mode.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setWorkMode (am_tsplayer_handle Hadl, am_tsplayer_work_mode mode);

/*AV sync*/
/**
 *\brief:        Get the playing time of specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     Playing time.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getCurrentTime(am_tsplayer_handle Hadl, int64_t *time);

/**
 *\brief:        Get the pts of specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      stream type.
 *\outparam:     pts.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getPts(am_tsplayer_handle Hadl, am_tsplayer_stream_type StrType, uint64_t *pts);
/**
 *\brief:        Set the tsync mode for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The enum of avsync mode.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setSyncMode(am_tsplayer_handle Hadl, am_tsplayer_avsync_mode mode );
/**
 *\brief:        Get the tsync mode for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The avsync mode of specified AmTsPlayer instance.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getSyncMode(am_tsplayer_handle Hadl, am_tsplayer_avsync_mode *mode );
/**
 *\brief:        Set pcr pid to specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The pid of pcr.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setPcrPid(am_tsplayer_handle Hadl, uint32_t pid);
/**
 *\brief:        Get the dealy time for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The AmTsPlayer delay time.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getDelayTime(am_tsplayer_handle Hadl, int64_t *time);


/*Player control interface*/
/**
 *\brief:        Start Fast play for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Fast play speed.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_startFast(am_tsplayer_handle Hadl, float scale);
/**
 *\brief:        Stop Fast play for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_stopFast(am_tsplayer_handle Hadl);
/**
 *\brief:        Start trick mode for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The enum of trick mode type
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setTrickMode(am_tsplayer_handle Hadl, am_tsplayer_video_trick_mode trickmode);
/**
 *\brief:        Start trick mode for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The stream type we want to check.
 *\outparam:     The struct of buffer status.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getBufferStat(am_tsplayer_handle Hadl, am_tsplayer_stream_type StrType,
                                                            am_tsplayer_buffer_stat *pBufStat);

/*Video interface*/
/**
 *\brief:        Set the video display rect size for specified
 *               AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The display rect x.
 *\inparam:      The display rect y.
 *\inparam:      The display rect width.
 *\inparam:      The display rect height.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setVideoWindow(am_tsplayer_handle Hadl,
                                                            int32_t x,int32_t y,
                                                            int32_t width,int32_t height);
/**
 *\brief:        Set Surface ptr to specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Surface ptr
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setSurface(am_tsplayer_handle Hadl, void* pSurface);
/**
 *\brief:        Show the video frame display for specified
 *               AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_showVideo(am_tsplayer_handle Hadl);
/**
 *\brief:        Hide the video frame display for specified
 *               AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_hideVideo(am_tsplayer_handle Hadl);
/**
 *\brief:        Get video display match mode for specified
                 AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The enum of video display match mode.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setVideoMatchMode(am_tsplayer_handle Hadl, am_tsplayer_video_match_mode MathMod);
/**
 *\brief:        Set video params need by demuxer and video decoder
 *               for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Params need by demuxer and video decoder.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setVideoParams(am_tsplayer_handle Hadl, am_tsplayer_video_params *pParams);
/**
 *\brief:        Set if need keep last frame for video display
 *               for specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      If blackout for last frame.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setVideoBlackOut(am_tsplayer_handle Hadl, bool_t blackout);
/**
 *\brief:        Get video basic info of specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The ptr of video basic info struct .
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getVideoInfo(am_tsplayer_handle Hadl, am_tsplayer_video_info *pInfo);
/**
 *\brief:        Get video decoder real time info
 *               of specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The ptr of video decoder real time info struct
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getVideoStat(am_tsplayer_handle Hadl, am_tsplayer_vdec_stat *pStat);
/**
 *\brief:        Start video decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_startVideoDecoding(am_tsplayer_handle Hadl);
/**
 *\brief:        Pause video decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_pauseVideoDecoding(am_tsplayer_handle Hadl);
/**
 *\brief:        Resume video decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_resumeVideoDecoding(am_tsplayer_handle Hadl);
/**
 *\brief:        Stop video decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_stopVideoDecoding(am_tsplayer_handle Hadl);


/*Audio interface*/
/**
 *\brief:        Set audio volume to specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Volume value.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setAudioVolume(am_tsplayer_handle Hadl, int32_t volume);
/**
 *\brief:        Get audio volume value from specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\outparam:     Volume value.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getAudioVolume(am_tsplayer_handle Hadl, int32_t *volume);
/**
 *\brief:        Set audio stereo mode to specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Stereo mode.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setAudioStereoMode(am_tsplayer_handle Hadl, am_tsplayer_audio_stereo_mode Mode);
/**
 *\brief:        Get audio stereo mode to specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\outparam:     Stereo mode.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getAudioStereoMode(am_tsplayer_handle Hadl, am_tsplayer_audio_stereo_mode *pMode);
/**
 *\brief:        Set audio output mute to specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\inparam:      If analog mute or unmute .
 *\inparam:      If digital mute or unmute .
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setAudioMute(am_tsplayer_handle Hadl, bool_t analog_mute, bool_t digital_mute);
/**
 *\brief:        Get audio output mute status from specified
                 AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\outparam:     If analog mute or unmute .
 *\outparam:     If digital mute or unmute .
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getAudioMute(am_tsplayer_handle Hadl, bool_t *analog_unmute, bool_t *digital_unmute);
/**
 *\brief:        Set audio params need by demuxer and audio decoder
 *               to specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Params need by demuxer and audio decoder.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setAudioParams(am_tsplayer_handle Hadl, am_tsplayer_audio_params *pParams);
/**
 *\brief:        Set audio output mode to specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Enum of audio output mode.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setAudioOutMode(am_tsplayer_handle Hadl, am_tsplayer_audio_out_mode Mode);
/**
 *\brief:        Get audio basic info of specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The ptr of audio basic info struct .
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getAudioInfo(am_tsplayer_handle Hadl,  am_tsplayer_audio_info *pInfo);
/**
 *\brief:        Get audio decoder real time info
 *               of specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The ptr of audio decoder real time info struct
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getAudioStat(am_tsplayer_handle Hadl, am_tsplayer_adec_stat *pStat);
/**
 *\brief:        Start audio decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_startAudioDecoding(am_tsplayer_handle Hadl);
/**
 *\brief:        Pause audio decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_pauseAudioDecoding(am_tsplayer_handle Hadl);
/**
 *\brief:        Resume audio decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_resumeAudioDecoding(am_tsplayer_handle Hadl);
/**
 *\brief:        Stop audio decoding for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_stopAudioDecoding(am_tsplayer_handle Hadl);
/**
 *\brief:        Set audio description params need by demuxer
 *               and audio decoder to specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Params need by demuxer and audio decoder.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setADParams(am_tsplayer_handle Hadl, am_tsplayer_audio_params *pParams);

/*Audio description interface*/
/**
 *\brief:        Set audio description mix level (master vol and ad vol)
 *\inparam:      AmTsPlayer handle.
 *\inparam:      Master volume value.
 *\inparam:      Slave volume value.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setADMixLevel(am_tsplayer_handle Hadl, int32_t master_vol, int32_t slave_vol);
/**
 *\brief:        Get audio description mix level (master vol and ad vol)
 *\inparam:      AmTsPlayer handle.
 *\outparam:     Master volume value.
 *\outparam:     Slave volume value.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getADMixLevel(am_tsplayer_handle Hadl, int32_t *master_vol, int32_t *slave_vol);
/**
 *\brief:        Enable audio description mix with master audio
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_enableADMix(am_tsplayer_handle Hadl);
/**
 *\brief:        Disable audio description mix with master audio
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_disableADMix(am_tsplayer_handle Hadl);
/**
 *\brief:        Get audio description basic info of specified
 *               AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The ptr of audio basic info struct .
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getADInfo(am_tsplayer_handle Hadl, am_tsplayer_audio_info *pInfo);
/**
 *\brief:        Get audio description decoder real time info
 *               of specified AmTsPlayer instance.
 *\inparam:      AmTsPlayer handle.
 *\outparam:     The ptr of audio decoder real time info struct
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_getADStat(am_tsplayer_handle Hadl, am_tsplayer_adec_stat *pStat);

/*Subtitle interface*/
/**
 *\brief:        Set subtitle pid for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\inparam:      The pid of subtitle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_setSubPid(am_tsplayer_handle Hadl, uint32_t pid);
/**
 *\brief:        Start subtitle for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_startSub(am_tsplayer_handle Hadl);
/**
 *\brief:        Stop subtitle for specified AmTsPlayer instance .
 *\inparam:      AmTsPlayer handle.
 *\return:       The AmTsPlayer result.
 */
am_tsplayer_result  AmTsPlayer_stopSub(am_tsplayer_handle Hadl);

#ifdef __cplusplus
}
#endif

#endif


\
