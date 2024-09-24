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
#include "id3_header_tag.h"
#include "id3_error.h"

// Analyses a buffer to determine if we have a valid ID3v2 tag header.
// If so, return the number of bytes (starting _after_ the header) to
// read so we get all of the tag

lsint ID3_IsTagHeader(uchar header[ID3_TAGHEADERSIZE])
{
  if (!memcmp ("ID3", header, 3) && header[3] <= MAX_ID3_TAGVERSION)
  {
    int28 temp(&header[6]);
    return (int)temp.get();
  }

  return -1;
}


luint ID3_TagHeader::Size(void)
{
  luint bytesUsed = ID3_TAGHEADERSIZE;

  if (info && info->hasExtHeader)
    bytesUsed += info->extHeaderBytes + sizeof (luint);

  return bytesUsed;
}


luint ID3_TagHeader::Render(uchar *buffer)
{
  luint bytesUsed = 0;

  memcpy (&buffer[bytesUsed], (uchar *) ID3_TAGID, strlen (ID3_TAGID));
  bytesUsed += strlen (ID3_TAGID);

  buffer[bytesUsed++] = version;
  buffer[bytesUsed++] = revision;

  // do the automatic flags
  if (info->setExpBit)
    flags |= ID3HF_EXPERIMENTAL;

  if (info->hasExtHeader)
    flags |= ID3HF_EXTENDEDHEADER;

  // set the flags byte in the header
  buffer[bytesUsed++] = (uchar) (flags & 0xFF);

  int28 temp = (uint32_t)dataSize;

  for (luint i = 0; i < sizeof (luint); i++)
    buffer[bytesUsed++] = temp[i];

  // now we render the extended header
  if (info->hasExtHeader)
  {
    luint i;

    for (i = 0; i < sizeof (luint); i++)
      buffer[bytesUsed + i] = (uchar) ((info->extHeaderBytes >> ((sizeof (luint) - i - 1) * 8)) & 0xFF);

    bytesUsed += i;
  }

  bytesUsed = Size();

  return bytesUsed;
}


