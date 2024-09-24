#include <precomp.h>
#include "msgbox.h"
#include <api/wndmgr/skinwnd.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/scriptguid.h>
#include <api/script/objects/c_script/c_container.h>
#include <api/script/objects/c_script/c_text.h>
#include <api/script/objects/c_script/c_button.h>

_btnstruct msgboxbtns[] =
    {
        { L"<<", MSGBOX_PREVIOUS },
        { L"OK", MSGBOX_OK },
        { L"Yes", MSGBOX_YES },
        { L"All", MSGBOX_ALL },
        { L"No", MSGBOX_NO },
        { L">>", MSGBOX_NEXT },
        { L"Cancel", MSGBOX_CANCEL },
    };

MsgBox::MsgBox(const wchar_t *_text, const wchar_t *_title, int _flags, const wchar_t *notanymore)
{
	text = _text;
	title = _title;
	flags = _flags;
#ifdef WIN32
#ifdef _DEBUG
	DebugStringW(L"msgbox: %s: %s", title, text.getValue());
#endif
#endif
	notanymore_id = notanymore;
	sw = NULL;
	if (flags == 0) flags = MSGBOX_OK;
}

MsgBox::~MsgBox()
{
	foreach(buttons)
	WASABI_API_SKIN->xui_delete(buttons.getfor());
	endfor
}

int MsgBox::run()
{
	int _r = -1;

#ifdef WASABI_COMPILE_CONFIG
	if (!notanymore_id.isempty())
	{
		StringPrintfW txt(L"msgbox_defaultvalue_%s", notanymore_id);
		if ((GetKeyState(VK_SHIFT) & 0x8000))
			WASABI_API_CONFIG->setIntPublic(txt, -1);
		_r = WASABI_API_CONFIG->getIntPublic(txt, -1);
	}
#endif

	if (_r == -1)
	{
		sw = new SkinWnd(L"msgbox.custom.group", L"modal", FALSE, NULL, 1, 1);
		if (!sw->getWindow()) return -1;
		ifc_window *grp = sw->getWindow();
		ifc_window *l = grp->getDesktopParent();

		GuiObject *_p = sw->findObject(L"msgbox.custom.group");
		//CUT: api_window *p = _p->guiobject_getRootWnd();

		C_Container cont(sw->getContainer());
		cont.setName(title);

		createButtons();
		int min_w = reposButtons();

		GuiObject *go_txt = sw->findObject(L"msgbox.text");
		if (go_txt != NULL)
		{
			GuiObject *to = go_txt->guiobject_findObject(L"wasabi.text");
			GuiObject *gto = sw->findObject(L"text");
			if (to != NULL && gto != NULL)
			{
				go_txt->guiobject_setXmlParam(L"text", text);
				C_GuiObject t(*go_txt);
				int _w = t.getAutoWidth();
				int _h = t.getAutoHeight();

				to->guiobject_setXmlParam(L"w", StringPrintfW(_w));
				to->guiobject_setXmlParam(L"h", StringPrintfW(_h));
				to->guiobject_setXmlParam(L"relatw", L"0");
				to->guiobject_setXmlParam(L"relath", L"0");

				int x, rx, w, rw;
				int y, ry, h, rh;
				int gtow, gtoh;

				go_txt->guiobject_getGuiPosition(&x, &y, &w, &h, &rx, &ry, &rw, &rh);
				if (rw == 1)
					_w += -w;
				else
					_w += x * 2;
				if (rh == 1)
					_h += -h;
				else
					_h += y * 2;

				gtow = _w;
				gtoh = _h;

				GuiObject *grpo = grp->getGuiObject();
				ASSERT(grpo != NULL);
				gto->guiobject_getGuiPosition(&x, &y, &w, &h, &rx, &ry, &rw, &rh);
				if (rw == 1)
					_w += -w;
				else
					_w += x * 2;
				if (rh == 1)
					_h += -h;
				else
					_h += y * 2;

				gto->guiobject_setXmlParam(L"w", StringPrintfW(gtow));
				gto->guiobject_setXmlParam(L"h", StringPrintfW(gtoh));

				grpo->guiobject_setXmlParam(L"w", StringPrintfW(_w));
				grpo->guiobject_setXmlParam(L"h", StringPrintfW(_h));
				grpo->guiobject_setXmlParam(L"lockminmax", L"1");
				grpo->guiobject_setXmlParam(L"propagatesize", L"1");
				XmlObject *xl = static_cast<XmlObject *>(l->getInterface(xmlObjectGuid));
				xl->setXmlParam(L"minimum_h", StringPrintfW(L"%d", _h));
				xl->setXmlParam(L"minimum_w", StringPrintfW(L"%d", (_w < min_w) ? min_w : _w));
			}
		}

		if (!notanymore_id.isempty())
		{
			GuiObject *o = WASABI_API_SKIN->xui_new(L"Wasabi:CheckBox"); // that'll be deleted automatically when our parent group destroys
			if (o != NULL)
			{
				ifc_window *w = o->guiobject_getRootWnd();
				C_GuiObject go(*o);
				go.init(_p->guiobject_getScriptObject());
				o->guiobject_setXmlParam(L"text", L"Do not show this message anymore");
				o->guiobject_setXmlParam(L"y", L"-50");
				o->guiobject_setXmlParam(L"relaty", L"1");
				o->guiobject_setXmlParam(L"x", L"12");
				o->guiobject_setXmlParam(L"relatx", L"0");
				o->guiobject_setXmlParam(L"w", L"-24");
				o->guiobject_setXmlParam(L"relatw", L"1");
				min_w = MAX(w->getPreferences(SUGGESTED_W), min_w);
			}
		}

		sw->notifyMinMaxChanged();
		reposButtons();
		_r = sw->runModal(1);
	}

#ifdef WASABI_COMPILE_CONFIG
	if (!notanymore_id.isempty() && _r != -1 && sw != NULL)
	{
		GuiObject *o = sw->findObject(L"checkbox.toggle");
		if (o != NULL)
		{
			C_Button b(*o);
			if (b.getActivated())
			{
				StringPrintfW txt(L"msgbox_defaultvalue_%s", notanymore_id);
				WASABI_API_CONFIG->setIntPublic(txt, _r);
			}
		}
	}
#endif

	if (sw) sw->destroy(); sw = NULL;

	return _r;
}

void MsgBox::addButton(const wchar_t *text, int retcode)
{
	GuiObject *o = WASABI_API_SKIN->xui_new(L"Wasabi:Button"); // that wil NOT be deleted automatically when our parent group destroys because we did not init with guiobject
	if (o != NULL)
	{
		o->guiobject_setXmlParam(L"action", L"endmodal");
		o->guiobject_setXmlParam(L"retcode", StringPrintfW(retcode));
		o->guiobject_setXmlParam(L"text", text);
		buttons.addItem(o);
	}
}

void MsgBox::createButtons()
{
	GuiObject *_p = sw->findObject(L"msgbox.custom.group");
	if (!_p) return ;

	ASSERT(buttons.getNumItems() == 0);
	buttons.deleteAll();

	for (int i = 0;i < sizeof(msgboxbtns) / sizeof(_btnstruct);i++)
	{
		if (flags & msgboxbtns[i].id)
		{
			addButton(msgboxbtns[i].txt, msgboxbtns[i].id);
		}
	}

	ifc_window *p = _p->guiobject_getRootWnd();

	foreach(buttons)
	ifc_window *wnd = buttons.getfor()->guiobject_getRootWnd();
	if (wnd != NULL)
	{
		wnd->setStartHidden(1);
		wnd->setParent(p);
		wnd->init(p);
	}
	endfor;
}

int MsgBox::reposButtons()
{
	RECT r;
	GuiObject *_p = sw->findObject(L"msgbox.custom.group");
	ifc_window *p = _p->guiobject_getRootWnd();
	p->getClientRect(&r);

	int shift = 0;

	//CUT: int _w = 0;
	//CUT: int _h = 0;
	for (int i = buttons.getNumItems() - 1;i >= 0;i--)
	{
		ifc_window *wnd = buttons.enumItem(i)->guiobject_getRootWnd();
		if (wnd != NULL)
		{
			int _w = wnd->getPreferences(SUGGESTED_W);
			int _h = wnd->getPreferences(SUGGESTED_H);
			if (_w == AUTOWH) _w = -1;
			int w = MAX(_w, 64);
			wnd->resize(r.right - w - 16 - shift, r.bottom - 8 - _h, w, _h);
			_w = MAX(w, _w);
			_h = MAX(_h, _h);
			shift += w + 4;
		}
	}

	foreach(buttons)
	ifc_window *wnd = buttons.getfor()->guiobject_getRootWnd();
	if (wnd != NULL)
		wnd->setVisible(1);
	endfor;
	return shift;
}
