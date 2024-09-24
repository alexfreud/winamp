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
#include <stdio.h>

#include "discid/discid.h"
#include "discid/discid_private.h"


char *mb_disc_get_default_device_unportable(void) {
	return "/dev/null";
}


int mb_disc_has_feature_unportable(enum discid_feature feature) {
	return 0;
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device, unsigned int features) {
	snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
		"disc reading not implemented on this platform");
	return 0;
}

/* EOF */
