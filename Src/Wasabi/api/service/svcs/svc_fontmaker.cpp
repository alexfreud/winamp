#include "svc_fontmaker.h"

#define CBCLASS svc_fontMakerI
START_DISPATCH
  CB(GETFONTMAKERNAME, getFontMakerName);
  CB(NEWTRUETYPEFONT, newTrueTypeFont);
  CB(DELETETRUETYPEFONT, deleteTrueTypeFont);
END_DISPATCH
#undef CBCLASS
