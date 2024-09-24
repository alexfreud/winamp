#include <precomp.h>
#include "xuititlebox.h"

// -----------------------------------------------------------------------
const wchar_t ScriptTitleBoxXuiObjectStr[] = L"Wasabi:TitleBox"; // This is the xml tag
char ScriptTitleBoxXuiSvcName[] = "Wasabi:TitleBox xui object";

XMLParamPair ScriptTitleBox::params[] = {
                                            {SCRIPTTITLEBOX_CENTERED, L"CENTERED"},
                                            {SCRIPTTITLEBOX_CONTENT, L"CONTENT"},
                                            {SCRIPTTITLEBOX_SUFFIX, L"SUFFIX"},
                                            {SCRIPTTITLEBOX_TITLE, L"TITLE"},
                                        };
// -----------------------------------------------------------------------
ScriptTitleBox::ScriptTitleBox() : SCRIPTTITLEBOX_PARENT()
{
	myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);
}

void ScriptTitleBox::CreateXMLParameters(int master_handle)
{
	//SCRIPTTITLEBOX_PARENT::CreateXMLParameters(master_handle);
int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
ScriptTitleBox::~ScriptTitleBox()
{}

// -----------------------------------------------------------------------
int ScriptTitleBox::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value)
{
	if (xuihandle != myxuihandle)
		return SCRIPTTITLEBOX_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

	// Parcel the values out to the window object we multiply inherit from
	switch (xmlattributeid)
	{
	case SCRIPTTITLEBOX_TITLE:
		setTitle(value);
		break;
	case SCRIPTTITLEBOX_CONTENT:
		setChildGroup(value);
		break;
	case SCRIPTTITLEBOX_CENTERED:
		setCentered(WTOI(value));
		break;
	case SCRIPTTITLEBOX_SUFFIX:
		setSuffix(value);
		break;
	default:
		return 0;
	}
	return 1;
}
