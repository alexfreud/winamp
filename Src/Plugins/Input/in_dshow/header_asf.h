#ifndef _HEADER_ASF_H
#define _HEADER_ASF_H

#include "Header.h"

class HeaderAsf : public Header
{
public:
  HeaderAsf();

  int getInfos(const wchar_t *filename, bool checkMetadata=false);
};

#endif