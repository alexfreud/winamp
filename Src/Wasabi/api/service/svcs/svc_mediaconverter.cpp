#include <precomp.h>

#include "svc_mediaconverter.h"

#define CBCLASS svc_mediaConverterNI
START_DISPATCH;
  CB(CANCONVERTFROM,canConvertFrom)
  CB(GETCONVERTERTO,getConverterTo)
  CB(GETINFOS,getInfos)
  CB(SETINFOS,setInfos)
  CB(GETINFOSXMLGROUP,getInfosXmlGroup)
  CB(PROCESSDATA,processData)
  CB(GETPOSITION,getPosition)
  CB(GETLATENCY,getLatency)
  CB(GETCORECALLBACK,getCoreCallback)
  CB(SORTPLACEMENT,sortPlacement)
  CB(ISSELECTABLEOUTPUT,isSelectableOutput)
  CB(CANSUPPORTCONTENTTYPE,canSupportContentType)
  VCB(SETCORETOKEN,setCoreToken)
  CB(ONCOREUSERMSG,onCoreUserMsg)
END_DISPATCH;
#undef CBCLASS

