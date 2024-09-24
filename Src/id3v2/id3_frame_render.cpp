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


// improved/optimized/whatever 10/30/00 JF

#include <string.h>
#include <memory.h>
#include "id3_tag.h"
#include "id3_misc_support.h"


luint ID3_Frame::Render (uchar *buffer)
{
  luint bytesUsed = 0;

  ID3_FrameHeader header;
  ID3_FrameDef *info;
  luint flags;
  luint extras = 0;

  header.SetVersion (version, revision);
  bytesUsed += header.Size();

  // here is where we include things like
  // grouping IDs and crypto IDs
  if (strlen (encryptionID))
  {
    buffer[bytesUsed] = encryptionID[0];
    bytesUsed++, extras++;
  }

  if (strlen (groupingID))
  {
    buffer[bytesUsed] = groupingID[0];
    bytesUsed++, extras++;
  }

	/*
	if (version < 4)
	{
		// benski> set any UTF8 strings read from id3v2.4 to UTF-16,
	// so we can write with id3v2.3
		for (luint i = 0; i < numFields; i++)
		{
			if (fields[i]->flags & ID3FF_ADJUSTENC)
			{
				ID3_TextEnc enc= (ID3_TextEnc) Field(ID3FN_TEXTENC).Get();
	
				if (enc == ID3TE_UTF8 || enc == ID3TE_UTF16_BE)
					Field(ID3FN_TEXTENC).Set(ID3TE_UNICODE);
			}
		}
	}
	*/

  // this call is to tell the string fields
  // what they should be rendered/parsed as
  // (ASCII or Unicode)
  UpdateStringTypes();

  for (luint i = 0; i < numFields; i++)
  {
    fields[i]->SetVersion (version, revision);
    bytesUsed += fields[i]->Render (&buffer[bytesUsed]);
  }

  // if we can compress frames individually and we
  // have been asked to compress the frames
  /*
  if (compression && version >= 3)
  {
  luint newFrameSize;
  uchar *newTemp;
  bytesUsed -= header.Size();
  newFrameSize = bytesUsed + (bytesUsed / 10) + 12;
  if (newTemp = new uchar[newFrameSize])
  {
  if (compress (newTemp, &newFrameSize, &buffer[header.Size() + extras], bytesUsed - extras) == Z_OK)
  {
  // if the compression actually saves space
  if ((newFrameSize + sizeof (luint)) < bytesUsed)
  {
  luint posn;
  int i;

  posn = header.Size();
  extras += sizeof (luint);

  memcpy (&buffer[posn + sizeof (luint)], newTemp, newFrameSize);

  for (i = 0; i < sizeof (luint); i++)
  buffer[posn + i] = (uchar) ((bytesUsed >> ((sizeof (luint) - i - 1) * 8)) & 0xFF);

  bytesUsed = newFrameSize + sizeof (luint);
  didCompress = true;
  }
  }
  else
  ID3_THROW (ID3E_zlibError);

  bytesUsed += header.Size();

  delete[] newTemp;
  }
  else
  ID3_THROW (ID3E_NoMemory);
  }
  */

  // perform any encryption here
  /* if (strlen (encryptionID))
  {
  }*/

  // determine which flags need to be set
  if (info = ID3_FindFrameDef (frameID))
  {
    bool didCompress = false;
    flags = 0;

	if (version == 4)
	{
		if (info->tagDiscard)
      flags |= ID3FL_TAGALTER_2_4;

    if (info->fileDiscard)
      flags |= ID3FL_FILEALTER_2_4;

    if (didCompress)
      flags |= ID3FL_COMPRESSION_2_4;

    if (strlen (encryptionID))
      flags |= ID3FL_ENCRYPTION_2_4;

    if (strlen (groupingID))
      flags |= ID3FL_GROUPING_2_4;
	}
	else
	{
		if (info->tagDiscard)
      flags |= ID3FL_TAGALTER_2_3;

    if (info->fileDiscard)
      flags |= ID3FL_FILEALTER_2_3;

    if (didCompress)
      flags |= ID3FL_COMPRESSION_2_3;

    if (strlen (encryptionID))
      flags |= ID3FL_ENCRYPTION_2_3;

    if (strlen (groupingID))
      flags |= ID3FL_GROUPING_2_3;
	}
  }
  else
    ID3_THROW (ID3E_InvalidFrameID);

  header.SetFrameID(frameID);
  header.SetFlags(flags);
  header.SetDataSize(bytesUsed - header.Size());
  header.Render(buffer);
  hasChanged = false;

  return bytesUsed;
}


