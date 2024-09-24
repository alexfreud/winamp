#include <precomp.h>
#include "sdebuggerui.h"
#include <api/script/debugger/jitd.h>
#include <tataki/canvas/canvas.h>
#include <api/wnd/wndclass/editwnd.h>
#include <api/script/objcontroller.h>
#include <bfc/string/stringdict.h>
#include <bfc/parse/paramparser.h>
#include <api/wnd/notifmsg.h>
#include <api/script/objecttable.h>
#include <api/script/debugger/sourcecodeline.h>
#include "../nu/AutoWide.h"

#define LEFTCOLUMN 100
#define RIGHTCOLUMN 200
#define INPUTHEIGHT 21
#define NCONSOLELINES 4
#define LINEHEIGHT 14
#define REGWIDTH 100

SimpleDebuggerUI::SimpleDebuggerUI()
{
	leave = 0;
	jitd = NULL;
	edit = NULL;
	memset(cmdbuf, 0, sizeof(cmdbuf));
	retcode = JITD_RETURN_CONTINUE;
}

SimpleDebuggerUI::~SimpleDebuggerUI()
{}

void SimpleDebuggerUI::setJITD(MakiJITD *_jitd)
{
	jitd = _jitd;
}

int SimpleDebuggerUI::messageLoop()
{
	leave = 0;
	retcode = JITD_RETURN_STEPINTO;

	if (!isInited())
	{
		setVirtual(0);
		setStartHidden(1);
		setParent(WASABI_API_WND->main_getRootWnd());
		init(WASABI_API_WND->main_getRootWnd(), 1);

		edit = new EditWnd;
		edit->setParent(this);
		edit->setBackgroundColor(RGB(0, 0, 0));
		edit->setTextColor(RGB(0, 255, 0));
		edit->setWantFocus(0);

		*cmdbuf = 0;
		edit->setBuffer(cmdbuf, 256);
		edit->init(this);

		RECT r;
		POINT pt = {0, 0};
		Wasabi::Std::getViewport(&r, &pt);
		resize(r.right - 656, r.top + 16, 640, 480);
		bringToFront();
	}

	for (int s = 0;s < jitd->getVSP();s++)
	{
		scriptVar v = WASABI_API_MAKIDEBUG->debugger_readStack(s);
		StringW str;
		switch (v.type)
		{
			case SCRIPT_VOID:
				str = L"NULL";
				break;
			case SCRIPT_INT:
				str = StringPrintfW(L"%d", GET_SCRIPT_INT(v));
				break;
			case SCRIPT_BOOLEAN:
				str = StringPrintfW(L"%s", GET_SCRIPT_BOOLEAN(v) ? L"true" : L"false");
				break;
			case SCRIPT_FLOAT:
				str = StringPrintfW(L"%f", GET_SCRIPT_FLOAT(v));
				break;
			case SCRIPT_DOUBLE:
				str = StringPrintfW(L"%f", (float)GET_SCRIPT_DOUBLE(v));
				break;
			case SCRIPT_STRING:
				str = GET_SCRIPT_STRING(v);
				break;
			default:
			{
				if (v.type == SCRIPT_OBJECT)
					str = L"Object";
				else
					str = ObjectTable::getClassName(v.type);
#ifdef WASABI_COMPILE_SKIN
				ScriptObject *o = GET_SCRIPT_OBJECT(v);
				if (o != NULL)
				{
					GuiObject *go = static_cast<GuiObject *>(o->vcpu_getInterface(guiObjectGuid));
					if (go != NULL)
					{
						str += L";";
						str.cat(go->guiobject_getId());
					}
				}
#endif
				break;
			}
		}
		strstack.addItem(new StringW(str));
	}

	setVisible(1);

	//WASABI_API_WND->pushModalWnd(this);

#ifdef WIN32
	MSG msg;
	//DWORD leavetime = GetTickCount()+5;
	while (!leave/* && !(GetTickCount() >leavetime)*/)
	{
		if (PeekMessage(&msg, /*(HWND)NULL*/ getOsWindowHandle(), 0, 0, PM_NOREMOVE))
		{
			GetMessage(&msg, /*(HWND) NULL*/getOsWindowHandle(), 0, 0) &&
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
#endif

	//WASABI_API_WND->popModalWnd(this);

	setVisible(0);
	strstack.deleteAll();

	return retcode;
}

int SimpleDebuggerUI::onPaint(Canvas *c)
{
	SimpleDebuggerUI_PARENT::onPaint(c);
	Wasabi::FontInfo fontInfo;
	RECT r;
	getClientRect(&r);
	c->fillRect(&r, RGB(0, 0, 0));
	c->drawRect(&r, 1, RGB(0, 255, 0));

	fontInfo.color = RGB(0, 255, 0);
	fontInfo.pointSize = 14;
	fontInfo.face = L"Courier New";
	c->pushPen(PENSTYLE_SOLID, 1, RGB(0, 255, 0));
	c->textOut(r.right - REGWIDTH + 7, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2) + 8,  StringPrintfW(L"VSD:%08X", jitd->getVSD()), &fontInfo);
	c->textOut(r.right - REGWIDTH + 7, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2) + 8 + (LINEHEIGHT + 1), StringPrintfW(L"VIP:%08X", jitd->getVIP()), &fontInfo);
	c->textOut(r.right - REGWIDTH + 7, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2) + 8 + (LINEHEIGHT + 1)*2, StringPrintfW(L"VSP:%08X", jitd->getVSP()), &fontInfo);
	c->textOut(r.right - REGWIDTH + 7, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2) + 8 + (LINEHEIGHT + 1)*3, StringPrintfW(L"VCC:%08X", jitd->getVCC()), &fontInfo);
	c->lineDraw(r.left, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2), r.right, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2));
	c->lineDraw(r.left, r.bottom - (INPUTHEIGHT + 2), r.right - REGWIDTH, r.bottom - (INPUTHEIGHT + 2));
	c->lineDraw(r.right - REGWIDTH, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2), r.right - REGWIDTH, r.bottom);
	c->lineDraw(r.right - RIGHTCOLUMN, 0, r.right - RIGHTCOLUMN, r.bottom - (INPUTHEIGHT + (NCONSOLELINES*LINEHEIGHT) + 2));
	disassemble(c);
	for (int s = 0;s < strstack.getNumItems();s++)
	{
		c->textOutEllipsed(r.right - RIGHTCOLUMN + 4, s*LINEHEIGHT + 4, RIGHTCOLUMN - 8, 16, strstack.enumItem(s)->getValue(), &fontInfo);
	}
	return 1;
}

int SimpleDebuggerUI::onLeftButtonDown(int x, int y)
{
	SimpleDebuggerUI_PARENT::onLeftButtonDown(x, y);
	if (edit) edit->onGetFocus();
//  leave = 1;
	return 1;
}

void SimpleDebuggerUI::disassemble(Canvas *c)
{
	RECT r;
	getClientRect(&r);
	int x = 4;

	int y = (r.bottom - r.top - 4) / 2 + 4;

	int ln = jitd->findLine(jitd->getVIP());
	if (ln != -1)
	{
		Wasabi::FontInfo fontInfo;
		SourceCodeLine *l = jitd->enumLine(ln);
		int sourcecode = l->getSourceFile() != NULL;
		int g;
		int j = 0;
		for (g = y;g < r.bottom - (INPUTHEIGHT + 4 + (NCONSOLELINES*LINEHEIGHT)) - LINEHEIGHT;g += LINEHEIGHT)
		{
			if (!sourcecode || j == 0)
				l = jitd->enumLine(ln + j);
			if (!l) break;

			if (j == 0)
			{
				RECT br;
				br.left = 4;
				br.top = g;
				br.right = r.right - (RIGHTCOLUMN + 4);
				br.bottom = br.top + LINEHEIGHT;
				c->fillRect(&br, RGB(0, 255, 0));
				fontInfo.color = RGB(0, 0, 0);
			}
			if (!sourcecode)
			{
				String str;
				unsigned const char *d = (unsigned const char *)(l->getPointer() + jitd->getCodeBlock());
				for (int k = 0;k < l->getLength();k++)
				{
					if (!str.isempty()) str += " ";
					str += StringPrintf("%02X", *d);
					d++;
				}
				c->textOut(x, g, StringPrintfW(L"%08X", l->getPointer()), &fontInfo);
				c->textOut(x + 70, g, AutoWide(str), &fontInfo);
				c->textOut(x + 70 + 150, g, l->getLine(), &fontInfo);
			}
			else
			{
				c->textOutEllipsed(x, g, r.right - r.left - (RIGHTCOLUMN + 4 + x), 16, (getLine(l->getSourceFile(), l->getSourceFileLine() + j)), &fontInfo);
			}

			j++;
		}
		j = 1;
		for (g = y - LINEHEIGHT;g > 1;g -= LINEHEIGHT)
		{
			if (!sourcecode || j == 0)
				l = jitd->enumLine(ln - j);
			if (!l) break;
			if (!sourcecode)
			{
				String str;
				unsigned const char *d = (unsigned const char *)(l->getPointer() + jitd->getCodeBlock());
				for (int k = 0;k < l->getLength();k++)
				{
					if (!str.isempty()) str += " ";
					str += StringPrintf("%02X", *d);
					d++;
				}
				c->textOut(x, g, StringPrintfW(L"%08X", l->getPointer()), &fontInfo);
				c->textOut(x + 70, g, AutoWide(str), &fontInfo);
				c->textOut(x + 70 + 150, g, (l->getLine()), &fontInfo);
			}
			else
			{
				c->textOutEllipsed(x, g, r.right - r.left - (RIGHTCOLUMN + 4 + x), 16, (getLine(l->getSourceFile(), l->getSourceFileLine() - j)), &fontInfo);
			}
			j++;
		}
	}
}

int SimpleDebuggerUI::onResize()
{
	SimpleDebuggerUI_PARENT::onResize();
	if (edit != NULL)
	{
		RECT r;
		getClientRect(&r);
		edit->resize(1, r.bottom - (INPUTHEIGHT + 1), r.right - r.left - (REGWIDTH + 1), INPUTHEIGHT);
	}
	return 1;
}

int SimpleDebuggerUI::childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2)
{
	if (child == edit)
	{
		if (msg == ChildNotify::EDITWND_ENTER_PRESSED)
		{
			onCommand(cmdbuf);
		}
	}
	return SimpleDebuggerUI_PARENT::childNotify(child, msg, p1, p2);
}

BEGIN_STRINGDICTIONARY(_debuggercommands)
SDI(L"b", DEBUG_CMD_BREAKPOINT);
SDI(L"break", DEBUG_CMD_BREAKPOINT);
SDI(L"x", DEBUG_CMD_CONTINUE);
SDI(L"continue", DEBUG_CMD_CONTINUE);
SDI(L"i", DEBUG_CMD_STEPINTO);
SDI(L"stepinto", DEBUG_CMD_STEPINTO);
SDI(L"o", DEBUG_CMD_STEPOVER);
SDI(L"stepover", DEBUG_CMD_STEPOVER);
SDI(L"p", DEBUG_CMD_STEPOUT);
SDI(L"stepout", DEBUG_CMD_STEPOUT);
SDI(L"k", DEBUG_CMD_KILL);
SDI(L"kill", DEBUG_CMD_KILL);
SDI(L"?", DEBUG_CMD_HELP);
SDI(L"help", DEBUG_CMD_HELP);
END_STRINGDICTIONARY(_debuggercommands, debuggercommands)


void SimpleDebuggerUI::onCommand(const wchar_t *cmd)
{
	if (*cmd == 0)
	{
		stepOver();
		return;
	}
	ParamParser pp(cmd, L" ");
	int i = debuggercommands.getId(pp.enumItem(0));
	switch (i)
	{
		case DEBUG_CMD_BREAKPOINT:
			addBreakPoint(pp.enumItem(1));
			break;
		case DEBUG_CMD_CONTINUE:
			continueExecution();
			break;
		case DEBUG_CMD_STEPINTO:
			stepInto();
			break;
		case DEBUG_CMD_STEPOVER:
			stepOver();
			break;
		case DEBUG_CMD_KILL:
			killScript();
			break;
		case DEBUG_CMD_HELP:
			showHelp();
			break;
	}
}

int SimpleDebuggerUI::evaluate(const wchar_t *ascii)
{
	if (!_wcsicmp(ascii, L"VSD")) return jitd->getVSD();
	if (!_wcsicmp(ascii, L"VIP")) return jitd->getVIP();
	if (!_wcsicmp(ascii, L"VSP")) return jitd->getVSP();
	if (!_wcsicmp(ascii, L"VCC")) return jitd->getVCC();
	wchar_t *end;
	return wcstol(ascii, &end, 16);
}

void SimpleDebuggerUI::addBreakPoint(const wchar_t *pointer_ascii)
{
	/*int i = */evaluate(pointer_ascii);
}

void SimpleDebuggerUI::continueExecution()
{
	retcode = JITD_RETURN_CONTINUE;
	leave = 1;
}

void SimpleDebuggerUI::stepInto()
{
	retcode = JITD_RETURN_STEPINTO;
	leave = 1;
}

void SimpleDebuggerUI::stepOver()
{
	int ln = jitd->findLine(jitd->getVIP());
	ln++;
	SourceCodeLine *l = jitd->enumLine(ln);

	if (l != NULL) // else ret as last opcode
		jitd->setSysBreakpoint(l->getPointer());

	retcode = JITD_RETURN_CONTINUE;
	leave = 1;
}

void SimpleDebuggerUI::killScript()
{
	retcode = JITD_RETURN_TERMINATE;
	leave = 1;
}

void SimpleDebuggerUI::showHelp()
{}

int SimpleDebuggerUI::onGetFocus()
{
	SimpleDebuggerUI_PARENT::onGetFocus();
	if (edit) edit->onGetFocus();
	return 1;
}

void SimpleDebuggerUI::onSetVisible(int show)
{
	SimpleDebuggerUI_PARENT::onSetVisible(show);
	if (edit) edit->onGetFocus();
}
#undef fgets
const wchar_t *SimpleDebuggerUI::getLine(const wchar_t *filename, int fileline)
{
	if (fileline <= 0)
		return L"";
	static StringW str;
	FILE *f = _wfopen(filename, L"rt");
	if (!f)
	{
		str = L"couldn't load ";
		str += filename;
		return str;
	}

	char t[256] = {0};
	char u[256] = {0};
	int n = fileline;
	while (n--)
	{
		*u = 0;
		char *p;
		do
		{
			p = *u ? t : u;
			fgets(p, 255, f);
			t[255] = 0;
		}
		while (!feof(f) && p[STRLEN(p)-1] != '\n' && p[STRLEN(p)-1] != '\r');
	}

	char *p = u;
	while (p && *p && p < u + 256)
	{
		if (*p < 0x21) *p = ' ';
		p++;
	}

	str = AutoWide(u, CP_UTF8);
	fclose(f);

	return str;
}

