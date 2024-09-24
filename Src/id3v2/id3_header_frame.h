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


#ifndef ID3LIB_HEADER_FRAME_H
#define ID3LIB_HEADER_FRAME_H


#include "id3_types.h"
#include "id3_header.h"
#include "id3_header_tag.h"
#include "id3_field.h"


#define ID3FL_TAGALTER_2_3 (1 << 15)
#define ID3FL_FILEALTER_2_3 (1 << 14)
#define ID3FL_SIGNED_2_3 (1 << 13)

#define ID3FL_COMPRESSION_2_3 (1 << 7)
#define ID3FL_ENCRYPTION_2_3 (1 << 6)
#define ID3FL_GROUPING_2_3 (1 << 5)

#define ID3FL_TAGALTER_2_4 (1 << 14)
#define ID3FL_FILEALTER_2_4 (1 << 13)
#define ID3FL_SIGNED_2_4 (1 << 12)

#define ID3FL_COMPRESSION_2_4 (1 << 3)
#define ID3FL_ENCRYPTION_2_4 (1 << 2)
#define ID3FL_GROUPING_2_4 (1 << 6)
#define ID3FL_UNSYNC_2_4 (1 << 1)

#define ID3FL_DATA_LENGTH_2_4 (1 << 0)

struct ID3_FrameAttr
{
	ID3_FrameAttr(void) : size(0), flags(0) { memset(&textID, 0, sizeof(textID)); }
  ~ID3_FrameAttr(void) {}

  char textID[5];
  luint size;

public:
	void ClearUnSync(int version);
	void SetFlags(luint _flags);
	bool HasCompression(int version);
	bool HasDataLength(int version);
	bool HasEncryption(int version);
	bool HasGrouping(int version);
	bool HasUnsync(int version);
private:
  luint flags;
};


class ID3_FrameHeader : public ID3_Header
{
public:
  virtual luint Size(void);
  void SetFrameID(ID3_FrameID id);
  luint GetFrameInfo(ID3_FrameAttr &attr, const uchar *buffer, size_t remSize);
  virtual luint Render(uchar *buffer);

  // *** PRIVATE INTERNAL DATA - DO NOT USE *** PRIVATE INTERNAL DATA - DO NOT USE ***

protected:
  ID3_FrameID frameID; // which frame are we the header for?
};


#endif


