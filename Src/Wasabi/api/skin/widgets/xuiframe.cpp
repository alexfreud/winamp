#include <precomp.h>
#include "xuiframe.h"
#include <tataki/canvas/ifc_canvas.h>
#include <api/script/scriptguid.h>

const wchar_t ScriptFrameXuiObjectStr[] = L"Wasabi:Frame"; // This is the xml tag
char ScriptFrameXuiSvcName[] = "Wasabi:ScriptFrame xui object"; 
XMLParamPair ScriptFrame::params[] = {
  {SCRIPTFRAME_SETORIENTATION, L"ORIENTATION"},
  {SCRIPTFRAME_SETLEFT, L"LEFT"}, // TOP/BOTTOM IS ALIAS FOR LEFT/RIGHT
  {SCRIPTFRAME_SETLEFT, L"TOP"},
  {SCRIPTFRAME_SETRIGHT, L"RIGHT"},
  {SCRIPTFRAME_SETRIGHT, L"BOTTOM"},
  {SCRIPTFRAME_SETFROM, L"FROM"},
  {SCRIPTFRAME_SETWIDTH, L"WIDTH"},
  {SCRIPTFRAME_SETWIDTH, L"HEIGHT"}, // HEIGHT IS AN ALIAS FOR WIDTH
  {SCRIPTFRAME_SETRESIZEABLE, L"RESIZABLE"}, 
	{SCRIPTFRAME_SETMAXWIDTH, L"MAXWIDTH"}, 
	{SCRIPTFRAME_SETMAXWIDTH, L"MAXHEIGHT"}, //ALIAS
	{SCRIPTFRAME_SETMINWIDTH, L"MINWIDTH"}, 
	{SCRIPTFRAME_SETMINWIDTH, L"MINHEIGHT"}, //ALIAS
	{SCRIPTFRAME_SETV_BITMAP, L"VBITMAP"}, // to override wasabi.framewnd.verticaldivider
	{SCRIPTFRAME_SETV_GRABBER, L"VGRABBER"},  // to override wasabi.framewnd.verticalgrabber
};

ScriptFrame::ScriptFrame() 
{
	getScriptObject()->vcpu_setInterface(scriptFrameGuid, (void *)static_cast<ScriptFrame *>(this));
	getScriptObject()->vcpu_setClassName(L"Frame");
	getScriptObject()->vcpu_setController(frameController);

  setVirtual(1);
  myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);	

  orientation = DIVIDER_VERTICAL;
  from = SDP_FROMLEFT;
  resizable = 1;
  width = 128;
  rootwndleft = rootwndright = NULL;
}

void ScriptFrame::CreateXMLParameters(int master_handle)
{
	//SCRIPTFRAME_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

ScriptFrame::~ScriptFrame() {
  if (rootwndright) {
    WASABI_API_SKIN->group_destroy(rootwndright);
    rootwndright = NULL;
  }
  if (rootwndleft) {
    WASABI_API_SKIN->group_destroy(rootwndleft);
    rootwndleft = NULL;
  }
}

// XuiObject automatically calls this back for all parameters registered using addParam
// encountered in the xml source
int ScriptFrame::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {

  if (xuihandle != myxuihandle) 
    return SCRIPTFRAME_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

  switch (xmlattributeid) {
    case SCRIPTFRAME_SETORIENTATION: setOrientation(value); break;
    case SCRIPTFRAME_SETLEFT:        setLeft(value);        break;
    case SCRIPTFRAME_SETRIGHT:       setRight(value);       break;
    case SCRIPTFRAME_SETFROM:        setFrom(value);        break;
    case SCRIPTFRAME_SETWIDTH:       setWidth(value);       break;
    case SCRIPTFRAME_SETRESIZEABLE:  setResize(value);      break;
	case SCRIPTFRAME_SETMAXWIDTH: setMaxWidth((!_wcsicmp(value, L"null")) ? 0 : WTOI(value)); break;
		case SCRIPTFRAME_SETMINWIDTH: setMinWidth(WTOI(value)); break;
		case SCRIPTFRAME_SETSNAP: setSnap(WTOI(value)); break;
		case SCRIPTFRAME_SETV_BITMAP: 
			Set_v_bitmap(value); 
			break;
		case SCRIPTFRAME_SETV_GRABBER: 
			Set_v_grabber(value);
			break;

    default:
      return 0;
  }
  return 1;
}

int ScriptFrame::onInit() 
{

  if (!left.isempty()) rootwndleft = WASABI_API_SKIN->group_create(left);
  if (!right.isempty()) rootwndright = WASABI_API_SKIN->group_create(right);

/*  GuiObject *gl = rootwndleft ? static_cast<GuiObject *>(rootwndleft->getInterface(guiObjectGuid)) : NULL;
  GuiObject *gr = rootwndright ? static_cast<GuiObject *>(rootwndleft->getInterface(guiObjectGuid)) : NULL;

  if (gl) gl->guiobject_setParentGroup(getGuiObject()->guiobject_getParentGroup());
  if (gr) gr->guiobject_setParentGroup(getGuiObject()->guiobject_getParentGroup());*/

  if (rootwndleft) rootwndleft->setParent(this);
  if (rootwndright) rootwndright->setParent(this);
  
  setChildrenRootWnd(rootwndleft, rootwndright);
  setDividerType(static_cast<FrameWndDividerType>(orientation));
  setDividerPos(from, width);
  setResizeable(resizable);
  
  
  SCRIPTFRAME_PARENT::onInit();
  return 1;
}

void ScriptFrame::onResizeChildren(RECT leftr, RECT rightr) {
/*  if (rootwndleft) rootwndleft->invalidate();
  if (rootwndright) rootwndright->invalidate();*/
  invalidate();
}

void ScriptFrame::setOrientation(const wchar_t *orient) {
  if (!WCSICMP(orient, L"v") || !WCSICMP(orient, L"vertical"))
    orientation = DIVIDER_VERTICAL;
  if (!WCSICMP(orient, L"h") || !WCSICMP(orient, L"horizontal"))
    orientation = DIVIDER_HORIZONTAL;
}

void ScriptFrame::setLeft(const wchar_t *groupname) {
  left = groupname;
}

void ScriptFrame::setRight(const wchar_t *groupname) {
  right = groupname;
}

void ScriptFrame::setFrom(const wchar_t *f) {
  if (!WCSICMP(f, L"l") || !WCSICMP(f, L"left") || !WCSICMP(f, L"t") || !WCSICMP(f, L"top"))
    from = SDP_FROMLEFT;
  if (!WCSICMP(f, L"r") || !WCSICMP(f, L"right") || !WCSICMP(f, L"b") || !WCSICMP(f, L"bottom"))
    from = SDP_FROMRIGHT;
}

void ScriptFrame::setWidth(const wchar_t *w) {
  width = WTOI(w);
}

void ScriptFrame::setResize(const wchar_t *w) {
  resizable = WTOI(w);
  if (isInited())
    setResizeable(resizable);
}

FrameScriptController _frameController;
FrameScriptController *frameController = &_frameController;

// -- Functions table -------------------------------------
function_descriptor_struct FrameScriptController::exportedFunction[] = 
{
	{L"setPosition", 1, (void*)ScriptFrame::script_vcpu_setPosition },
	{L"getPosition", 0, (void*)ScriptFrame::script_vcpu_getPosition },
};
// --------------------------------------------------------

const wchar_t *FrameScriptController::getClassName()
{
	return L"Frame";
}

const wchar_t *FrameScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *FrameScriptController::instantiate()
{
	ScriptFrame *t = new ScriptFrame;
	ASSERT(t != NULL);
	return t->getScriptObject();
}

void FrameScriptController::destroy(ScriptObject *o)
{
	ScriptFrame *t = static_cast<ScriptFrame *>(o->vcpu_getInterface(scriptFrameGuid));
	ASSERT(t != NULL);
	delete t;
}

void *FrameScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for Wasabi:Frame yet
}

void FrameScriptController::deencapsulate(void *o)
{}

int FrameScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *FrameScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID FrameScriptController::getClassGuid()
{
	return scriptFrameGuid;
}

const wchar_t *ScriptFrame::vcpu_getClassName()
{
	return L"Frame";
}

scriptVar ScriptFrame::script_vcpu_setPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar t)
{
	SCRIPT_FUNCTION_INIT
	ASSERT(t.type == SCRIPT_INT);
	ScriptFrame *fr = static_cast<ScriptFrame *>(o->vcpu_getInterface(scriptFrameGuid));
	if (fr)
		fr->setDividerPosNoCfg(fr->from, ::GET_SCRIPT_INT(t));	

	RETURN_SCRIPT_VOID;
}

scriptVar ScriptFrame::script_vcpu_getPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	ScriptFrame *fr = static_cast<ScriptFrame *>(o->vcpu_getInterface(scriptFrameGuid));
	if (fr)
	{
		int pos, from;
		fr->getDividerPos(&from, &pos);
		return MAKE_SCRIPT_INT(pos);
	}
	return MAKE_SCRIPT_INT(0);
}
