/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender
   Copyright (C) 2006 Robert Kaye
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

----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>
#include <sys/ioctl.h>
#include <sys/param.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/storage/IOCDBlockStorageDevice.h>
#include <IOKit/storage/IOCDMediaBSDClient.h>

#include "discid/discid.h"
#include "discid/discid_private.h"
#include "unix.h"

#define MB_DEFAULT_DEVICE "1"	/* first disc drive (empty or not) */
#define TOC_BUFFER_LEN 2048


static int find_cd_block_devices(io_iterator_t *device_iterator)
{
	mach_port_t master_port;
	CFMutableDictionaryRef matching_dictionary;

	if (IOMasterPort(MACH_PORT_NULL, &master_port) != KERN_SUCCESS)
		return 0;

	matching_dictionary = IOServiceMatching(kIOCDBlockStorageDeviceClass);

	if (IOServiceGetMatchingServices(master_port, matching_dictionary,
					 device_iterator) != KERN_SUCCESS)
		return 0;
	else
		return 1;
}

static int get_device_from_entry(io_registry_entry_t entry,
				 char *device_path, int max_len)
{
	int return_value = 0;
	size_t dev_path_len;;
        CFStringRef cf_device_name;;
	*device_path = '\0';

        cf_device_name = IORegistryEntrySearchCFProperty(entry,
			kIOServicePlane, CFSTR(kIOBSDNameKey),
			kCFAllocatorDefault, kIORegistryIterateRecursively);

	if (cf_device_name) {
		strcpy(device_path, _PATH_DEV);
		/* Add "r" before the device name to use the raw disk node
		 * without the buffering cache. */
		strcat(device_path, "r");
		dev_path_len = strlen(device_path);
		if (CFStringGetCString(cf_device_name,
			   device_path + dev_path_len,
			   max_len - dev_path_len - 1,
			   kCFStringEncodingASCII))
			return_value = 1;

		CFRelease(cf_device_name);
	}
	return return_value;
}

static int get_device_from_number(int device_number,
				  char * buffer, int buffer_len) {
	int return_value = 0;
	int index = 0;
	io_iterator_t device_iterator;
	io_object_t device_object = IO_OBJECT_NULL;

	if (!find_cd_block_devices(&device_iterator))
		return 0;

	while (index < device_number
			&& (device_object = IOIteratorNext(device_iterator)))
		index++;

	if (index != device_number) {
		return_value = 0;
	} else {
		return_value = get_device_from_entry(device_object,
						     buffer, buffer_len);
	}

	IOObjectRelease(device_object);
	IOObjectRelease(device_iterator);

	return return_value;
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc)
{
    dk_cd_read_mcn_t cd_read_mcn;
    bzero(&cd_read_mcn, sizeof(cd_read_mcn));

    if(ioctl(fd, DKIOCCDREADMCN, &cd_read_mcn) == -1) {
        fprintf(stderr, "Warning: Unable to read the disc's media catalog number.\n");
    } else {
        strncpy( disc->mcn, cd_read_mcn.mcn, MCN_STR_LENGTH );
    }
}

void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc, int track)
{
    dk_cd_read_isrc_t	cd_read_isrc;
    bzero(&cd_read_isrc, sizeof(cd_read_isrc));
    cd_read_isrc.track = track;

    if(ioctl(fd, DKIOCCDREADISRC, &cd_read_isrc) == -1) {
        fprintf(stderr, "Warning: Unable to read the international standard recording code (ISRC) for track %i\n", track);
        return;
    } else {
        strncpy( disc->isrc[track], cd_read_isrc.isrc, ISRC_STR_LENGTH );
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

char *mb_disc_get_default_device_unportable(void) 
{
	return MB_DEFAULT_DEVICE;
}

int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *mb_toc) {
	dk_cd_read_toc_t toc;
	CDTOC *cdToc;
	mb_disc_toc_track *track;
	int i, numDesc;
	int track_num, min_track, max_track;

	memset(&toc, 0, sizeof(toc));
	toc.format = kCDTOCFormatTOC;
	toc.formatAsTime = 0;
	toc.buffer = (char *)malloc(TOC_BUFFER_LEN);
	toc.bufferLength = TOC_BUFFER_LEN;
	if (ioctl(fd, DKIOCCDREADTOC, &toc) < 0) {
		return 0;
	}
	if (toc.bufferLength < sizeof(CDTOC)) {
		return 0;
	}

	cdToc = (CDTOC *)toc.buffer;
	numDesc = CDTOCGetDescriptorCount(cdToc);
	min_track = -1;
	max_track = -1;
	for(i = 0; i < numDesc; i++) {
		CDTOCDescriptor *desc = &cdToc->descriptors[i];
		track = NULL;

		/* A2 is the code for the lead-out position in the lead-in */
		if (desc->point == 0xA2 && desc->adr == 1) {
			track = &mb_toc->tracks[0];
		}

		/* actual track data, (adr 2-3 are for MCN and ISRC data) */
		if (desc->point <= 99 && desc->adr == 1) {
			track_num = desc->point;
			track = &mb_toc->tracks[track_num];
			if (min_track < 0 || min_track > track_num) {
				min_track = track_num;
			}
			if (max_track < track_num) {
				max_track = track_num;
			}
		}

		if (track) {
			track->address = CDConvertMSFToLBA(desc->p);
			track->control = desc->control;
		}
	}

	mb_toc->first_track_num = min_track;
	mb_toc->last_track_num = max_track;

	free(toc.buffer);

	return 1;
}

int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *toc) {
	/* On Darwin the tracks are already filled along with the header */
	return 1;
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device,
			    unsigned int features) {
	int device_number;
	char device_name[MAXPATHLEN] = "\0";

	device_number = (int) strtol(device, NULL, 10);
	if (device_number > 0) {
		if (!get_device_from_number(device_number,
					    device_name, MAXPATHLEN)) {
			snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
				 "no disc in drive number: %d", device_number);
			return 0;
		} else {
			return mb_disc_unix_read(disc, device_name, features);
		}
	} else {
		return mb_disc_unix_read(disc, device, features);
	}
}
