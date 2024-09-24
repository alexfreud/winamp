/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2006 Matthias Friedrich
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

     $Id$

--------------------------------------------------------------------------- */
/*
 * For internal use only. This header file is not installed.
 */
#ifndef MUSICBRAINZ_DISC_ID_PRIVATE_H
#define MUSICBRAINZ_DISC_ID_PRIVATE_H

#include "discid/discid.h"

/* Length of toc string, "xxx+xxx" + 100 tracks 7 bytes each ("+xxxxxx")
 * The highest possible offset is 90 minutes * 60 seconds/minute * 75 frames/second = 405000.
 * That is 6 digits plus one plus sign = 7 characters per track.
 * So 3 + 3 (first and last) + 100*7 (disc length plus 99 tracks) = 706
 */
#define MB_TOC_STRING_LENGTH (3 + 3 + 100*7)

/* Length of a MusicBrainz DiscID in bytes (without a trailing '\0'-byte). */
#define MB_DISC_ID_LENGTH		32

/* Length of a FreeDB DiscID in bytes (without a trailing '\0'-byte). */
#define FREEDB_DISC_ID_LENGTH	8

/* The maximum permitted length for an error message (without the '\0'-byte). */
#define MB_ERROR_MSG_LENGTH		255

/* Length of url prefixes */
#define MB_URL_PREFIX_LENGTH    300

/* Maximum length of any url (including query string) */
#define MB_MAX_URL_LENGTH		(MB_URL_PREFIX_LENGTH + MB_DISC_ID_LENGTH + MB_TOC_STRING_LENGTH)

/* The URL that can be used for submitting DiscIDs (no parameters yet) */
#define MB_SUBMISSION_URL		"http://musicbrainz.org/cdtoc/attach"

/* The URL that can be used for retrieving XML for a CD */
#define MB_WEBSERVICE_URL		"http://musicbrainz.org/ws/1/release"

/* Maximum length of a Media Catalogue Number string */
#define MCN_STR_LENGTH		13

/* Maximum length of a ISRC code string */
#define ISRC_STR_LENGTH		12

/* Maximum disc length in frames/sectors
 * This is already not according to spec, but many players might still work
 * Spec is 79:59.75 = 360000 + lead-in + lead-out */
#define MAX_DISC_LENGTH		(90 * 60 * 75)

/*
 * This data structure represents an audio disc.
 *
 * We use fixed length strings here because that way the user doesn't have to
 * check for memory exhaustion conditions. As soon as the mb_disc object has
 * been created, all calls returning strings will be successful.
 */
typedef struct {
	int first_track_num;
	int last_track_num;
	int track_offsets[100];
	char id[MB_DISC_ID_LENGTH+1];
	char freedb_id[FREEDB_DISC_ID_LENGTH+1];
	char submission_url[MB_MAX_URL_LENGTH+1];
	char webservice_url[MB_MAX_URL_LENGTH+1];
	char toc_string[MB_TOC_STRING_LENGTH+1];
	char error_msg[MB_ERROR_MSG_LENGTH+1];
	char isrc[100][ISRC_STR_LENGTH+1];
	char mcn[MCN_STR_LENGTH+1];
	int success;
} mb_disc_private;

typedef struct {
	int control;
	int address;
} mb_disc_toc_track;

typedef struct {
	int first_track_num;
	int last_track_num;
	mb_disc_toc_track tracks[100];
} mb_disc_toc;

/*
 * This function has to be implemented once per operating system.
 *
 * The caller guarantees that both the disc and device parameters are
 * not NULL.
 *
 * Implementors have to set mb_disc_private's first_track_num, last_track_num,
 * and track_offsets attributes. If there is an error, the error_msg attribute
 * has to be set to a human-readable error message.
 *
 * On error, 0 is returned. On success, 1 is returned.
 */
LIBDISCID_INTERNAL int mb_disc_read_unportable(mb_disc_private *disc, const char *device, unsigned int features);


/*
 * This should return the name of the default/preferred CDROM/DVD device 
 * on this operating system. It has to be in a format usable for the second
 * parameter of mb_disc_read_unportable().
 */
LIBDISCID_INTERNAL char *mb_disc_get_default_device_unportable(void);

/*
 * This should return 1 if the feature is supported by the platform
 * and 0 if not.
 */
LIBDISCID_INTERNAL int mb_disc_has_feature_unportable(enum discid_feature feature);

/*
 * Load data to the mb_disc_private structure based on mb_disc_toc.
 *
 * On error, 0 is returned. On success, 1 is returned.
 */
LIBDISCID_INTERNAL int mb_disc_load_toc(mb_disc_private *disc, mb_disc_toc *toc);

#endif /* MUSICBRAINZ_DISC_ID_PRIVATE_H */
