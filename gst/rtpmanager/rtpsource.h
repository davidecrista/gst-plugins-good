/* GStreamer
 * Copyright (C) <2007> Wim Taymans <wim@fluendo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __RTP_SOURCE_H__
#define __RTP_SOURCE_H__

#include <gst/gst.h>
#include <gst/rtp/gstrtcpbuffer.h>
#include <gst/netbuffer/gstnetbuffer.h>

#include "rtpstats.h"

/* the default number of consecutive RTP packets we need to receive before the
 * source is considered valid */
#define RTP_NO_PROBATION        0
#define RTP_DEFAULT_PROBATION   2

typedef struct _RTPSource RTPSource;
typedef struct _RTPSourceClass RTPSourceClass;

#define RTP_TYPE_SOURCE             (rtp_source_get_type())
#define RTP_SOURCE(src)             (G_TYPE_CHECK_INSTANCE_CAST((src),RTP_TYPE_SOURCE,RTPSource))
#define RTP_SOURCE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),RTP_TYPE_SOURCE,RTPSourceClass))
#define RTP_IS_SOURCE(src)          (G_TYPE_CHECK_INSTANCE_TYPE((src),RTP_TYPE_SOURCE))
#define RTP_IS_SOURCE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),RTP_TYPE_SOURCE))
#define RTP_SOURCE_CAST(src)        ((RTPSource *)(src))

/**
 * RTP_SOURCE_IS_ACTIVE:
 * @src: an #RTPSource
 *
 * Check if @src is active. A source is active when it has been validated
 * and has not yet received a BYE packet.
 */
#define RTP_SOURCE_IS_ACTIVE(src)  (src->validated && !src->received_bye)

/**
 * RTP_SOURCE_IS_SENDER:
 * @src: an #RTPSource
 *
 * Check if @src is a sender.
 */
#define RTP_SOURCE_IS_SENDER(src)  (src->is_sender)

/**
 * RTPSourcePushRTP:
 * @src: an #RTPSource
 * @buffer: the RTP buffer ready for processing
 * @user_data: user data specified when registering
 *
 * This callback will be called when @src has @buffer ready for further
 * processing.
 *
 * Returns: a #GstFlowReturn.
 */
typedef GstFlowReturn (*RTPSourcePushRTP) (RTPSource *src, GstBuffer *buffer, gpointer user_data);

/**
 * RTPSourceClockRate:
 * @src: an #RTPSource
 * @payload: a payload type
 * @user_data: user data specified when registering
 *
 * This callback will be called when @src needs the clock-rate of the
 * @payload.
 *
 * Returns: a clock-rate for @payload.
 */
typedef gint (*RTPSourceClockRate) (RTPSource *src, guint8 payload, gpointer user_data);

/**
 * RTPSourceCallbacks:
 * @push_rtp: a packet becomes available for handling
 * @clock_rate: a clock-rate is requested
 * @get_time: the current clock time is requested
 *
 * Callbacks performed by #RTPSource when actions need to be performed.
 */
typedef struct {
  RTPSourcePushRTP     push_rtp;
  RTPSourceClockRate   clock_rate;
} RTPSourceCallbacks;

/**
 * RTPSource:
 *
 * A source in the #RTPSession
 */
struct _RTPSource {
  GObject       object;

  /*< private >*/
  RTPSourceCallbacks callbacks;
  gpointer           user_data;

  guint32       ssrc;
  gchar        *cname;
  gint          probation;
  gboolean      validated;
  gboolean      received_bye;
  gchar        *bye_reason;

  gboolean      is_csrc;
  gboolean      is_sender;

  gboolean      have_rtp_from;
  GstNetAddress rtp_from;
  gboolean      have_rtcp_from;
  GstNetAddress rtcp_from;

  guint8        payload;
  gint          clock_rate;

  GQueue       *packets;

  RTPSourceStats stats;
};

struct _RTPSourceClass {
  GObjectClass   parent_class;
};

GType rtp_source_get_type (void);

/* managing lifetime of sources */
RTPSource*      rtp_source_new            (guint32 ssrc);

void            rtp_source_set_callbacks  (RTPSource *src, RTPSourceCallbacks *cb, gpointer data);
void            rtp_source_set_as_csrc    (RTPSource *src);

void            rtp_source_set_rtp_from   (RTPSource *src, GstNetAddress *address);
void            rtp_source_set_rtcp_from  (RTPSource *src, GstNetAddress *address);

GstFlowReturn   rtp_source_process_rtp    (RTPSource *src, GstBuffer *buffer, RTPArrivalStats *arrival);

GstFlowReturn   rtp_source_send_rtp       (RTPSource *src, GstBuffer *buffer);

/* RTCP messages */
void            rtp_source_process_bye    (RTPSource *src, const gchar *reason);
void            rtp_source_process_sr     (RTPSource *src, guint64 ntptime, guint32 rtptime,
                                           guint32 packet_count, guint32 octet_count);
void            rtp_source_process_rb     (RTPSource *src, guint8 fractionlost, gint32 packetslost,
                                           guint32 exthighestseq, guint32 jitter,
                                           guint32 lsr, guint32 dlsr);

#endif /* __RTP_SOURCE_H__ */