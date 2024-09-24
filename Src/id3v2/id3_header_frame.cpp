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
#include "id3_header_frame.h"
#include "id3_error.h"

bool ID3_FrameAttr::HasDataLength(int version)
{
	if (version == 4)
		return !!(flags & ID3FL_DATA_LENGTH_2_4);
	else
		return 0;
}

bool ID3_FrameAttr::HasCompression(int version)
{
	if (version == 4)
		return !!(flags & ID3FL_COMPRESSION_2_4);
	else
		return !!(flags & ID3FL_COMPRESSION_2_3);
}

bool ID3_FrameAttr::HasEncryption(int version)
{
	if (version == 4)
		return !!(flags & ID3FL_ENCRYPTION_2_4);
	else
		return !!(flags & ID3FL_ENCRYPTION_2_3);
}

bool ID3_FrameAttr::HasGrouping(int version)
{
	if (version == 4)
		return !!(flags & ID3FL_GROUPING_2_4);
	else
		return !!(flags & ID3FL_GROUPING_2_3);
}

bool ID3_FrameAttr::HasUnsync(int version)
{
	if (version == 4)
		return !!(flags & ID3FL_UNSYNC_2_4);
	else
		return false;
}

void ID3_FrameAttr::ClearUnSync(int version)
{
	if (version == 4)
		flags&= ~ID3FL_UNSYNC_2_4;
}

void ID3_FrameAttr::SetFlags(luint _flags)
{
	flags = _flags;
}

void ID3_FrameHeader::SetFrameID (ID3_FrameID id)
{
	frameID = id;
}

luint ID3_FrameHeader::Size(void)
{
	if (!info)
		return 0;
  return info->frameIDBytes + info->frameSizeBytes + info->frameFlagsBytes;
}

// TODO: benski> we should make a return value of 0 mean 'error'
luint ID3_FrameHeader::GetFrameInfo(ID3_FrameAttr &attr, const uchar *buffer, size_t remSize)
{
	luint posn = 0;
	luint i = 0, bpos = 4;

	// verify that the text is all between A-Z and 0-9 (for TALB and TIT2, etc)
	for (i = 0; i < 4; i++)
	{
		if (!((buffer[i] >= '0' && buffer[i] <= '9') || (buffer[i] >= 'A' && buffer[i] <= 'Z')))
		{
			// TODO: benski> return an error here
			// this helps us to get ID3v2.2 PIC frames without
			// breaking others since it's not null-terminated!
			bpos = i;
		}
	}

	memcpy(attr.textID, (char *) buffer, (bpos > 0 && bpos <= 4 ? bpos : 4));
	if (bpos == 3) attr.textID[3] = 0;
	attr.textID[4] = 0;

	posn += info->frameIDBytes;

	attr.size = 0;

	for (i = 0; i < info->frameSizeBytes; i++)
		attr.size |= buffer[posn + i] << ((info->frameSizeBytes - 1 - i) * 8); 

	if (version == 4) // 2.4 uses syncsafe sizes
	{
		// many programs write non-syncsafe sizes anyway (iTunes is the biggest culprit)
		// so we'll try to detect it.  unfortunately this isn't foolproof
		int mask = attr.size & 0x80808080;
		if (!quirks.id3v2_4_itunes_bug  // make sure we've havn't previously identified that this tag has a problem
			&& mask == 0) // if none of the 'reserved' bits are set
		{
			attr.size = int28().setFromFile(attr.size).get(); // convert to syncsafe value
		}
		else
		{
			// benski> cut for now. this can't be trusted because it applies on subsequent re-parses but not anything before this
			// quirks.id3v2_4_itunes_bug = true;  // mark that the tag has a problem
		}
		// TODO: it'd be nice to look ahead into the buffer and make sure that our calculated size
		// lets us land on the start of a new frame
		// although that isn't foolproof either (non-standard fields, etc)
	}
	posn += info->frameSizeBytes;

	luint flags=0;
	flags = 0;

	for (i = 0; i < info->frameFlagsBytes; i++)
		flags |= buffer[ posn + i ] << ((info->frameFlagsBytes - 1 - i) * 8);

	attr.SetFlags(flags);

	posn += info->frameFlagsBytes;

	return posn;
}

luint ID3_FrameHeader::Render(uchar *buffer)
{
	luint bytesUsed = 0;
	ID3_FrameDef *frameDef = NULL;
	char *textID = NULL;
	luint i;

	if (frameDef = ID3_FindFrameDef(frameID))
	{
		if (info->frameIDBytes < strlen (frameDef->longTextID))
			textID = frameDef->shortTextID;
		else
			textID = frameDef->longTextID;
	}
	else
		ID3_THROW (ID3E_InvalidFrameID);

	memcpy (&buffer[ bytesUsed ], (uchar *) textID, info->frameIDBytes);
	bytesUsed += info->frameIDBytes;

	for (i = 0; i < info->frameSizeBytes; i++)
	{
		if (version==4) // 2.4 uses syncsafe sizes
			buffer[ bytesUsed + i ] = (uchar) ((dataSize >> ((info->frameSizeBytes - i - 1) * 7)) & 0x7F);
		else
			buffer[ bytesUsed + i ] = (uchar) ((dataSize >> ((info->frameSizeBytes - i - 1) * 8)) & 0xFF);
	}

	bytesUsed += info->frameSizeBytes;

	for (i = 0; i < info->frameFlagsBytes; i++)
		buffer[ bytesUsed + i ] = (uchar) ((flags >> ((info->frameFlagsBytes - i - 1) * 8)) & 0xFF);

	bytesUsed += info->frameFlagsBytes;

	return bytesUsed;
}