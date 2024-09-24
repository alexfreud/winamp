#include <precomp.h>
#include "wa2groupdefs.h"
#include <bfc/string/StringW.h>

//-----------------------------------------------------------------------------------------------
Wa2Groupdefs::Wa2Groupdefs() {
  WASABI_API_SYSCB->syscb_registerCallback(this);
}

//-----------------------------------------------------------------------------------------------
Wa2Groupdefs::~Wa2Groupdefs() {
  WASABI_API_SYSCB->syscb_deregisterCallback(this);
}

//-----------------------------------------------------------------------------------------------
int Wa2Groupdefs::skincb_onBeforeLoadingElements() {
  StringW s;

       // header

  s =  L"buf:"
       L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"yes\"?>"
       L"<WinampAbstractionLayer version=\"0.8\">"

       L"<groupdef id=\"library.content.group\">"
       L"  <windowholder hold=\"{6B0EDF80-C9A5-11d3-9F26-00C04F39FFC6}\" fitparent=\"1\" />"
       L"</groupdef>";

       // footer

	s += L"</WinampAbstractionLayer>";

  WASABI_API_SKIN->loadSkinFile(s);
  return 1;
}

