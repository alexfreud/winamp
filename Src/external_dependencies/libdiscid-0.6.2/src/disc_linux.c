/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender
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
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <scsi/sg.h>


#include "discid/discid.h"
#include "discid/discid_private.h"
#include "unix.h"


/* timeout better shouldn't happen for scsi commands -> device is reset */
#define DEFAULT_TIMEOUT 30000	/* in ms */

#ifndef SG_MAX_SENSE
#define SG_MAX_SENSE 16
#endif

#define MB_DEFAULT_DEVICE "/dev/cdrom"
#define MAX_DEV_LEN 50

#if (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#	define THREAD_LOCAL __thread
#else
#	define THREAD_LOCAL
#endif

static THREAD_LOCAL char default_device[MAX_DEV_LEN] = "";


static int get_device(int number, char *device, int device_len) {
	FILE *proc_file;
	char *current_device;
	char *lineptr = NULL;
	char *saveptr = NULL;
	size_t bufflen;
	int i, count, counter;
	int return_value = 0;

	proc_file = fopen("/proc/sys/dev/cdrom/info", "r");
	if (proc_file != NULL) {
		/* skip to line containing device names */
		do {
			if (getline(&lineptr, &bufflen, proc_file) < 0) {
				return 0;
			}
		} while (strstr(lineptr, "drive name:") == NULL);

		/* count number of devices = number of tabs - 1*/
		count = -1;
		for (i = 0; i < strlen(lineptr); i++) {
			if (lineptr[i] == '\t') count++;
		}

		/* go through devices, they are in reverse order */
		current_device = strtok_r(lineptr, "\t", &saveptr);
		/* skip column title */
		current_device = strtok_r(NULL, "\t", &saveptr);
		counter = count;
		while (current_device != NULL && counter >= number) {
			if (counter == number) {
				snprintf(device, device_len,
					 "/dev/%s", current_device);
				return_value = 1;
			}
			/* go to next in list */
			current_device = strtok_r(NULL, "\t", &saveptr);
			counter--;
		}

		/* trim the trailing \n for the last entry = first device */
		if (return_value && device[strlen(device)-1] == '\n') {
			device[strlen(device)-1] = '\0';
		}
		free(lineptr);
		fclose(proc_file);
	}
	return return_value;
}

int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *toc) {
	struct cdrom_tochdr th;

	int ret = ioctl(fd, CDROMREADTOCHDR, &th);

	if ( ret < 0 )
		return 0; /* error */

	toc->first_track_num = th.cdth_trk0;
	toc->last_track_num = th.cdth_trk1;

	return 1;
}


int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *track) {
	struct cdrom_tocentry te;
	int ret;

	te.cdte_track = track_num;
	te.cdte_format = CDROM_LBA;

	ret = ioctl(fd, CDROMREADTOCENTRY, &te);
	assert( te.cdte_format == CDROM_LBA );

	if ( ret < 0 )
		return 0; /* error */

	track->address = te.cdte_addr.lba;
	track->control = te.cdte_ctrl;

	return 1;
}

char *mb_disc_get_default_device_unportable(void) {
	/* prefer the default device symlink to the internal names */
	if (mb_disc_unix_exists(MB_DEFAULT_DEVICE)) {
		return MB_DEFAULT_DEVICE;
	} else {
		if (get_device(1, default_device, MAX_DEV_LEN)) {
			return default_device;
		} else {
			return MB_DEFAULT_DEVICE;
		}
	}
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc) {
	struct cdrom_mcn mcn;
	memset(&mcn, 0, sizeof mcn);

	if(ioctl(fd, CDROM_GET_MCN, &mcn) == -1) {
		fprintf(stderr, "Warning: Unable to read the disc's media catalog number.\n");
	} else {
		strncpy( disc->mcn,
				(const char *)mcn.medium_catalog_number,
				MCN_STR_LENGTH );
	}
}

/* Send a scsi command and receive data. */
static int scsi_cmd(int fd, unsigned char *cmd, int cmd_len,
		    unsigned char *data, int data_len) {
	unsigned char sense_buffer[SG_MAX_SENSE]; /* for "error situations" */
	sg_io_hdr_t io_hdr;

	memset(&io_hdr, 0, sizeof io_hdr);

	assert(cmd_len <= 16);

	io_hdr.interface_id = 'S'; /* must always be 'S' (SCSI generic) */
	io_hdr.cmd_len = cmd_len;
	io_hdr.cmdp = cmd;
	io_hdr.timeout = DEFAULT_TIMEOUT; /* timeout in ms */
	io_hdr.sbp = sense_buffer;/* only used when status is CHECK_CONDITION */
	io_hdr.mx_sb_len = sizeof sense_buffer;
	io_hdr.flags = SG_FLAG_DIRECT_IO;

	io_hdr.dxferp = (void*)data;
	io_hdr.dxfer_len = data_len;
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

	if (ioctl(fd, SG_IO, &io_hdr) != 0) {
		return errno;
	} else {
		return io_hdr.status;	/* 0 = success */
	}
}

void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc, int track_num) {
	int i;
	unsigned char cmd[10];
	unsigned char data[24];
	char buffer[ISRC_STR_LENGTH+1];

	memset(cmd, 0, sizeof cmd);
	memset(data, 0, sizeof data);
	memset(buffer, 0, sizeof buffer);

	/* data read from the last appropriate sector encountered
	 * by a current or previous media access operation.
	 * The Logical Unit accesses the media when there is/was no access.
	 * TODO: force access at a specific block? -> no duplicate ISRCs?
	 */
	cmd[0] = 0x42;		/* READ SUB-CHANNEL */
	/* cmd[1] reserved / MSF bit (unused) */
	cmd[2] = 1 << 6;	/* 6th bit set (SUBQ) -> get sub-channel data */
	cmd[3] = 0x03;		/* get ISRC (ADR 3, Q sub-channel Mode-3) */
	/* 4+5 reserved */
	cmd[6] = track_num;
	/* cmd[7] = upper byte of the transfer length */
	cmd[8] = sizeof data;  /* transfer length in bytes (4 header, 20 data)*/
	/* cmd[9] = control byte */

	if (scsi_cmd(fd, cmd, sizeof cmd, data, sizeof data) != 0) {
		fprintf(stderr, "Warning: Cannot get ISRC code for track %d\n",
			track_num);
		return;
	}

	/* data[1:4] = sub-q channel data header (audio status, data length) */
	if (data[8] & (1 << 7)) { /* TCVAL is set -> ISRCs valid */
		for (i = 0; i < ISRC_STR_LENGTH; i++) {
			buffer[i] = data[9 + i];
		}
		buffer[ISRC_STR_LENGTH] = 0;
		strncpy(disc->isrc[track_num], buffer, ISRC_STR_LENGTH);
	}
	/* data[21:23] = zero, AFRAME, reserved */
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
			return 0;
		} else {
			return mb_disc_unix_read(disc, device_name, features);
		}
	} else {
		return mb_disc_unix_read(disc, device, features);
	}
}

/* EOF */
