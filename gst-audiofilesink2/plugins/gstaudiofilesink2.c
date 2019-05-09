/* GStreamer
 * Copyright (C) 2019 FIXME <fixme@example.com>
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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstaudiofilesink2
 *
 * The audiofilesink2 element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! audiofilesink2 ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/audio/gstaudiosink.h>
#include "gstaudiofilesink2.h"

GST_DEBUG_CATEGORY_STATIC (gst_audiofilesink2_debug_category);
#define GST_CAT_DEFAULT gst_audiofilesink2_debug_category

/* prototypes */


static void gst_audiofilesink2_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_audiofilesink2_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_audiofilesink2_dispose (GObject * object);
static void gst_audiofilesink2_finalize (GObject * object);

static gboolean gst_audiofilesink2_open (GstAudioSink * sink);
static gboolean gst_audiofilesink2_prepare (GstAudioSink * sink,
    GstAudioRingBufferSpec * spec);
static gboolean gst_audiofilesink2_unprepare (GstAudioSink * sink);
static gboolean gst_audiofilesink2_close (GstAudioSink * sink);
static gint gst_audiofilesink2_write (GstAudioSink * sink, gpointer data,
    guint length);
static guint gst_audiofilesink2_delay (GstAudioSink * sink);
static void gst_audiofilesink2_reset (GstAudioSink * sink);

enum
{
  PROP_0,
  PROP_LOCATION
};

/* pad templates */

/* FIXME add/remove the formats that you want to support */
static GstStaticPadTemplate gst_audiofilesink2_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=16000,"
      "channels=[1,max],layout=interleaved")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstAudiofilesink2, gst_audiofilesink2, GST_TYPE_AUDIO_SINK,
  GST_DEBUG_CATEGORY_INIT (gst_audiofilesink2_debug_category, "audiofilesink2", 0,
  "debug category for audiofilesink2 element"));

static void
gst_audiofilesink2_class_init (GstAudiofilesink2Class * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstAudioSinkClass *audio_sink_class = GST_AUDIO_SINK_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_audiofilesink2_sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_audiofilesink2_set_property;
  gobject_class->get_property = gst_audiofilesink2_get_property;

  g_object_class_install_property(gobject_class, PROP_LOCATION,
                                  g_param_spec_string("location", "Location",
                                                      "Location of the audio file.",
                                                      "/tmp/audio.raw", 
                                                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gobject_class->dispose = gst_audiofilesink2_dispose;
  gobject_class->finalize = gst_audiofilesink2_finalize;
  audio_sink_class->open = GST_DEBUG_FUNCPTR (gst_audiofilesink2_open);
  audio_sink_class->prepare = GST_DEBUG_FUNCPTR (gst_audiofilesink2_prepare);
  audio_sink_class->unprepare = GST_DEBUG_FUNCPTR (gst_audiofilesink2_unprepare);
  audio_sink_class->close = GST_DEBUG_FUNCPTR (gst_audiofilesink2_close);
  audio_sink_class->write = GST_DEBUG_FUNCPTR (gst_audiofilesink2_write);
  audio_sink_class->delay = GST_DEBUG_FUNCPTR (gst_audiofilesink2_delay);
  audio_sink_class->reset = GST_DEBUG_FUNCPTR (gst_audiofilesink2_reset);

}

static void
gst_audiofilesink2_init (GstAudiofilesink2 *audiofilesink2)
{
    audiofilesink2->buffer= gst_buffer_new();
}

void
gst_audiofilesink2_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (object);

  GST_DEBUG_OBJECT (audiofilesink2, "set_property");

  switch (property_id) {
    case PROP_LOCATION:
        g_free(audiofilesink2->location);
        audiofilesink2->location = g_strdup(g_value_get_string(value));
    break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_audiofilesink2_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (object);

  GST_DEBUG_OBJECT (audiofilesink2, "get_property");

  switch (property_id) {
    case PROP_LOCATION:
      g_value_set_string(value, audiofilesink2->location);
    break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_audiofilesink2_dispose (GObject * object)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (object);

  GST_DEBUG_OBJECT (audiofilesink2, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_audiofilesink2_parent_class)->dispose (object);
}

void
gst_audiofilesink2_finalize (GObject * object)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (object);

  GST_DEBUG_OBJECT (audiofilesink2, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_audiofilesink2_parent_class)->finalize (object);
}

/* open the device with given specs */
static gboolean
gst_audiofilesink2_open (GstAudioSink * sink)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (sink);

  GST_DEBUG_OBJECT (audiofilesink2, "open");

  return TRUE;
}

/* prepare resources and state to operate with the given specs */
static gboolean
gst_audiofilesink2_prepare (GstAudioSink * sink, GstAudioRingBufferSpec * spec)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (sink);

  GST_DEBUG_OBJECT (audiofilesink2, "prepare");

  return TRUE;
}

/* undo anything that was done in prepare() */
static gboolean
gst_audiofilesink2_unprepare (GstAudioSink * sink)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (sink);

  GST_DEBUG_OBJECT (audiofilesink2, "unprepare");

  return TRUE;
}

/* close the device */
static gboolean
gst_audiofilesink2_close (GstAudioSink * sink)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (sink);
  g_print("WRITING TO FILE");

  GstBuffer *buffer = GST_BUFFER(audiofilesink2->buffer);
  GstMapInfo info;
  gst_buffer_map(buffer, &info, GST_MAP_READ);

  g_file_set_contents(audiofilesink2->location, (gchar*)info.data, info.size, NULL);

  GST_DEBUG_OBJECT (audiofilesink2, "close");

  return TRUE;
}

/* write samples to the device */
static gint
gst_audiofilesink2_write (GstAudioSink * sink, gpointer data, guint length)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (sink);

  GstBuffer *buffer = gst_buffer_new_allocate(NULL, length, NULL);
  gst_buffer_fill(buffer, 0, data, length);

  audiofilesink2->buffer = gst_buffer_append(audiofilesink2->buffer, gst_buffer_copy_deep(buffer));

  g_usleep(10000);

  GST_DEBUG_OBJECT (audiofilesink2, "write");

  return length;
}

/* get number of samples queued in the device */
static guint
gst_audiofilesink2_delay (GstAudioSink * sink)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (sink);
  

  GST_DEBUG_OBJECT (audiofilesink2, "delay");

  return 0;
}

/* reset the audio device, unblock from a write */
static void
gst_audiofilesink2_reset (GstAudioSink * sink)
{
  GstAudiofilesink2 *audiofilesink2 = GST_AUDIOFILESINK2 (sink);

  GST_DEBUG_OBJECT (audiofilesink2, "reset");

}