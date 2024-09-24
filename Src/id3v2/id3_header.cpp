// The authors have released ID3Lib as Public Domain (PD) and claim no copyright,
// patent or other intellectual property protection in this work. This means that
// it may be modified, redistributed and used in commercial and non-commercial
// software and hardware without restrictions. ID3Lib is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//
// The ID3Lib authors encourage improvements and optimisations to be sent to the
// ID3Lib coordinator, currently Dirk Mahoney (dirk@id3.org). Approved
// submissions may be altered, and will be included and released under these terms.
//
// Mon Nov 23 18:34:01 1998


#include <string.h>
#include <memory.h>
#include "id3_header.h"
#include "id3_error.h"


static ID3_HeaderInfo ID3_VersionInfo[4][3] =
{
	// SIZEOF SIZEOF SIZEOF EXT EXT EXPERIM
	// VER REV FRID FRSZ FRFL HEADER SIZE BIT
	{
		{ 2, 0, 3, 3, 0, false, 0, false },
		{ 2, 1, 3, 3, 0, true, 8, true },
		{0,},
	},
	{
		{ 3, 0, 4, 4, 2, false, 10, false },
		{0,},
	},
	{
		{ 4, 0, 4, 4, 2, false, 10, false },
		{0,},
	},
	{ 0 }
};

ID3_HeaderInfo *ID3_LookupHeaderInfo(uchar ver, uchar rev)
{
	ID3_HeaderInfo *info = NULL;
	size_t v=0,r=0;

	while (ID3_VersionInfo[v][0].version != 0)
	{
		r=0;
		if (ID3_VersionInfo[v][0].version == ver)
		{
			while (ID3_VersionInfo[v][r].version != 0)
			{
				if (ID3_VersionInfo[v][r].revision == rev)
				{
					goto found;
				}
				else
					r++;
			}
			// later revision.  still backwards compat so we'll stick with it.
			r--;
			goto found;
		}
		else
			v++;
	}
found:

	if (ID3_VersionInfo[v][r].version != 0)
		info = &ID3_VersionInfo[v][r];

	return info;
}

ID3_Header::ID3_Header(void)
{
	SetVersion(ID3_TAGVERSION, ID3_TAGREVISION);
	dataSize = 0;
	flags = 0;
}

void ID3_Header::SetVersion(uchar ver, uchar rev)
{
	version = ver;
	revision = rev;
	info = ID3_LookupHeaderInfo(version, revision);
}

void ID3_Header::SetDataSize(luint newSize)
{
	dataSize = newSize;
}

void ID3_Header::SetFlags(luint newFlags)
{
	flags = newFlags;
}


