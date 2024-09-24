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


#ifndef ID3LIB_HEADER_H
#define ID3LIB_HEADER_H

#include "id3_types.h"

#define MIN_ID3_TAGVERSION 2
#define MAX_ID3_TAGVERSION 4
#define ID3_TAGVERSION (3)
#define ID3_TAGREVISION (0)

struct ID3_HeaderInfo
{
  unsigned __int8 version;
  unsigned __int8 revision;
  unsigned __int8 frameIDBytes;
  unsigned __int8 frameSizeBytes;
  unsigned __int8 frameFlagsBytes;
  bool hasExtHeader;
  luint extHeaderBytes;
  bool setExpBit;
};

struct ID3_Quirks
{
	ID3_Quirks() : id3v2_4_itunes_bug(false) {}
	bool id3v2_4_itunes_bug;  // itunes doesn't write syncsafe values for 2.4
};

class ID3_Header
{
public:
  ID3_Header (void);

  void SetVersion(uchar ver, uchar rev);
  void SetDataSize(luint newSize);
  void SetFlags(luint newFlags);
	void SetQuirks(ID3_Quirks &_quirks)
	{
		quirks=_quirks;
	}

	ID3_Quirks &GetQuirks()
	{
		return quirks;
	}
	
  virtual luint Size(void) = 0;
  virtual luint Render(unsigned __int8 *buffer) = 0;

  // *** PRIVATE INTERNAL DATA - DO NOT USE *** PRIVATE INTERNAL DATA - DO NOT USE ***

protected:
  unsigned __int8 version; // which version?
  unsigned __int8 revision; // which revision?
  luint dataSize; // how big is the data?
  luint flags; // tag flags
  ID3_HeaderInfo *info; // the info about this version of the headers
	ID3_Quirks quirks;
};


ID3_HeaderInfo *ID3_LookupHeaderInfo (uchar ver, uchar rev);


#endif


