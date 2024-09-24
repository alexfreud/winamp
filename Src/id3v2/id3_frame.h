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

#pragma once

#include "id3_types.h"
#include "id3_field.h"
#include "id3_header_frame.h"


class ID3_Frame
{
public:
  ID3_Frame(ID3_FrameID id = ID3FID_NOFRAME);
  ~ID3_Frame(void);

  void Clear(void);
  void SetID(ID3_FrameID id);
  ID3_FrameID GetID(void);
  ID3_Field& Field(ID3_FieldID name);

  // *** PRIVATE INTERNAL DATA - DO NOT USE *** PRIVATE INTERNAL DATA - DO NOT USE ***

  bool HasChanged(void);
  void SetVersion(uchar ver, uchar rev);
  void Parse(uchar *buffer, luint size);
  luint Size(void);
  luint Render(uchar *buffer);
  char encryptionID[256]; // the encryption method with which this frame is encrypted
  char groupingID[256]; // the group to which this frame belongs
  bool compression; // should we try to compress?
  bool hasChanged; // has the frame changed since the last parse/render?
  bitset fieldBits; // which fields are present?
  ID3_FrameID frameID; // what frame are we?

protected:
  void UpdateStringTypes(void);
  void UpdateFieldDeps(void);
  lsint FindField(ID3_FieldID name);
  uchar version; // what version tag?
  uchar revision; // what revision tag?
  luint numFields; // how many fields are in this frame?
  ID3_Field **fields; // an array of field object pointers
};


