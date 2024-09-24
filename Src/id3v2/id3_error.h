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


#ifndef ID3LIB_ERROR_H
#define ID3LIB_ERROR_H

#include "id3_types.h"

enum ID3_Err
{
  ID3E_NoMemory = 0,
  ID3E_NoData,
  ID3E_NoBuffer,
  ID3E_InvalidFrameID,
  ID3E_FieldNotFound,
  ID3E_UnknownFieldType,
  ID3E_TagAlreadyAttached,
  ID3E_InvalidTagVersion,
  ID3E_NoFile,
  ID3E_zlibError
};


class ID3_Error
{
public:
  ID3_Err GetErrorID(void);
  //char *GetErrorDesc(void);
  char *GetErrorFile(void);
  luint GetErrorLine(void);

  // *** PRIVATE INTERNAL DATA - DO NOT USE *** PRIVATE INTERNAL DATA - DO NOT USE ***

  ID3_Error(ID3_Err id, char *file, luint lineNum);
protected:
  ID3_Err error;
  luint errLine;
  char errFile[256];
};

#ifdef _DEBUG
//#define ID3_THROW(x) throw ID3_Error (x, __FILE__, __LINE__)
#define ID3_THROW(x) void()
#else
#define ID3_THROW(x) void()
#endif


#endif


