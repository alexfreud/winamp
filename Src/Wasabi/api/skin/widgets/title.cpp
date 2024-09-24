#include <precomp.h>
#include "title.h"
#include <api/wndmgr/layout.h>
#include <api/wnd/paintset.h>
#include <api/skin/widgets/text.h>
#include <api/locales/xlatstr.h>
#include <api/wnd/cwndtrack.h>
#include <api/util/varmgr.h>
#include <api/wnd/PaintCanvas.h>

#define DC_MAXIMIZE 0x9831

const wchar_t titleBarXuiObjectStr[] = L"TitleBar"; // This is the xml tag
char titleBarXuiSvcName[] = "TitleBar xui object"; // this is the name of the xuiservice
XMLParamPair Title::params[] = {
                                   {TITLE_SETBORDER, L"BORDER"},
                                   {TITLE_SETDBLCLKACTION, L"DBLCLICKACTION"},
                                   {TITLE_SETMAXIMIZE, L"MAXIMIZE"},
                                   {TITLE_SETSTREAKS, L"STREAKS"},
                                   {TITLE_SETTITLE, L"TITLE"},
                               };
Title::Title()
{
	getScriptObject()->vcpu_setInterface(titleGuid, (void *)static_cast<Title *>(this));
	getScriptObject()->vcpu_setClassName(L"Title");
	getScriptObject()->vcpu_setController(titleController);
	dostreaks = 1;
	doborder = 1;
	m_maximize = 0;
	getGuiObject()->guiobject_setMover(1);
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);
}

void Title::CreateXMLParameters(int master_handle)
{
	//TITLE_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

Title::~Title()
{}

int Title::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval)
{
	if (xuihandle != _xuihandle) return TITLE_PARENT::setXuiParam(_xuihandle, attrid, name, strval);
	switch (attrid)
	{
	case TITLE_SETTITLE:
		setTitle(strval);
		break;
	case TITLE_SETSTREAKS:
		setStreaks(WTOI(strval));
		break;
	case TITLE_SETBORDER:
		setBorder(WTOI(strval));
		break;
	case TITLE_SETMAXIMIZE:
		m_maximize = WTOI(strval);
		break;
	case TITLE_SETDBLCLKACTION:
		dblClickAction = strval;
		break;
	default:
		return 0;
	}
	return 1;
}

void Title::setStreaks(int s)
{
	if (s == dostreaks) return ;
	dostreaks = s;
	invalidate();
}

void Title::setBorder(int b)
{
	if (b == doborder) return ;
	doborder = b;
	invalidate();
}

int Title::getPreferences(int what)
{
	if (what == SUGGESTED_W) return 128;
	if (what == SUGGESTED_H) return 22;
	return TITLE_PARENT::getPreferences(what);
}

int Title::onPaint(Canvas *canvas)
{
	const wchar_t *tempname = title;
	//StringW tempname(title);
	PaintCanvas paintcanvas;
	if (canvas == NULL)
	{
		if (!paintcanvas.beginPaint(this)) return 0;
		canvas = &paintcanvas;
	}
	TITLE_PARENT::onPaint(canvas);

#ifdef WA3COMPATIBILITY
	//tempname = PublicVarManager::translate(title, getGuiObject());
#else
	tempname = title;
#endif


#ifdef WASABI_COMPILE_PAINTSETS
	RECT pr(TITLE_PARENT::clientRect());
	const wchar_t *t = NULL;
	switch(wantTranslation())
	{
	case 0:
		t = tempname;
		break;
	case 1:
		t = _(tempname);
		break;
	case 2:
		t = __(tempname);
		break;
	}
	paintset_renderTitle(t, canvas, &pr, getPaintingAlpha(), dostreaks, doborder);
#endif


	return 1;
}

void Title::setTitle(const wchar_t *t)
{
	title = t;
	title.toupper();
}

const wchar_t *Title::getTitle()
{
	return title;
}

int Title::onLeftButtonDblClk(int x, int y)
{
	if (m_maximize)
		postDeferredCallback(DC_MAXIMIZE, 0);
	else
	{
#ifdef WASABI_COMPILE_WNDMGR
		if (dblClickAction)
		{
			const wchar_t *toCheck = L"SWITCH;";
			if (!WCSNICMP(dblClickAction, toCheck, 7))
			{
				onLeftButtonUp(x, y);
				getGuiObject()->guiobject_getParentGroup()->getParentContainer()->switchToLayout(dblClickAction.getValue() + 7);
			}
		}
#endif

	}
	ifc_window *b = getParent();
	if (b)
		return b->onLeftButtonDblClk(x, y);
	return TITLE_PARENT::onLeftButtonDblClk(x, y);
}

int Title::onDeferredCallback(intptr_t param1, intptr_t param2)
{
	switch (param1)
	{
#ifdef WASABI_COMPILE_WNDMGR
	case DC_MAXIMIZE:
		Container *c = getGuiObject()->guiobject_getParentGroup()->getParentContainer();
		if (c)
		{
			Layout *l = c->getCurrentLayout();
			if (l)
			{
				if (l->isMaximized()) l->restore();
				else l->maximize();
			}
		}
		return 1;
#endif

	}
	return TITLE_PARENT::onDeferredCallback(param1, param2);
}

TitleScriptController _titleController;
TitleScriptController *titleController = &_titleController;


// -- Functions table -------------------------------------
function_descriptor_struct TitleScriptController::exportedFunction[] = {
            {L"fake", 0, (void*)Title::script_vcpu_fake },
        };

const wchar_t *TitleScriptController::getClassName()
{
	return L"Title";
}

const wchar_t *TitleScriptController::getAncestorClassName()
{
	return L"GuiObject";
}

ScriptObject *TitleScriptController::instantiate()
{
	Title *t = new Title;
	ASSERT(t != NULL);
	return t->getScriptObject();
}

void TitleScriptController::destroy(ScriptObject *o)
{
	Title *t = static_cast<Title *>(o->vcpu_getInterface(titleGuid));
	ASSERT(t != NULL);
	delete t;
}

void *TitleScriptController::encapsulate(ScriptObject *o)
{
	return NULL; // no encapsulation for title yet
}

void TitleScriptController::deencapsulate(void *o)
{}

int TitleScriptController::getNumFunctions()
{
	return sizeof(exportedFunction) / sizeof(function_descriptor_struct);
}

const function_descriptor_struct *TitleScriptController::getExportedFunctions()
{
	return exportedFunction;
}

GUID TitleScriptController::getClassGuid()
{
	return titleGuid;
}


const wchar_t *Title::vcpu_getClassName()
{
	return L"Title";
}

scriptVar Title::script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o)
{
	SCRIPT_FUNCTION_INIT
	RETURN_SCRIPT_VOID;
}

