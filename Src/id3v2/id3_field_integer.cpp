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
// improved/optimized/whatEVER jan-08-2006 benski

#include "id3_field.h"

#if 0 // taking out operators
ID3_Field& ID3_Field::operator=(luint newData)
{
  Set(newData);

  return *this;
}
#endif


void ID3_Field::Set(luint newData)
{
  Clear();

  data = reinterpret_cast<uchar *>(newData);
  size = sizeof(luint);
  type = ID3FTY_INTEGER;
  hasChanged = true;
}


luint ID3_Field::Get(void)
{
  return (luint) data;
}


luint ID3_Field::ParseInteger(uchar *buffer, luint posn, luint buffSize)
{
  luint bytesUsed = 0;

  if(buffer && buffSize)
  {
    luint i;
    luint temp = 0;

    bytesUsed = 4;

    if(fixedLength != -1)
      bytesUsed = MIN(fixedLength, bytesUsed);

    for(i = 0; i < bytesUsed; i++)
      temp |=(buffer[ posn + i ] <<(((bytesUsed - i) - 1) * 8));

    Set(temp);
    hasChanged = false;
  }

  return bytesUsed;
}


luint ID3_Field::RenderInteger(uchar *buffer)
{
  luint bytesUsed = 0;
  luint length = BinSize();

  for(luint i = 0; i < length; i++)
    buffer[ i ] = (uchar)((((luint) data) >>(((length - i) - 1) * 8)) & 0xFF);

  bytesUsed = length;
  hasChanged = false;

  return bytesUsed;
}
