#include <precomp.h>
#include "api_font.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS api_fontI
START_DISPATCH;
  VCB(API_FONT_FONT_TEXTOUT, font_textOut);
  CB(API_FONT_FONT_GETINFO, font_getInfo);
END_DISPATCH;
