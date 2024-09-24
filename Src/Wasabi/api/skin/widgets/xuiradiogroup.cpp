#include <precomp.h>
#include "xuiradiogroup.h"
#include <bfc/parse/paramparser.h>
#include <api/script/objects/c_script/c_text.h>

// -----------------------------------------------------------------------
const wchar_t ScriptRadioGroupXuiObjectStr[] = L"Wasabi:RadioGroup"; // This is the xml tag
char ScriptRadioGroupXuiSvcName[] = "Wasabi:RadioGroup xui object"; 

// -----------------------------------------------------------------------
ScriptRadioGroup::ScriptRadioGroup() : SCRIPTRADIOGROUP_PARENT(), myxuihandle(0) {
}

// -----------------------------------------------------------------------
ScriptRadioGroup::~ScriptRadioGroup() {
}

