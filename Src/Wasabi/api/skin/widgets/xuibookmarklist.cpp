#include <precomp.h>
#include "xuibookmarklist.h"

// -----------------------------------------------------------------------
const wchar_t BookmarkListXuiObjectStr[] = L"BookmarkList"; // This is the xml tag
char BookmarkListXuiSvcName[] = "BookmarkList xui object";

XMLParamPair BookmarkList::params[] = {
	{BOOKMARKLIST_SET, L""},
};


// -----------------------------------------------------------------------
BookmarkList::BookmarkList() {
  myxuihandle = newXuiHandle();
 	CreateXMLParameters(myxuihandle);
}

void BookmarkList::CreateXMLParameters(int master_handle)
{
	//BOOKMARKLIST_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

BookmarkList::~BookmarkList() 
{
}

// -----------------------------------------------------------------------
int BookmarkList::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return BOOKMARKLIST_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case BOOKMARKLIST_SET:
      set(value);
      break;
    default:
      return 0;
  }
  return 1;
}

// -----------------------------------------------------------------------
void BookmarkList::set(const wchar_t *elementname) {
}

