#include <precomp.h>
#include "xuiwndholder.h"
#include <tataki/canvas/ifc_canvas.h>
#include <bfc/parse/paramparser.h>
#include <api/script/objects/sregion.h>

// -----------------------------------------------------------------------
const wchar_t WindowHolderXuiObjectStr[] = L"WindowHolder"; // This is the xml tag
const wchar_t WindowHolderXuiObjectStrCompat[] = L"Component"; // This is the old xml tag

char WindowHolderXuiSvcName[] = "WindowHolder xui object";
char WindowHolderXuiSvcNameCompat[] = "Component xui object";

// {7DB51C8C-36C0-4585-9240-A2DB22B1B8F7}
static const GUID pvt_xuiWindowHolder = 
{ 0x7db51c8c, 0x36c0, 0x4585, { 0x92, 0x40, 0xa2, 0xdb, 0x22, 0xb1, 0xb8, 0xf7 } };

XMLParamPair XuiWindowHolder::params[] = {
  {XUIWNDHOLDER_ADDHOLD, L"PARAM"},
  {XUIWNDHOLDER_ADDHOLD, L"COMPONENT"},
  {XUIWNDHOLDER_ADDHOLD, L"HOLD"},
  {XUIWNDHOLDER_SETNOSHOWCMDBAR, L"NOSHOWCMDBAR"},
  {XUIWNDHOLDER_SETNOANIMRECTS, L"NOANIMATEDRECTS"},
  {XUIWNDHOLDER_SETNOANIMRECTS, L"DISABLEANIMATEDRECTS"}, 
  {XUIWNDHOLDER_SETAUTOOPEN, L"AUTOOPEN"},
  {XUIWNDHOLDER_SETAUTOCLOSE, L"AUTOCLOSE"},
  {XUIWNDHOLDER_SETAUTOFOCUS, L"AUTOFOCUS"},
  {XUIWNDHOLDER_SETAUTOAVAILABLE, L"AUTOAVAILABLE"},
	};
// -----------------------------------------------------------------------
XuiWindowHolder::XuiWindowHolder() {
  getScriptObject()->vcpu_setInterface(windowHolderGuid, (void *)static_cast<WindowHolder *>(this));
  getScriptObject()->vcpu_setInterface(pvt_xuiWindowHolder, (void *)static_cast<XuiWindowHolder *>(this));
  getScriptObject()->vcpu_setClassName(L"WindowHolder"); // this is the script class name
  getScriptObject()->vcpu_setController(windowHolderController);

  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);

  setXmlParam(L"autofocus", L"1");
}
void XuiWindowHolder::CreateXMLParameters(int master_handle)
{
	//XUIWNDHOLDER_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

XuiWindowHolder::~XuiWindowHolder() {
}

void XuiWindowHolder::setRegionFromMap(ScriptObject *map, int byte, int inverse) {
}

void XuiWindowHolder::setRegion(ScriptObject *_region) {
//#pragma CHAT("bas", "lone", "I implemented setRegion for ya once SRegion is safe")
#if 1// if SRegion were cross-dll safe this would work
  const GUID regionGuid = 
{ 0x3a370c02, 0x3cbf, 0x439f, { 0x84, 0xf1, 0x86, 0x88, 0x5b, 0xcf, 0x1e, 0x36 } };
  void *reg = _region->vcpu_getInterface(regionGuid);
  ASSERT(reg != NULL);
  SRegion *sr = static_cast<SRegion*>(reg);
  ASSERT(sr != NULL);
  api_region *region = sr->getRegion();
  ASSERT(region != NULL);
  api_region *clone = region->clone();
  clone->scale(getRenderRatio(), getRenderRatio());
	ifc_window *curRootWnd = getCurRootWnd();
	if (curRootWnd)
	{
#ifdef _WIN32
		OSWINDOWHANDLE osWnd = curRootWnd->gethWnd();
		if (osWnd)
		  SetWindowRgn(osWnd, clone->makeWindowRegion(), TRUE);
#else
#warning port me
    // can probably use a mask here
#endif
	}
  region->disposeClone(clone);
#endif
}

// -----------------------------------------------------------------------
int XuiWindowHolder::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return XUIWNDHOLDER_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) 
	{
    case XUIWNDHOLDER_ADDHOLD: 
      if (!WCSICMP(value, L"@ALL@") || !WCSICMP(value, L"guid:default")) 
			{
        setAcceptAllGuids(1);
        setAcceptAllGroups(1);
      } else {
        setAcceptAllGuids(0);
        setAcceptAllGroups(0);
        ParamParser pp(value);
        for (int i=0;i<pp.getNumItems();i++) {
          const wchar_t *e = pp.enumItem(i);
          if (*e == '{' || !WCSNICMP(e, L"guid:", 5)) {
            GUID *g = parseGUID(e);
            if (g)
              addAcceptGuid(*g);
          }
          else
            addAcceptGroup(e);
        }
      }
      break;
    case XUIWNDHOLDER_SETNOSHOWCMDBAR:
      setNoCmdBar(WTOI(value));
      break;
    case XUIWNDHOLDER_SETNOANIMRECTS:
      setNoAnim(WTOI(value));
      break;
    case XUIWNDHOLDER_SETAUTOOPEN:
      setAutoOpen(WTOI(value));
      break;
    case XUIWNDHOLDER_SETAUTOCLOSE:
      setAutoClose(WTOI(value));
      break;
    case XUIWNDHOLDER_SETAUTOFOCUS:
      setAutoFocus(WTOI(value));
      break;
    case XUIWNDHOLDER_SETAUTOAVAILABLE:
      setAutoAvailable(WTOI(value));
      break;
    default:
      return 0;
  }
  return 1;
}

GUID *XuiWindowHolder::parseGUID(const wchar_t *id) {
  if (WCSNICMP(id, L"guid:{",6)==0) {
    static GUID g;
    id+=5;
    g = nsGUID::fromCharW(id);
    return &g;
  }
  if (id && *id == '{') {
    static GUID g;
    g = nsGUID::fromCharW(id);
    return &g;
  }                                             
  if(WCSICMP(id,L"guid:avs")==0) {
    static GUID g={ 10, 12, 16, { 255, 123, 1, 1, 66, 99, 69, 12 } };
    return &g;
  }
  if (WCSICMP(id,L"guid:pl")==0 || WCSICMP(id,L"guid:playlist")==0) {
    static GUID g={ 0x45f3f7c1, 0xa6f3, 0x4ee6, { 0xa1, 0x5e, 0x12, 0x5e, 0x92, 0xfc, 0x3f, 0x8d } };
    return &g;
  }
  if (WCSICMP(id,L"guid:ml")==0 || WCSICMP(id,L"guid:musiclibrary")==0 || WCSICMP(id,L"guid:library")==0) {
    static GUID g={ 0x6b0edf80, 0xc9a5, 0x11d3, { 0x9f, 0x26, 0x00, 0xc0, 0x4f, 0x39, 0xff, 0xc6 } };
    return &g;
  }
	if(WCSICMP(id,L"guid:null")==0) {
    static GUID g=INVALID_GUID;
    return &g;
  }
	if(WCSICMP(id,L"guid:player")==0) {
    static GUID g = { 0xe6323f86, 0x1724, 0x4cd3, { 0x9d, 0x87, 0x70, 0x59, 0x1f, 0xc1, 0x6e, 0x5e } };
    return &g;
  }
	
  return NULL;
}

// -----------------------------------------------------------------------
// Script Object

WindowHolderScriptController _windowHolderController;
WindowHolderScriptController *windowHolderController = &_windowHolderController;

// -- Functions table -------------------------------------
function_descriptor_struct WindowHolderScriptController::exportedFunction[] = {
  {L"getGUID", 1, (void*)WindowHolderScriptController::script_getGUID },
  {L"setRegionFromMap", 3, (void*)WindowHolderScriptController::script_setRegionFromMap },
  {L"setRegion", 1, (void*)WindowHolderScriptController::script_setRegion },
  {L"getContent", 0, (void*)WindowHolderScriptController::script_getContent},
  {L"getComponentName", 0, (void*)WindowHolderScriptController::script_getComponentName},
};

ScriptObject *WindowHolderScriptController::instantiate() {
  XuiWindowHolder *wh = new XuiWindowHolder;
  ASSERT(wh != NULL);
  return wh->getScriptObject();
}

void WindowHolderScriptController::destroy(ScriptObject *o) {
  XuiWindowHolder *wh = static_cast<XuiWindowHolder *>(o->vcpu_getInterface(pvt_xuiWindowHolder));
  ASSERT(wh != NULL);
  delete wh;
}

void *WindowHolderScriptController::encapsulate(ScriptObject *o) {
  return NULL; // no encapsulation for windowholder yet
}

void WindowHolderScriptController::deencapsulate(void *o) {
}

int WindowHolderScriptController::getNumFunctions() { 
  return sizeof(exportedFunction) / sizeof(function_descriptor_struct); 
}

const function_descriptor_struct *WindowHolderScriptController::getExportedFunctions() { 
  return exportedFunction; 
}

scriptVar WindowHolderScriptController::script_getComponentName(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) 
{
	SCRIPT_FUNCTION_INIT
	XuiWindowHolder *wh = static_cast<XuiWindowHolder*>(o->vcpu_getInterface(pvt_xuiWindowHolder));
	
	if (wh) 
	{
		ifc_window *ro = wh->getCurRootWnd();
		if (ro)
		{
			return MAKE_SCRIPT_STRING(ro->getRootWndName());
		}
	}
	return MAKE_SCRIPT_STRING(L"");  
}

scriptVar WindowHolderScriptController::script_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) 
{
  SCRIPT_FUNCTION_INIT
  XuiWindowHolder *wh = static_cast<XuiWindowHolder*>(o->vcpu_getInterface(pvt_xuiWindowHolder));
	
  if (wh) 
	{
		static wchar_t guidstr[256];
		nsGUID::toCharW(wh->getCurGuid(), guidstr);
		return MAKE_SCRIPT_STRING(guidstr);
	}
  return MAKE_SCRIPT_STRING(L"");  
}

scriptVar WindowHolderScriptController::script_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv) {
  SCRIPT_FUNCTION_INIT
  XuiWindowHolder *xu = static_cast<XuiWindowHolder*>(o->vcpu_getInterface(pvt_xuiWindowHolder));
  if (xu) xu->setRegionFromMap(GET_SCRIPT_OBJECT(map), GET_SCRIPT_INT(byte), GET_SCRIPT_INT(inv));
  RETURN_SCRIPT_VOID;  
}

scriptVar WindowHolderScriptController::script_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar reg) {
  SCRIPT_FUNCTION_INIT
  XuiWindowHolder *xu= static_cast<XuiWindowHolder*>(o->vcpu_getInterface(pvt_xuiWindowHolder));
  if (xu) xu->setRegion(GET_SCRIPT_OBJECT(reg));
  RETURN_SCRIPT_VOID;  
}

scriptVar WindowHolderScriptController::script_getContent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o) {
  SCRIPT_FUNCTION_INIT
  XuiWindowHolder *xu= static_cast<XuiWindowHolder*>(o->vcpu_getInterface(pvt_xuiWindowHolder));
  if (xu) {
    ifc_window *w = xu->getRootWndPtr();
    if (w) {
      GuiObject *o = w->getGuiObject();
      if (o) 
        return MAKE_SCRIPT_OBJECT(o->guiobject_getScriptObject());
    }
  }
  RETURN_SCRIPT_NULL;
}
