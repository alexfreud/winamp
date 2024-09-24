/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2008 Patrick Hurrelmann
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__FreeBSD__)
#include <netinet/in.h> /* for ntohl() */
#else
#include <util.h> /* for getrawpartition() */
#endif

#include "discid/discid_private.h"
#include "unix.h"

#define MAX_DEV_LEN 15

static int get_device(int n, char* device_name, size_t device_name_length) {
#if !defined(__FreeBSD__) /* /dev/rcdNX, where X is the letter for the raw partition */
	snprintf(device_name, device_name_length, "/dev/rcd%d%c", n - 1, 'a' + getrawpartition());
#else /* on FreeBSD it's just /dev/cdN */
	snprintf(device_name, device_name_length, "/dev/cd%d", n - 1);
#endif
	return mb_disc_unix_exists(device_name);
}

int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *toc) {
	struct ioc_toc_header th;
	struct cd_toc_entry te[100];
	struct ioc_read_toc_entry rte;
	int i;

	memset(&th, 0, sizeof th);
	if (ioctl(fd, CDIOREADTOCHEADER, &th) < 0)
		return 0; /* error */

	toc->first_track_num = th.starting_track;
	toc->last_track_num  = th.ending_track;

	if (toc->last_track_num == 0)
		return 1; /* no entries to read */

	/* Read all the TOC entries in one icotl() call */

	memset(&te,  0, sizeof  te);
	memset(&rte, 0, sizeof rte);
	rte.address_format = CD_LBA_FORMAT;
	rte.data           = &te[0];
	rte.data_len       = sizeof te;
	rte.starting_track = toc->first_track_num;
	
	if (ioctl(fd, CDIOREADTOCENTRYS, &rte) < 0)
		return 0; /* error */

	for (i = toc->first_track_num; i <= toc->last_track_num; ++i) {
		assert(te[i - toc->first_track_num].track == i);
#if defined(__FreeBSD__) /* LBA address is in network byte order */
		toc->tracks[i].address = ntohl(te[i - toc->first_track_num].addr.lba);
#else
		toc->tracks[i].address = te[i - toc->first_track_num].addr.lba;
#endif
		toc->tracks[i].control = te[i - toc->first_track_num].control;
	}
	/* leadout - track number 170 (0xAA) */
	assert(te[i - toc->first_track_num].track == 0xAA);
#if defined(__FreeBSD__) /* LBA address is in network byte order */
	toc->tracks[0].address = ntohl(te[i - toc->first_track_num].addr.lba);
#else
	toc->tracks[0].address = te[i - toc->first_track_num].addr.lba;
#endif
	toc->tracks[0].control = te[i - toc->first_track_num].control;

	return 1;
}

int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *track) {
	/* All TOC entries were already read by mb_disc_unix_read_toc_header() */
	return 1;
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc) {
	struct cd_sub_channel_info sci;
	struct ioc_read_subchannel rsc;

	memset(&sci, 0, sizeof sci);
	memset(&rsc, 0, sizeof rsc);
	rsc.address_format = CD_LBA_FORMAT; /* not technically relevant */
	rsc.data_format    = CD_MEDIA_CATALOG;
	rsc.data_len       = sizeof sci;
	rsc.data           = &sci;

	if ( ioctl(fd, CDIOCREADSUBCHANNEL, &rsc) < 0 )
		perror ("Warning: Unable to read the disc's media catalog number");
	else {
		if (sci.what.media_catalog.mc_valid)
			strncpy( disc->mcn,
				 (const char *) sci.what.media_catalog.mc_number,
				 MCN_STR_LENGTH );
		else
			memset( disc->mcn, 0, MCN_STR_LENGTH );
	}
}

void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc, int track_num) {
	struct cd_sub_channel_info sci;
	struct ioc_read_subchannel rsc;

	memset(&sci, 0, sizeof sci);
	memset(&rsc, 0, sizeof rsc);
	rsc.address_format = CD_LBA_FORMAT; /* not technically relevant */
	rsc.data_format    = CD_TRACK_INFO;
	rsc.track          = track_num;
	rsc.data_len       = sizeof sci;
	rsc.data           = &sci;

	if ( ioctl(fd, CDIOCREADSUBCHANNEL, &rsc) < 0 )
		perror ("Warning: Unable to read track info (ISRC)");
	else {
		if (sci.what.track_info.ti_valid)
			strncpy( disc->isrc[track_num],
				 (const char *) sci.what.track_info.ti_number,
				 ISRC_STR_LENGTH );
		else
			memset( disc->isrc[track_num], 0, ISRC_STR_LENGTH );
	}
}

int mb_disc_has_feature_unportable(enum discid_feature feature) {
	switch(feature) {
		case DISCID_FEATURE_READ:
		case DISCID_FEATURE_MCN:
		case DISCID_FEATURE_ISRC:
			return 1;
		default:
			return 0;
	}
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device,
			    unsigned int features) {
	char device_name[MAX_DEV_LEN] = "";
	int device_number;

	device_number = (int) strtol(device, NULL, 10);

	if (device_number > 0) {
		if(!get_device(device_number, device_name, MAX_DEV_LEN)) {
			snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
				 "cannot find cd device with the number '%d'",
				 device_number);
			return 0; /* error */
		}
		device = device_name;
	}

	return mb_disc_unix_read(disc, device, features);
}

char *mb_disc_get_default_device_unportable(void) {
	static char result[MAX_DEV_LEN + 1];
	/* No error check here, so we always return the appropriate device for cd0 */
	get_device(1, result, sizeof result);
	return result;
}

/* EOF */
