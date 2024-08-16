/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef _MEDIA_METRAICS_DATA_H_
#define _MEDIA_METRAICS_DATA_H_


typedef struct _media_metrics_info {
    uint64_t rendered_cnt;
    uint64_t drop_cnt;
    uint64_t inframe_cnt;
    uint64_t outframe_cnt;
} media_metrics_info;

typedef struct _metrics_frame_info {
    int32_t decoder_instid;
    int32_t vd_instid;
    int64_t frame_index;
    int64_t ino;
    int64_t bitstreamid;
    int64_t decoded_time;
    int64_t toggle_time;
    int64_t signalfence_time;
    int64_t framein_time;
    int32_t drop;
    int64_t mediatime;
    int32_t reserved[4];
} metrics_frame_info;

#endif
