/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2006 Matthias Friedrich
   Copyright (C) 2000 Robert Kaye
   Copyright (C) 1999 Marc E E van Woerkom
   
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

--------------------------------------------------------------------------- */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string.h>
#include <assert.h>
#include <limits.h>

#include "sha1.h"
#include "base64.h"

#include "discid/discid.h"
#include "discid/discid_private.h"


#define TRACK_NUM_IS_VALID(disc, i) \
	( i >= disc->first_track_num && i <= disc->last_track_num )


static void create_disc_id(mb_disc_private *d, char buf[]);
static void create_freedb_disc_id(mb_disc_private *d, char buf[]);
static char *create_toc_string(mb_disc_private *d, char *sep);
static void create_submission_url(mb_disc_private *d, char buf[]);
static void create_webservice_url(mb_disc_private *d, char buf[]);


/****************************************************************************
 *
 * Implementation of the public interface.
 *
 ****************************************************************************/

DiscId *discid_new() {
	/* initializes everything to zero */
	return calloc(1, sizeof(mb_disc_private));
}


void discid_free(DiscId *d) {
	free(d);
}


char *discid_get_error_msg(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);

	return disc->error_msg;
}


char *discid_get_id(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return NULL;

	if (strlen(disc->id) == 0)
		create_disc_id(disc, disc->id);

	return disc->id;
}


char *discid_get_freedb_id(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return NULL;

	if (strlen(disc->freedb_id) == 0)
		create_freedb_disc_id(disc, disc->freedb_id);

	return disc->freedb_id;
}

char *discid_get_toc_string(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert( disc != NULL );
	assert( disc->success );

	if ( ! disc->success )
		return NULL;

	if ( strlen(disc->toc_string) == 0 ) {
		char *toc = create_toc_string(disc, " ");
		if (toc) {
			memcpy(disc->toc_string, toc, strlen(toc) + 1);
			free(toc);
		}
	}

	return disc->toc_string;
}

char *discid_get_submission_url(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return NULL;

	if (strlen(disc->submission_url) == 0)
		create_submission_url(disc, disc->submission_url);

	return disc->submission_url;
}

char *discid_get_webservice_url(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return NULL;

	if (strlen(disc->webservice_url) == 0)
		create_webservice_url(disc, disc->webservice_url);

	return disc->webservice_url;
}

int discid_read(DiscId *d, const char *device) {
	return discid_read_sparse(d, device, UINT_MAX);
}

int discid_read_sparse(DiscId *d, const char *device, unsigned int features) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);

	if (device == NULL)
		device = discid_get_default_device();

	assert(device != NULL);

	/* Necessary, because the disc handle could have been used before. */
	memset(disc, 0, sizeof(mb_disc_private));

	/* pre-read the TOC to reduce "not-ready" problems
	 * See LIB-44 (issues with multi-session discs)
	 */
	if (!mb_disc_read_unportable(disc, device, DISCID_FEATURE_READ)) {
		return 0;
	}
	memset(disc, 0, sizeof(mb_disc_private));

	return disc->success = mb_disc_read_unportable(disc, device, features);
}

int discid_put(DiscId *d, int first, int last, int *offsets) {
	int i, disc_length;
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);

	/* Necessary, because the disc handle could have been used before. */
	memset(disc, 0, sizeof(mb_disc_private));

	/* extensive checking of given parameters */
	if (first > last || first < 1
			|| first > 99 || last < 1 || last > 99) {

		sprintf(disc->error_msg, "Illegal track limits");
		return 0;
	}
	if (offsets == NULL) {
		sprintf(disc->error_msg, "No offsets given");
		return 0;
	}
	disc_length = offsets[0];
	if (disc_length > MAX_DISC_LENGTH) {
		sprintf(disc->error_msg, "Disc too long");
		return 0;
	}
	for (i = 0; i <= last; i++) {
		if (offsets[i] > disc_length) {
			sprintf(disc->error_msg, "Invalid offset");
			return 0;
		}
		if (i > 1 && offsets[i-1] > offsets[i]) {
			sprintf(disc->error_msg, "Invalid order");
			return 0;
		}
	}

	disc->first_track_num = first;
	disc->last_track_num = last;

	memcpy(disc->track_offsets, offsets, sizeof(int) * (last+1));

	disc->success = 1;

	return 1;
}


char *discid_get_default_device(void) {
	return mb_disc_get_default_device_unportable();
}

int discid_get_first_track_num(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return -1;
	else
		return disc->first_track_num;
}


int discid_get_last_track_num(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return -1;
	else
		return disc->last_track_num;
}


int discid_get_sectors(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return -1;
	else
		return disc->track_offsets[0];
}


int discid_get_track_offset(DiscId *d, int i) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);
	assert(TRACK_NUM_IS_VALID(disc, i));

	if (!disc->success || !TRACK_NUM_IS_VALID(disc, i))
		return -1;
	else
		return disc->track_offsets[i];
}


int discid_get_track_length(DiscId *d, int i) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);
	assert(TRACK_NUM_IS_VALID(disc, i));

	if (!disc->success || !TRACK_NUM_IS_VALID(disc, i))
		return -1;
	else if (i < disc->last_track_num)
		return disc->track_offsets[i+1] - disc->track_offsets[i];
	else
		return disc->track_offsets[0] - disc->track_offsets[i];
}

char *discid_get_mcn(DiscId *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);

	if (!disc->success)
		return NULL;
	else
		return disc->mcn;
}

char* discid_get_track_isrc(DiscId *d, int i) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert(disc != NULL);
	assert(disc->success);
	assert(TRACK_NUM_IS_VALID(disc, i));

	if (!disc->success || i == 0 || !TRACK_NUM_IS_VALID(disc, i))
		return NULL;
	else
		return disc->isrc[i];
}

int discid_has_feature(enum discid_feature feature) {
	return mb_disc_has_feature_unportable(feature);
}

void discid_get_feature_list(char *features[DISCID_FEATURE_LENGTH]) {
	int i;

	/* for the code, the parameter is actually only a pointer */
	memset(features, 0, sizeof(char *) * DISCID_FEATURE_LENGTH);
	i = 0;

	if (discid_has_feature(DISCID_FEATURE_READ)) {
		features[i++] = DISCID_FEATURE_STR_READ;
	}
	if (discid_has_feature(DISCID_FEATURE_MCN)) {
		features[i++] = DISCID_FEATURE_STR_MCN;
	}
	if (discid_has_feature(DISCID_FEATURE_ISRC)) {
		features[i++] = DISCID_FEATURE_STR_ISRC;
	}

	return;
}

char *discid_get_version_string(void) {
#ifdef HAVE_CONFIG_H
	return PACKAGE_STRING;
#else
	return "";
#endif
}



/****************************************************************************
 *
 * Private utilities, not exported.
 *
 ****************************************************************************/

/*
 * Create a DiscID based on the TOC data found in the DiscId object.
 * The DiscID is placed in the provided string buffer.
 */
static void create_disc_id(mb_disc_private *d, char buf[]) {
	SHA_INFO	sha;
	unsigned char	digest[20], *base64;
	unsigned long	size;
	char		tmp[17]; /* for 8 hex digits (16 to avoid trouble) */
	int		i;

	assert(d != NULL);
	assert(d->success);

	sha_init(&sha);

	sprintf(tmp, "%02X", d->first_track_num);
	sha_update(&sha, (unsigned char *) tmp, strlen(tmp));

	sprintf(tmp, "%02X", d->last_track_num);
	sha_update(&sha, (unsigned char *) tmp, strlen(tmp));

	for (i = 0; i < 100; i++) {
		sprintf(tmp, "%08X", d->track_offsets[i]);
		sha_update(&sha, (unsigned char *) tmp, strlen(tmp));
	}

	sha_final(digest, &sha);

	base64 = rfc822_binary(digest, sizeof(digest), &size);

	memcpy(buf, base64, size);
	buf[size] = '\0';

	free(base64);
}


/*
 * Create a FreeDB DiscID based on the TOC data found in the DiscId object.
 * The DiscID is placed in the provided string buffer.
 */
static void create_freedb_disc_id(mb_disc_private *d, char buf[]) {
	int i, n, m, t;

	assert(d != NULL);
	assert(d->success);

	n = 0;
	for (i = 0; i < d->last_track_num; i++) {
		m = d->track_offsets[i + 1] / 75;
		while (m > 0) {
			n += m % 10;
			m /= 10;
		}
	}
	t = d->track_offsets[0] / 75 - d->track_offsets[1] / 75;
	sprintf(buf, "%08x", ((n % 0xff) << 24 | t << 8 | d->last_track_num));
}

/*
 * Create a string based on the TOC data found in the mb_disc_private
 * object. The returned string is allocated, caller has to free() it.
 * On failure, it returns NULL.
 *
 * Format is:
 * [1st track num][sep][last track num][sep][length in sectors][sep][1st track offset][sep]...
 */
static char *create_toc_string(mb_disc_private *d, char *sep) {
	char tmp[16];
	char *toc;
	int i, size;

	assert( d != NULL );

	/* number of tracks */
	size = 1 + d->last_track_num - d->first_track_num;
	/* first&last track num and total length */
	size += 3;
	/* number + separator */
	size *= (6 + strlen(sep));
	/* nul */
	size++;

	toc = calloc(size, sizeof(char));
	if (!toc) return NULL;

	sprintf(toc, "%d%s%d%s%d",
			d->first_track_num,
			sep,
			d->last_track_num,
			sep,
			d->track_offsets[0]);

	for (i = d->first_track_num; i <= d->last_track_num; i++) {
		sprintf(tmp, "%s%d", sep, d->track_offsets[i]);
		strcat(toc, tmp);
	}

	return toc;
}

/* Append &toc=... to buf, calling  create_toc_string() */
static void cat_toc_param(mb_disc_private *d, char *buf) {
	char *toc = create_toc_string(d, "+");
	if (toc) {
		strcat(buf, "&toc=");
		strcat(buf, toc);
		free(toc);
	}
}

/*
 * Create a submission URL based on the TOC data found in the mb_disc_private
 * object. The URL is placed in the provided string buffer.
 */
static void create_submission_url(mb_disc_private *d, char buf[]) {
	char tmp[16];

	assert(d != NULL);
	assert(d->success);

	strcpy(buf, MB_SUBMISSION_URL);

	strcat(buf, "?id=");
	strcat(buf, discid_get_id((DiscId *) d));

	sprintf(tmp, "&tracks=%d", d->last_track_num);
	strcat(buf, tmp);

	cat_toc_param(d, buf);
}

/*
 * Create a web service URL based on the TOC data found in the mb_disc_private
 * object. The URL is placed in the provided string buffer.
 */
static void create_webservice_url(mb_disc_private *d, char buf[]) {
	assert(d != NULL);
	assert(d->success);

	strcpy(buf, MB_WEBSERVICE_URL);

	strcat(buf, "?type=xml&discid=");
	strcat(buf, discid_get_id((DiscId *) d));

	cat_toc_param(d, buf);
}

/* EOF */
