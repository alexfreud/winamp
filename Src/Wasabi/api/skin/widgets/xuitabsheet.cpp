#include <precomp.h>
#include "xuitabsheet.h"
#include <bfc/parse/paramparser.h>
static ScriptTabSheetController TabsheetController;
ScriptTabSheetController *tabsheetController=&TabsheetController;

// -----------------------------------------------------------------------
const wchar_t ScriptTabSheetXuiObjectStr[] = L"Wasabi:TabSheet"; // This is the xml tag
char ScriptTabSheetXuiSvcName[] = "Wasabi:TabSheet xui object"; 

XMLParamPair ScriptTabSheet::params[] = {

  {SCRIPTTABSHEET_SETCHILDREN, L"CHILDREN"},
  {SCRIPTTABSHEET_SETCONTENTMARGINBOTTOM, L"CONTENT_MARGIN_BOTTOM"},
  {SCRIPTTABSHEET_SETCONTENTMARGINLEFT, L"CONTENT_MARGIN_LEFT"},
	{SCRIPTTABSHEET_SETCONTENTMARGINRIGHT, L"CONTENT_MARGIN_RIGHT"},
  {SCRIPTTABSHEET_SETCONTENTMARGINTOP, L"CONTENT_MARGIN_TOP"},
	{SCRIPTTABSHEET_SETTYPE, L"TYPE"},
	{SCRIPTTABSHEET_SETWINDOWTYPE, L"WINDOWTYPE"},
	};
// -----------------------------------------------------------------------
ScriptTabSheet::ScriptTabSheet() : TypeSheet(0) {
  getScriptObject()->vcpu_setInterface(tabsheetGuid, (void *)static_cast<ScriptTabSheet*>(this));
  getScriptObject()->vcpu_setClassName(L"TabSheet"); // this is the script class name
  getScriptObject()->vcpu_setController(tabsheetController);

  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);
	
  wndtype = 0;
  type = TABSHEET_GROUPS;
  left_margin = right_margin = bottom_margin = top_margin = 0;
}

void ScriptTabSheet::CreateXMLParameters(int master_handle)
{
	//SCRIPTTABSHEET_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
ScriptTabSheet::~ScriptTabSheet() {
  children_id.deleteAll();
}

// -----------------------------------------------------------------------
int ScriptTabSheet::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) 
{
  if (xuihandle != myxuihandle)
    return SCRIPTTABSHEET_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) 
	{
    case  SCRIPTTABSHEET_SETWINDOWTYPE:
      setWindowType(value);
      break;
    case  SCRIPTTABSHEET_SETCHILDREN:
      setChildrenIds(value);
      break;
    case  SCRIPTTABSHEET_SETTYPE:
      setType(value);
      break;
    case  SCRIPTTABSHEET_SETCONTENTMARGINLEFT:
    case  SCRIPTTABSHEET_SETCONTENTMARGINTOP:
    case  SCRIPTTABSHEET_SETCONTENTMARGINRIGHT:
    case  SCRIPTTABSHEET_SETCONTENTMARGINBOTTOM:
      setContentMarginX(value, xmlattributeid);
      break;
    default:
      return 0;
  }
  return 1;
}

// -----------------------------------------------------------------------
void ScriptTabSheet::setWindowType(const wchar_t *paramvalue) {
  if (WCSEQLSAFE(wndtype, paramvalue)) return;
  wndtype = paramvalue;
  reloadChildren();
}

// -----------------------------------------------------------------------
void ScriptTabSheet::setChildrenIds(const wchar_t *paramvalue) {
  if (WCSEQLSAFE(paramvalue, L"")) {
    children_id.removeAll();
  }
  ParamParser pp(paramvalue);
  for (int i=0;i<pp.getNumItems();i++) 
    children_id.addItem(new StringW(pp.enumItem(i)));
  reloadChildren();
}

// -----------------------------------------------------------------------
void ScriptTabSheet::setType(const wchar_t *paramvalue) {
  int ttype = WTOI(paramvalue);
  if (type == ttype) return;
  type = ttype;
  setButtonType(type);
}

// -----------------------------------------------------------------------
void ScriptTabSheet::reloadChildren() {
  if (!isInited()) return;
  killChildren();
  for (int i=0;i<children_id.getNumItems();i++) {
    GuiObjectWnd *w = new GuiObjectWnd;
    //w->abstract_setAllowDeferredContent(1);
    w->setContent(children_id.enumItem(i)->getValue());
    addChild(w);
  }
  TypeSheet::setWindowType(wndtype);
  if (!wndtype.isempty()) 
    TypeSheet::load();
}

// -----------------------------------------------------------------------
int ScriptTabSheet::onInit() {
  int r = SCRIPTTABSHEET_PARENT::onInit();
  setButtonType(type);
  reloadChildren();
  return r;
}

void ScriptTabSheet::setContentMarginX(const wchar_t *value, int what) {
  switch (what) {
    case SCRIPTTABSHEET_SETCONTENTMARGINLEFT:
      setContentMarginLeft(WTOI(value));
      break;
    case SCRIPTTABSHEET_SETCONTENTMARGINTOP:
      setContentMarginTop(WTOI(value));
      break;
    case SCRIPTTABSHEET_SETCONTENTMARGINRIGHT:
      setContentMarginRight(WTOI(value));
      break;
    case SCRIPTTABSHEET_SETCONTENTMARGINBOTTOM:
      setContentMarginBottom(WTOI(value));
      break;
  }
}

// -----------------------------------------------------------------------
// Script Object

// -- Functions table -------------------------------------
function_descriptor_struct ScriptTabSheetController::exportedFunction[] = {
  {L"getCurPage",             0, (void*)ScriptTabSheetController::tabsheet_getCurPage },
  {L"setCurPage",             1, (void*)ScriptTabSheetController::tabsheet_setCurPage },
};

ScriptObject *ScriptTabSheetController::instantiate() {
  ScriptTabSheet *sts = new ScriptTabSheet;
  ASSERT(sts != NULL);
  return sts->getScriptObject();
}

void ScriptTabSheetController::destroy(ScriptObject *o) {
  ScriptTabSheet *sts = static_cast<ScriptTabSheet *>(o->vcpu_getInterface(tabsheetGuid));
  ASSERT(sts != NULL);
  delete sts;
}

void *ScriptTabSheetController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for tabsheet yet
}

void ScriptTabSheetController::deencapsulate(void *o) {
}

int ScriptTabSheetController::getNumFunctions() { 
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *ScriptTabSheetController::getExportedFunctions() { 
  return exportedFunction; 
}


scriptVar ScriptTabSheetController::tabsheet_getCurPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptTabSheet *sts = static_cast<ScriptTabSheet *>(o->vcpu_getInterface(tabsheetGuid));
  int a = 0;
  if (sts) a = sts->getCurPage();
  return MAKE_SCRIPT_INT(a);
}

scriptVar ScriptTabSheetController::tabsheet_getNumPages(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptTabSheet *sts = static_cast<ScriptTabSheet *>(o->vcpu_getInterface(tabsheetGuid));
  int a = 0;
  if (sts) a = sts->getNumPages();
  return MAKE_SCRIPT_INT(a);
}

scriptVar ScriptTabSheetController::tabsheet_setCurPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar a) {
  SCRIPT_FUNCTION_INIT
  ScriptTabSheet *sts = static_cast<ScriptTabSheet *>(o->vcpu_getInterface(tabsheetGuid));
  int _a = GET_SCRIPT_INT(a);
  if (sts) sts->setCurPage(_a);
  RETURN_SCRIPT_VOID;
}

scriptVar ScriptTabSheetController::tabsheet_nextPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptTabSheet *sts = static_cast<ScriptTabSheet *>(o->vcpu_getInterface(tabsheetGuid));
  if (sts) sts->nextPage();
  RETURN_SCRIPT_VOID;
}

scriptVar ScriptTabSheetController::tabsheet_previousPage(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  ScriptTabSheet *sts = static_cast<ScriptTabSheet *>(o->vcpu_getInterface(tabsheetGuid));
  if (sts) sts->previousPage();
  RETURN_SCRIPT_VOID;
}
