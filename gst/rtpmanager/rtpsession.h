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

#ifndef __RTP_SESSION_H__
#define __RTP_SESSION_H__

#include <gst/gst.h>
#include <gst/netbuffer/gstnetbuffer.h>

#include "rtpsource.h"

typedef struct _RTPSession RTPSession;
typedef struct _RTPSessionClass RTPSessionClass;

#define RTP_TYPE_SESSION             (rtp_session_get_type())
#define RTP_SESSION(sess)            (G_TYPE_CHECK_INSTANCE_CAST((sess),RTP_TYPE_SESSION,RTPSession))
#define RTP_SESSION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),RTP_TYPE_SESSION,RTPSessionClass))
#define RTP_IS_SESSION(sess)         (G_TYPE_CHECK_INSTANCE_TYPE((sess),RTP_TYPE_SESSION))
#define RTP_IS_SESSION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),RTP_TYPE_SESSION))
#define RTP_SESSION_CAST(sess)       ((RTPSession *)(sess))

#define RTP_SESSION_LOCK(sess)     (g_mutex_lock ((sess)->lock))
#define RTP_SESSION_UNLOCK(sess)   (g_mutex_unlock ((sess)->lock))

/**
 * RTPSessionProcessRTP:
 * @sess: an #RTPSession
 * @src: the #RTPSource
 * @buffer: the RTP buffer ready for processing
 * @user_data: user data specified when registering
 *
 * This callback will be called when @sess has @buffer ready for further
 * processing. Processing the buffer typically includes decoding and displaying
 * the buffer.
 *
 * Returns: a #GstFlowReturn.
 */
typedef GstFlowReturn (*RTPSessionProcessRTP) (RTPSession *sess, RTPSource *src, GstBuffer *buffer, gpointer user_data);

/**
 * RTPSessionSendRTP:
 * @sess: an #RTPSession
 * @src: the #RTPSource
 * @buffer: the RTP buffer ready for sending
 * @user_data: user data specified when registering
 *
 * This callback will be called when @sess has @buffer ready for sending to
 * all listening participants in this session.
 *
 * Returns: a #GstFlowReturn.
 */
typedef GstFlowReturn (*RTPSessionSendRTP) (RTPSession *sess, RTPSource *src, GstBuffer *buffer, gpointer user_data);

/**
 * RTPSessionSendRTCP:
 * @sess: an #RTPSession
 * @src: the #RTPSource
 * @buffer: the RTCP buffer ready for sending
 * @user_data: user data specified when registering
 *
 * This callback will be called when @sess has @buffer ready for sending to
 * all listening participants in this session.
 *
 * Returns: a #GstFlowReturn.
 */
typedef GstFlowReturn (*RTPSessionSendRTCP) (RTPSession *sess, RTPSource *src, GstBuffer *buffer, gpointer user_data);

/**
 * RTPSessionClockRate:
 * @sess: an #RTPSession
 * @payload: the payload
 * @user_data: user data specified when registering
 *
 * This callback will be called when @sess needs the clock-rate of @payload.
 *
 * Returns: the clock-rate of @pt.
 */
typedef gint (*RTPSessionClockRate) (RTPSession *sess, guint8 payload, gpointer user_data);

/**
 * RTPSessionGetTime:
 * @sess: an #RTPSession
 * @user_data: user data specified when registering
 *
 * This callback will be called when @sess needs the current time in
 * nanoseconds.
 *
 * Returns: a #GstClockTime with the current time in nanoseconds.
 */
typedef GstClockTime (*RTPSessionGetTime) (RTPSession *sess, gpointer user_data);

/**
 * RTPSessionCallbacks:
 * @RTPSessionProcessRTP: callback to process RTP packets
 * @RTPSessionSendRTP: callback for sending RTP packets
 * @RTPSessionSendRTCP: callback for sending RTCP packets
 * @RTPSessionGetTime: callback for returning the current time
 *
 * These callbacks can be installed on the session manager to get notification
 * when RTP and RTCP packets are ready for further processing. These callbacks
 * are not implemented with signals for performance reasons.
 */
typedef struct {
  RTPSessionProcessRTP  process_rtp;
  RTPSessionSendRTP     send_rtp;
  RTPSessionSendRTCP    send_rtcp;
  RTPSessionClockRate   clock_rate;
  RTPSessionGetTime     get_time;
} RTPSessionCallbacks;

/**
 * RTPSession:
 * @lock: lock to protect the session
 * @source: the source of this session
 * @ssrcs: Hashtable of sources indexed by SSRC
 * @cnames: Hashtable of sources indexed by CNAME
 * @num_sources: the number of sources
 * @activecount: the number of active sources
 * @callbacks: callbacks
 * @user_data: user data passed in callbacks
 *
 * The RTP session manager object
 */
struct _RTPSession {
  GObject       object;

  GMutex       *lock;

  guint         header_len;

  RTPSource    *source;
  GHashTable   *ssrcs;
  GHashTable   *cnames;
  guint         total_sources;

  RTPSessionCallbacks callbacks;
  gpointer            user_data;

  RTPSessionStats stats;
};

/**
 * RTPSessionClass:
 * @on_new_ssrc: emited when a new source is found
 * @on_bye_ssrc: emited when a source is gone
 *
 * The session class.
 */
struct _RTPSessionClass {
  GObjectClass   parent_class;

  /* signals */
  void (*on_new_ssrc)       (RTPSession *sess, RTPSource *source);
  void (*on_ssrc_collision) (RTPSession *sess, RTPSource *source);
  void (*on_ssrc_validated) (RTPSession *sess, RTPSource *source);
  void (*on_bye_ssrc)       (RTPSession *sess, RTPSource *source);
};

GType rtp_session_get_type (void);

/* create and configure */
RTPSession*     rtp_session_new           (void);
void            rtp_session_set_callbacks          (RTPSession *sess, 
		                                    RTPSessionCallbacks *callbacks,
                                                    gpointer user_data);
void            rtp_session_set_bandwidth          (RTPSession *sess, gdouble bandwidth);
gdouble         rtp_session_get_bandwidth          (RTPSession *sess);
void            rtp_session_set_rtcp_fraction      (RTPSession *sess, gdouble fraction);
gdouble         rtp_session_get_rtcp_fraction      (RTPSession *sess);

/* handling sources */
gboolean        rtp_session_add_source             (RTPSession *sess, RTPSource *src);
gint            rtp_session_get_num_sources        (RTPSession *sess);
gint            rtp_session_get_num_active_sources (RTPSession *sess);
RTPSource*      rtp_session_get_source_by_ssrc     (RTPSession *sess, guint32 ssrc);
RTPSource*      rtp_session_get_source_by_cname    (RTPSession *sess, const gchar *cname);
RTPSource*      rtp_session_create_source          (RTPSession *sess);

/* processing packets from receivers */
GstFlowReturn   rtp_session_process_rtp            (RTPSession *sess, GstBuffer *buffer);
GstFlowReturn   rtp_session_process_rtcp           (RTPSession *sess, GstBuffer *buffer);

/* processing packets for sending */
GstFlowReturn   rtp_session_send_rtp               (RTPSession *sess, GstBuffer *buffer);

/* get interval for next RTCP interval */
gdouble         rtp_session_get_rtcp_interval      (RTPSession *sess);
GstFlowReturn   rtp_session_produce_rtcp           (RTPSession *sess);

#endif /* __RTP_SESSION_H__ */