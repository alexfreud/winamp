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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "discid/discid_private.h"
#include "unix.h"


int mb_disc_unix_exists(const char *device) {
	int fd;
	fd = open(device, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		/* we only check for existance, access should fail later on */
		if (errno == ENOENT) {
			return 0;
		} else {
			return 1;
		}
	} else {
		close(fd);
		return 1;
	}
}

char *mb_disc_unix_find_device(char *candidates[], int num_candidates) {
	int i;

	for (i = 0; i < num_candidates; i++) {
		if (mb_disc_unix_exists(candidates[i])) {
			return candidates[i];
		}
	}
	/* use the first name for the error message later on */
	return candidates[0];
}

int mb_disc_unix_open(mb_disc_private *disc, const char *device) {
	int fd;

	fd = open(device, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			 "cannot open device `%s'", device);
	}
	/* fd < 0 check needs to be made by caller */
	return fd;
}

int mb_disc_unix_read_toc(int fd, mb_disc_private *disc, mb_disc_toc *toc) {
	int i;

	/* Find the numbers of the first track (usually 1) and the last track. */
	if ( !mb_disc_unix_read_toc_header(fd, toc) ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"cannot read table of contents");
		return 0;
	}

	/* basic error checking */
	if ( toc->last_track_num == 0 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"this disc has no tracks");
		return 0;
	}

	/*
	 * Read the TOC entry for every track.
	 */
	for (i = toc->first_track_num; i <= toc->last_track_num; i++) {
		if ( !mb_disc_unix_read_toc_entry(fd, i, &toc->tracks[i]) ) {
			snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
				 "cannot read TOC entry for track %d", i);
			return 0;
		}
	}
	if ( !mb_disc_unix_read_toc_entry(fd, 0xAA, &toc->tracks[0]) ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			 "cannot read TOC entry for lead-out");
		return 0;
	}

	return 1;
}

int mb_disc_unix_read(mb_disc_private *disc, const char *device,
		      unsigned int features) {
	mb_disc_toc toc;
	int fd;
	int i;

	fd = mb_disc_unix_open(disc, device);
	if (fd < 0)
		return 0;


	if ( !mb_disc_unix_read_toc(fd, disc, &toc) ) {
		close(fd);
		return 0;
	}

	if ( !mb_disc_load_toc(disc, &toc) ) {
		close(fd);
		return 0;
	}

	/* Read in the media catalog number */
	if (features & DISCID_FEATURE_MCN
		&& mb_disc_has_feature_unportable(DISCID_FEATURE_MCN)) {
		mb_disc_unix_read_mcn(fd, disc);
	}

	/* Read the ISRC for the track */
	if (features & DISCID_FEATURE_ISRC
		&& mb_disc_has_feature_unportable(DISCID_FEATURE_ISRC)) {
		for (i = disc->first_track_num; i <= disc->last_track_num; i++) {
			mb_disc_unix_read_isrc(fd, disc, i);
		}
	}

	close(fd);

	return 1;
}
/* EOF */
