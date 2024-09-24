#include <precomp.h>
#include "svc_stringconverter.h"

#define CBCLASS svc_stringConverterI
START_DISPATCH
  CB(CANCONVERT, canConvert);
  CB(CONVERTTOUTF8, convertToUTF8);
  CB(PREFLIGHTTOUTF8, preflightToUTF8);
  CB(CONVERTFROMUTF8, convertFromUTF8);
  CB(PREFLIGHTFROMUTF8, preflightFromUTF8);
END_DISPATCH
#undef CBCLASS
