/* Quicktime muxer plugin for GStreamer
 * Copyright (C) 2008 Thiago Sousa Santos <thiagoss@embedded.ufcg.edu.br>
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

#ifndef __ATOMS_H__
#define __ATOMS_H__

#include <glib.h>
#include <string.h>

#include "descriptors.h"
#include "properties.h"
#include "fourcc.h"
#include "ftypcc.h"

/* light-weight context that may influence header atom tree construction */
typedef enum _AtomsTreeFlavor
{
  ATOMS_TREE_FLAVOR_MOV,
  ATOMS_TREE_FLAVOR_ISOM
} AtomsTreeFlavor;

typedef struct _AtomsContext
{
  AtomsTreeFlavor flavor;
} AtomsContext;

AtomsContext* atoms_context_new  (AtomsTreeFlavor flavor);
void          atoms_context_free (AtomsContext *context);

#define METADATA_DATA_FLAG 0x0
#define METADATA_TEXT_FLAG 0x1

/* atom defs and functions */

/**
 * Used for storing time related values for some atoms.
 */
typedef struct _TimeInfo
{
  guint64 creation_time;
  guint64 modification_time;
  guint32 timescale;
  guint64 duration;
} TimeInfo;

typedef struct _Atom
{
  guint32 size;
  guint32 type;
  guint64 extended_size;
} Atom;

typedef struct _AtomFull
{
  Atom header;

  guint8 version;
  guint8 flags[3];
} AtomFull;

/*
 * Generic extension atom
 */
typedef struct _AtomData
{
  Atom header;

  /* not written */
  guint32 datalen;
  guint8 *data;
} AtomData;

typedef struct _AtomFTYP
{
  Atom header;
  guint32 major_brand;
  guint32 version;
  guint32 *compatible_brands;

  /* not written */
  guint32 compatible_brands_size;
} AtomFTYP;

typedef struct _AtomMVHD
{
  AtomFull header;

  /* version 0: 32 bits */
  TimeInfo time_info;

  guint32 prefered_rate;      /* ISO: 0x00010000 */
  guint16 volume;             /* ISO: 0x0100 */
  guint16 reserved3;          /* ISO: 0x0 */
  guint32 reserved4[2];       /* ISO: 0, 0 */
  /* ISO: identity matrix =
   * { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 } */
  guint32 matrix[9];

  /* ISO: all 0 */
  guint32 preview_time;
  guint32 preview_duration;
  guint32 poster_time;
  guint32 selection_time;
  guint32 selection_duration;
  guint32 current_time;

  guint32 next_track_id;
} AtomMVHD;

typedef struct _AtomTKHD
{
  AtomFull header;

  /* version 0: 32 bits */
  /* like the TimeInfo struct, but it has this track_ID inside */
  guint64 creation_time;
  guint64 modification_time;
  guint32 track_ID;
  guint32 reserved;
  guint64 duration;

  guint32 reserved2[2];
  guint16 layer;
  guint16 alternate_group;
  guint16 volume;
  guint16 reserved3;

  /* ISO: identity matrix =
   * { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 } */
  guint32 matrix[9];
  guint32 width;
  guint32 height;
} AtomTKHD;

typedef struct _AtomMDHD
{
  AtomFull header;

  /* version 0: 32 bits */
  TimeInfo time_info;

  /* ISO: packed ISO-639-2/T language code (first bit must be 0) */
  guint16 language_code;
  /* ISO: 0 */
  guint16 quality;
} AtomMDHD;

typedef struct _AtomHDLR
{
  AtomFull header;

  /* ISO: 0 */
  guint32 component_type;
  guint32 handler_type;
  guint32 manufacturer;
  guint32 flags;
  guint32 flags_mask;
  gchar *name;
} AtomHDLR;

typedef struct _AtomVMHD
{
  AtomFull header;          /* ISO: flags = 1 */

  guint16 graphics_mode;
  /* RGB */
  guint16 opcolor[3];
} AtomVMHD;

typedef struct _AtomSMHD
{
  AtomFull header;

  guint16 balance;
  guint16 reserved;
} AtomSMHD;

typedef struct _AtomHMHD
{
  AtomFull header;

  guint16 max_pdu_size;
  guint16 avg_pdu_size;
  guint32 max_bitrate;
  guint32 avg_bitrate;
  guint32 sliding_avg_bitrate;
} AtomHMHD;

typedef struct _AtomURL
{
  AtomFull header;

  gchar *location;
} AtomURL;

typedef struct _AtomDREF
{
  AtomFull header;

  GList *entries;
} AtomDREF;

typedef struct _AtomDINF
{
  Atom header;

  AtomDREF dref;
} AtomDINF;

typedef struct _STTSEntry
{
  guint32 sample_count;
  gint32 sample_delta;
} STTSEntry;

typedef struct _AtomSTTS
{
  AtomFull header;

  guint n_entries;
  /* list of STTSEntry */
  GList *entries;
} AtomSTTS;

typedef struct _AtomSTSS
{
  AtomFull header;

  guint n_entries;
  /* list of sample indexes (guint32) */
  GList *entries;
} AtomSTSS;

typedef struct _AtomESDS
{
  AtomFull header;

  ESDescriptor es;
} AtomESDS;

typedef struct _AtomFRMA
{
  Atom header;

  guint32 media_type;
} AtomFRMA;

typedef enum _SampleEntryKind
{
  UNKNOWN,
  AUDIO,
  VIDEO
} SampleEntryKind;

typedef struct _SampleTableEntry
{
  Atom header;

  guint8 reserved[6];
  guint16 data_reference_index;

  /* sort of entry */
  SampleEntryKind kind;
} SampleTableEntry;

typedef struct _AtomHintSampleEntry
{
  SampleTableEntry se;
  guint32 size;
  guint8 *data;
} AtomHintSampleEntry;

typedef struct _SampleTableEntryMP4V
{
  SampleTableEntry se;

  guint16 version;
  guint16 revision_level;

  guint32 vendor;                 /* fourcc code */
  guint32 temporal_quality;
  guint32 spatial_quality;

  guint16 width;
  guint16 height;

  guint32 horizontal_resolution;
  guint32 vertical_resolution;
  guint32 datasize;

  guint16 frame_count;            /* usually 1 */

  guint8 compressor[32];         /* pascal string, i.e. first byte = length */

  guint16 depth;
  guint16 color_table_id;

  /* (optional) list of AtomInfo */
  GList *extension_atoms;
} SampleTableEntryMP4V;

typedef struct _SampleTableEntryMP4A
{
  SampleTableEntry se;

  guint16 version;
  guint16 revision_level;
  guint32 vendor;

  guint16 channels;
  guint16 sample_size;
  guint16 compression_id;
  guint16 packet_size;

  guint32 sample_rate;            /* fixed point 16.16 */

  guint32 samples_per_packet;
  guint32 bytes_per_packet;
  guint32 bytes_per_frame;
  guint32 bytes_per_sample;

  /* (optional) list of AtomInfo */
  GList *extension_atoms;
} SampleTableEntryMP4A;

typedef struct _SampleTableEntryMP4S
{
  SampleTableEntry se;

  AtomESDS es;
} SampleTableEntryMP4S;

typedef struct _AtomSTSD
{
  AtomFull header;

  guint n_entries;
  /* list of subclasses of SampleTableEntry */
  GList *entries;
} AtomSTSD;

typedef struct _AtomSTSZ
{
  AtomFull header;

  guint32 sample_size;

  /* need the size here because when sample_size is constant,
   * the list is empty */
  guint32 table_size;
  /* list of guint32 */
  GList *entries;
} AtomSTSZ;

typedef struct _STSCEntry
{
  guint32 first_chunk;
  guint32 samples_per_chunk;
  guint32 sample_description_index;
} STSCEntry;

typedef struct _AtomSTSC
{
  AtomFull header;

  guint n_entries;
  /* list of STSCEntry */
  GList *entries;
} AtomSTSC;


/*
 * used for both STCO and CO64
 * if used as STCO, entries should be truncated to use only 32bits
 */
typedef struct _AtomSTCO64
{
  AtomFull header;

  guint n_entries;
  /* list of guint64 */
  GList *entries;
} AtomSTCO64;

typedef struct _CTTSEntry
{
  guint32 samplecount;
  guint32 sampleoffset;
} CTTSEntry;

typedef struct _AtomCTTS
{
  AtomFull header;

  /* also entry count here */
  guint n_entries;
  GList *entries;
} AtomCTTS;

typedef struct _AtomSTBL
{
  Atom header;

  AtomSTSD stsd;
  AtomSTTS stts;
  AtomSTSS stss;
  AtomSTSC stsc;
  AtomSTSZ stsz;
  /* NULL if not present */
  AtomCTTS *ctts;

  AtomSTCO64 stco64;
} AtomSTBL;

typedef struct _AtomMINF
{
  Atom header;

  /* only (exactly) one of those must be present */
  AtomVMHD *vmhd;
  AtomSMHD *smhd;
  AtomHMHD *hmhd;

  AtomHDLR *hdlr;
  AtomDINF dinf;
  AtomSTBL stbl;
} AtomMINF;

typedef struct _AtomMDIA
{
  Atom header;

  AtomMDHD mdhd;
  AtomHDLR hdlr;
  AtomMINF minf;
} AtomMDIA;

typedef struct _AtomILST
{
  Atom header;

  /* list of AtomInfo */
  GList* entries;
} AtomILST;

typedef struct _AtomTagData
{
  AtomFull header;
  guint32 reserved;

  guint32 datalen;
  guint8* data;
} AtomTagData;

typedef struct _AtomTag
{
  Atom header;

  AtomTagData data;
} AtomTag;

typedef struct _AtomMETA
{
  AtomFull header;
  AtomHDLR hdlr;
  AtomILST *ilst;
} AtomMETA;

typedef struct _AtomUDTA
{
  Atom header;

  AtomMETA *meta;
} AtomUDTA;

typedef struct _AtomTRAK
{
  Atom header;

  AtomTKHD tkhd;
  AtomMDIA mdia;
} AtomTRAK;

typedef struct _AtomMOOV
{
  Atom header;

  AtomMVHD mvhd;

  /* list of AtomTRAK */
  GList *traks;
  AtomUDTA *udta;
} AtomMOOV;

typedef struct _AtomWAVE
{
  Atom header;

  /* list of AtomInfo */
  GList *extension_atoms;
} AtomWAVE;


/*
 * Function to serialize an atom
 */
typedef guint64 (*AtomCopyDataFunc) (Atom *atom, guint8 **buffer, guint64 *size, guint64 *offset);

/*
 * Releases memory allocated by an atom
 */
typedef guint64 (*AtomFreeFunc) (Atom *atom);

/*
 * Some atoms might have many optional different kinds of child atoms, so this
 * is useful for enabling generic handling of any atom.
 * All we need are the two functions (copying it to an array
 * for serialization and the memory releasing function).
 */
typedef struct _AtomInfo
{
  Atom *atom;
  AtomCopyDataFunc copy_data_func;
  AtomFreeFunc free_func;
} AtomInfo;


guint64    atom_copy_data              (Atom *atom, guint8 **buffer,
                                        guint64 *size, guint64* offset);

AtomFTYP*  atom_ftyp_new               (AtomsContext *context, guint32 major,
                                        guint32 version, GList *brands);
guint64    atom_ftyp_copy_data         (AtomFTYP *ftyp, guint8 **buffer,
                                        guint64 *size, guint64 *offset);
void       atom_ftyp_free              (AtomFTYP *ftyp);

AtomTRAK*  atom_trak_new               (AtomsContext *context);
void       atom_trak_add_samples       (AtomTRAK * trak, guint32 nsamples, guint32 delta,
                                        guint32 size, guint64 chunk_offset, gboolean sync,
                                        gboolean do_pts, gint64 pts_offset);
guint32    atom_trak_get_timescale     (AtomTRAK *trak);

AtomMOOV*  atom_moov_new               (AtomsContext *context);
void       atom_moov_free              (AtomMOOV *moov);
guint64    atom_moov_copy_data         (AtomMOOV *atom, guint8 **buffer, guint64 *size, guint64* offset);
void       atom_moov_update_timescale  (AtomMOOV *moov, guint32 timescale);
void       atom_moov_update_duration   (AtomMOOV *moov);
void       atom_moov_set_64bits        (AtomMOOV *moov, gboolean large_file);
void       atom_moov_chunks_add_offset (AtomMOOV *moov, guint32 offset);
void       atom_moov_add_trak          (AtomMOOV *moov, AtomTRAK *trak);

/* media sample description related helpers */

typedef struct
{
  guint32 fourcc;
  guint width;
  guint height;
  guint depth;
  guint frame_count;
  gint color_table_id;

  GstBuffer *codec_data;
} VisualSampleEntry;

typedef struct
{
  guint32 fourcc;
  guint version;
  gint compression_id;
  guint sample_rate;
  guint channels;
  guint sample_size;
  guint bytes_per_packet;
  guint samples_per_packet;
  guint bytes_per_sample;
  guint bytes_per_frame;

  GstBuffer *codec_data;
} AudioSampleEntry;

void atom_trak_set_audio_type (AtomTRAK * trak, AtomsContext * context,
                               AudioSampleEntry * entry, guint32 scale,
                               AtomInfo * ext, gint sample_size);
void atom_trak_set_video_type (AtomTRAK * trak, AtomsContext * context,
                               VisualSampleEntry * entry, guint32 rate,
                               AtomInfo * ext);

AtomInfo *   build_codec_data_extension  (guint32 fourcc, const GstBuffer * codec_data);
AtomInfo *   build_mov_aac_extension     (AtomTRAK * trak, const GstBuffer * codec_data);
AtomInfo *   build_esds_extension        (AtomTRAK * trak, guint8 object_type,
                                          guint8 stream_type, const GstBuffer * codec_data);
AtomInfo *   build_jp2h_extension        (AtomTRAK * trak, gint width, gint height,
                                          guint32 fourcc);


/*
 * Meta tags functions
 */
void atom_moov_add_str_tag    (AtomMOOV *moov, guint32 fourcc, const gchar *value);
void atom_moov_add_uint_tag   (AtomMOOV *moov, guint32 fourcc, guint32 flags,
                               guint32 value);
void atom_moov_add_tag        (AtomMOOV *moov, guint32 fourcc, guint32 flags,
                               const guint8 * data, guint size);
void atom_moov_add_blob_tag   (AtomMOOV *moov, guint8 *data, guint size);

#endif /* __ATOMS_H__ */