// The authors have released ID3Lib as Public Domain(PD) and claim no copyright,
// patent or other intellectual property protection in this work. This means that
// it may be modified, redistributed and used in commercial and non-commercial
// software and hardware without restrictions. ID3Lib is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
//
// The ID3Lib authors encourage improvements and optimisations to be sent to the
// ID3Lib coordinator, currently Dirk Mahoney(dirk@id3.org). Approved
// submissions may be altered, and will be included and released under these terms.
//
// Mon Nov 23 18:34:01 1998


#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "id3_tag.h"
#include "../Plugins/Input/in_mp3/LAMEinfo.h"
#include <assert.h>
#include "zlib.h"

ID3_Elem *ID3_Tag::GetLastElem(ID3_Elem *list)
{
	ID3_Elem *last = list;

	while (last && last->next)
		last = last->next;

	return last;
}

void ID3_Tag::AddBinary(const uchar *buffer, luint size)
{
	uchar *newBin;

	if (size > 0)
	{
		if (newBin = (uchar*)calloc(size, sizeof(uchar)))
		{
			ID3_Elem * elem;

			memcpy(newBin, buffer, size);

			if (elem = (ID3_Elem*)calloc(1, sizeof(ID3_Elem)))
			{
				elem->next = NULL;
				elem->frame = NULL;
				elem->binary = newBin;
				elem->binarySize = size;
				elem->tagOwns = true;

				ID3_Elem * lastElem = GetLastElem(binaryList);

				if (lastElem)
					lastElem->next = elem;
				else
					binaryList = elem;
			}
			else
			{
				free(newBin);
				ID3_THROW(ID3E_NoMemory);
			}
		}
		else
			ID3_THROW(ID3E_NoMemory);
	}
	else
		ID3_THROW(ID3E_NoData);

	return ;
}

void ID3_Tag::ExpandBinaries(const uchar *buffer, luint size)
{
	ID3_FrameAttr attr;
	ID3_FrameHeader frHeader;
	luint posn = 0;

	while (posn < (size - 6) && buffer[posn] != 0)
	{
		//luint newBinSize;

		frHeader.SetVersion(version, revision);
		frHeader.SetQuirks(quirks);

		posn += frHeader.GetFrameInfo(attr, &buffer[posn], size);
		//newBinSize = frHeader.Size() + attr.size;

		if ((posn + attr.size) > (size))
		{
			ID3_THROW(ID3E_NoMemory);
			return ;
		}

		quirks=frHeader.GetQuirks();

		// firstly, let's check to see if we are parsing a CDM. If so,
		// let's expanded it out now.
		if (strcmp(attr.textID, "CDM") == 0)
		{
			// if the method was zlib
			if (buffer[posn] == 'z')
			{
				unsigned long expandedSize = 0;
				uchar *expBin;

				expandedSize |= buffer[posn + 1] << 24;
				expandedSize |= buffer[posn + 2] << 16;
				expandedSize |= buffer[posn + 3] << 8;
				expandedSize |= buffer[posn + 4];

				if (expBin = (uchar*)calloc(expandedSize, sizeof(uchar)))
				{
					uncompress(expBin, &expandedSize, &buffer[posn + 1 + sizeof(luint)], (uLong)(attr.size - sizeof(luint) - 1));

					ExpandBinaries(expBin, expandedSize);

					posn += attr.size;

					free(expBin);
				}
			}
		}
		else
		{
			AddBinary(&buffer[posn - frHeader.Size()], attr.size + frHeader.Size());
			posn += attr.size;
		}
	}

	return ;
}

void ID3_Tag::ProcessBinaries(ID3_FrameID whichFrame, bool attach)
{
	ID3_FrameAttr attr;
	ID3_FrameHeader frHeader;
	ID3_Elem *cur = binaryList;

	frHeader.SetVersion(version, revision);
	frHeader.SetQuirks(quirks);
	while (cur && cur->binary)
	{
		ID3_FrameID id;
		ID3_Frame *frame;
		luint posn;
		luint extras = 0;
		unsigned long expandedSize = 0;
		//uchar groupingID = 0;
		//uchar encryptionID = 0;

		posn = frHeader.GetFrameInfo(attr, cur->binary, cur->binarySize);
		assert(attr.size + frHeader.Size() == cur->binarySize);
		uint32_t data_length = 0;
		if (attr.HasDataLength(version))
		{
			data_length = 0;

			for (int i = 0; i < 4; i++)
				data_length |= cur->binary[posn + i] << ((data_length - 1 - i) * 8); 

			// size of Re-sync'd data. we'll just ignore for now
			data_length = int28().setFromFile(data_length).get();
			//attr.size = data_length;
			posn+=4;
			extras+=4;
		}

		if (attr.HasCompression(version))
		{
			if (attr.HasDataLength(version))
				expandedSize = data_length;
			else
			{
				expandedSize |= cur->binary[posn + 0] << 24;
				expandedSize |= cur->binary[posn + 1] << 16;
				expandedSize |= cur->binary[posn + 2] << 8;
				expandedSize |= cur->binary[posn + 3];

				extras += sizeof(luint);
				posn+=4;
			}
		}

		if (attr.HasEncryption(version))
		{
			//encryptionID = cur->binary[posn];
			posn++, extras++;
		}

		if (attr.HasGrouping(version))
		{
			//groupingID = cur->binary[posn];
			posn++, extras++;
		}

		id = ID3_FindFrameID(attr.textID);

		if ((id == whichFrame || whichFrame == ID3FID_NOFRAME) && id != ID3FID_NOFRAME)
		{
			uchar *expBin;
			bool useExpBin = false;
			luint frameSize= attr.size - extras;

			if (attr.HasUnsync(version)  // if the per-frame unsync flag is set
				&& !globalUnsync) // but make sure we didn't already unsync the whole file
			{
				frameSize=ReSync(&cur->binary[posn], attr.size - 10) + extras;
			}

			if (attr.HasCompression(version))
			{
				if (!(expBin = (uchar*)calloc(expandedSize, sizeof(uchar))))
					ID3_THROW(ID3E_NoMemory);

				if (expBin)
				{
					int x = uncompress(expBin, &expandedSize, &cur->binary[posn], frameSize);
					if (x == 0/*Z_OK*/)
					{
						useExpBin = true;
					}
					else
					{
						free(expBin);
						expBin=0;
					}
				}
				
			}

			if (!(frame = new ID3_Frame))
				ID3_THROW(ID3E_NoMemory);

			ID3_Elem *elem;

			frame->SetVersion(version, revision);
			frame->SetID(id);

			if (useExpBin)
			{
				frame->Parse(expBin, expandedSize);
				free(expBin);
			}
			else
				frame->Parse(&cur->binary[posn], frameSize);

			// here is where we call a special handler
			// for this frame type if one is specified
			// in the frame definition
			{
				ID3_FrameDef *frameInfo;

				frameInfo = ID3_FindFrameDef(id);

				if (frameInfo && frameInfo->parseHandler)
					attach = frameInfo->parseHandler(frame);
			}

			// if, after all is said and done, we
			// are still supposed to attach our
			// newly parsed frame to the tag, do so
			if (attach)
			{
				if (!(elem = (ID3_Elem *)calloc(1, sizeof(ID3_Elem))))
					ID3_THROW(ID3E_NoMemory);

				elem->next = NULL;
				elem->frame = frame;
				elem->binary = NULL;
				elem->binarySize=0;
				elem->tagOwns = true;

				ID3_Elem * lastElem = GetLastElem(frameList);

				if (lastElem)
					lastElem->next = elem;
				else
					frameList = elem;
			}
			else
			{
				// if not, delete it
				delete frame;
			}

			ID3_Elem *temp = cur;
			cur = cur->next;

			RemoveFromList(temp, &binaryList);
		}
		else
		{
			cur = cur->next;
		}
	}

	return ;
}


void ID3_Tag::Parse(const uchar header[ID3_TAGHEADERSIZE], const uchar *buffer)
{
	luint tagSize = 0;
	int28 temp = &header[6];
	luint posn = 0;
	uchar prevVer = version;
	uchar prevRev = revision;

	Clear();

	tagSize = temp.get();

	SetVersion(header[3], header[4]);

	// make sure we understand this version
	if (version > MAX_ID3_TAGVERSION || version < MIN_ID3_TAGVERSION)
		return; 

	globalUnsync = !!(header[5] & ID3HF_UNSYNC);
	if (globalUnsync)
	{
		assert(syncBuffer==0);
		syncBuffer = (uchar *)calloc(tagSize, sizeof(uchar));
		memcpy(syncBuffer, buffer, tagSize);
		buffer=syncBuffer;
		tagSize = ReSync(syncBuffer, tagSize);
	}


	// okay, if we are 2.01, then let's skip over the
	// extended header for now because I am lazy
	if (version == 2 && revision == 1)
		if (header[5] & ID3HF_EXTENDEDHEADER)
		{
			luint extSize = 0;

			extSize |= buffer[0] << 24;
			extSize |= buffer[1] << 16;
			extSize |= buffer[2] << 8;
			extSize |= buffer[3] << 0;

			posn = extSize + sizeof(luint);
		}

	// okay, if we are 3.00, then let's actually
	// parse the extended header(for now, we skip
	// it because we are lazy)
	if (version == 3 && revision == 0)
	{
		// according to id3v2.3 specs section 3.1
		// "All the other flags should be cleared. If one of these undefined flags are set that might mean that the tag is not readable for a parser that does not know the flags function."
		if (header[5] & ~ID3HF_2_3_MASK)
		{
			// TODO: benski> is this the best way to show an error?
			Clear();
			return;
		}

		if (header[5] & ID3HF_EXTENDEDHEADER)
		{
			luint extSize = 0;

			extSize |= buffer[0] << 24;
			extSize |= buffer[1] << 16;
			extSize |= buffer[2] << 8;
			extSize |= buffer[3] << 0;

			posn = extSize + sizeof(luint);
		}
	}

	// id3v2.4
	if (version == 4 && revision == 0)
	{
		// according to id3v2.4 specs section 3.1
		// "All the other flags MUST be cleared. If one of these undefined flags
		// are set, the tag might not be readable for a parser that does not
    // know the flags function."
		if (header[5] & ~ID3HF_2_4_MASK)
		{
			// TODO: benski> is this the best way to show an error?
			Clear();
			return;
		}

		if (header[5] & ID3HF_EXTENDEDHEADER)
		{
			luint extSize = 0;

			extSize |= buffer[0] << 24;
			extSize |= buffer[1] << 16;
			extSize |= buffer[2] << 8;
			extSize |= buffer[3] << 0;

			posn = extSize + sizeof(luint);
		}
	}

	if (posn >= tagSize) 
	{
		// TODO: benski> is this the best way to show an error?
		Clear();
		return;
	}

	// this call will convert the binary data block(tag)
	// into a linked list of binary frames
	ExpandBinaries(&buffer[posn], tagSize-posn);

	// let's parse the CRYPTO frames
	// the 'false' parameter means "don't
	// attach the frame to the tag when
	// processed". This is because we have
	// installed a parsing handler for the
	// crypto reg frame. This is a default
	// parameter - if the frame type has a
	// custom parsing handler, that handler
	// will tell ID3Lib whether to attach
	// or not.
	ProcessBinaries(ID3FID_CRYPTOREG, false);

	// let's parse the GROUPING frames
	// the 'false' parameter means "don't
	// attach the frame to the tag when
	// processed". This is because we have
	// installed a parsing handler for the
	// crypto reg frame. This is a default
	// parameter - if the frame type has a
	// custom parsing handler, that handler
	// will tell ID3Lib whether to attach
	// or not.
	ProcessBinaries(ID3FID_GROUPINGREG, false);

	// let's parse the rest of the binaries
	ProcessBinaries();

	// upgrade older tags to our preferred revision
	if (prevVer > version || (prevVer == version && prevRev > revision))
		SetVersion(prevVer, prevRev);

	// set the flag which says that the tag hasn't changed
	hasChanged = false;

	return ;
}

