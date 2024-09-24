/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender

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

#include "discid/discid_private.h"


/*
 * required functions
 * ------------------
 */

/*
 * Read the TOC header from disc
 *
 * THIS FUNCTION HAS TO BE IMPLEMENTED FOR THE PLATFORM
 */
LIBDISCID_INTERNAL int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *toc);

/*
 * Read a TOC entry for a certain track from disc
 *
 * THIS FUNCTION HAS TO BE IMPLEMENTED FOR THE PLATFORM
 */
LIBDISCID_INTERNAL int mb_disc_unix_read_toc_entry(int fd, int track_num,
						   mb_disc_toc_track *track);

/*
 * Read the MCN from the disc
 *
 * THIS FUNCTION HAS TO BE IMPLEMENTED FOR THE PLATFORM
 */
LIBDISCID_INTERNAL void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc);

/*
 * Read the ISRC for a certain track from disc
 *
 * THIS FUNCTION HAS TO BE IMPLEMENTED FOR THE PLATFORM
 */
LIBDISCID_INTERNAL void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc,
					       int track_num);


/*
 * provided functions
 * ------------------
 */

/*
 * This function is implemented in unix.c and can be used
 * for most platforms to implement mb_disc_read_unportable
 * after the above functions are implemented on the platform.
 * Returns 1 on success and 0 on failure.
 */
LIBDISCID_INTERNAL int mb_disc_unix_read(mb_disc_private *disc,
				const char *device, unsigned int features);

/*
 * This function is implemented in unix.c and can be used
 * after the above functions are implemented on the platform.
 * This uses mb_disc_unix_read_toc_* and adds some error checking.
 * Returns 1 on success and 0 on failure.
 */
LIBDISCID_INTERNAL int mb_disc_unix_read_toc(int fd, mb_disc_private *disc,
					     mb_disc_toc *toc);

/*
 * utility function to find an existing device from a candidate list
 */
LIBDISCID_INTERNAL int mb_disc_unix_exists(const char *device);

/*
 * utility function to find an existing device from a candidate list
 */
LIBDISCID_INTERNAL char *mb_disc_unix_find_device(char *candidates[],
						  int num_candidates);

/*
 * utility function to try opening the device with open()
 * returns a non-negative file descriptor on success.
 * On failure a negative integer is returned and error_msg filled
 * with an appropriate string.
 */
LIBDISCID_INTERNAL int mb_disc_unix_open(mb_disc_private *disc,
					 const char *device);
