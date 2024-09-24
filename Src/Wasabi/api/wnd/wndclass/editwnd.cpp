#include <precomp.h>
#include "editwnd.h"

#include <tataki/canvas/canvas.h>
#include <api/wnd/notifmsg.h>

#include <bfc/assert.h>

#define ID_EDITCHILD 12

enum { IDLETIMER = 8, DELETETIMER = 10 };
#define IDLETIME 350 // comprimises suck ;)


#if UTF8
#ifdef WANT_UTF8_WARNINGS
#pragma CHAT("mig", "all", "UTF8 is enabled in editwnd.cpp -- Things might be screwy till it's all debugged?")
#endif
# include <bfc/string/encodedstr.h>
#endif

#ifdef WIN32
#include <commctrl.h>
#endif


#ifdef WIN32
static LRESULT CALLBACK static_editWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EditWnd *editwnd = (EditWnd *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	if (editwnd == NULL) return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	return editwnd->editWndProc(hWnd, uMsg, wParam, lParam);
}
#endif

EditWnd::EditWnd(wchar_t *buffer, int buflen)
{
	wantfocus = 1;
	nextenterfaked = 0;
	idleenabled = 1;
	beforefirstresize = 1;
	editWnd = NULL;
	prevWndProc = NULL;
	setBuffer(buffer, buflen);
	maxlen = 0;
	retcode = EDITWND_RETURN_NOTHING;
	modal = 0;
	autoenter = 0;
	autoselect = 0;
	outbuf = NULL;
	//  bordered = 0;
	idletimelen = IDLETIME;
	multiline = 0;
	readonly = 0;
	password = 0;
	autohscroll = 1;
	autovscroll = 1;
	vscroll = 0;
#ifdef WIN32
	oldbrush = NULL;
#endif
#ifdef LINUX
	selstart = selend = 0;
	cursorpos = 0;
	selectmode = 0;
	viewstart = 0;
#endif
#ifdef WASABI_EDITWND_LISTCOLORS
	if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text"))
		textcolor = L"wasabi.list.text";
	else
		textcolor = L"wasabi.edit.text";
	if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.background"))
		backgroundcolor = L"wasabi.list.background";
	else
		textcolor.setColor(WASABI_API_SKIN->skin_getBitmapColor(L"wasabi.list.background"));
#else
	backgroundcolor = "wasabi.edit.background";
	textcolor = "wasabi.edit.text";
#endif
	selectioncolor = L"wasabi.edit.selection";
	setVirtual(0);
}

EditWnd::~EditWnd()
{
	killTimer(IDLETIMER);
#ifdef WIN32
	if (oldbrush != NULL)
		DeleteObject(oldbrush);
	oldbrush = NULL;
	if (editWnd != NULL)
	{
		SetWindowLong(editWnd, GWLP_USERDATA, (LONG_PTR)0);
		SetWindowLongPtrW(editWnd, GWLP_WNDPROC, (LONG_PTR)prevWndProc);
		DestroyWindow(editWnd);
	}
#endif
	notifyParent(ChildNotify::RETURN_CODE, retcode);
}

int EditWnd::onInit()
{
	EDITWND_PARENT::onInit();

#ifdef WIN32
	RECT r = clientRect();

		editWnd = CreateWindowW(L"EDIT", NULL,
		                       WS_CHILD
		                       | (autohscroll ? ES_AUTOHSCROLL : 0)
		                       | (readonly ? ES_READONLY : 0)
		                       | (multiline ? ES_MULTILINE : 0)
		                       | (password ? ES_PASSWORD : 0)
		                       | (autovscroll ? ES_AUTOVSCROLL : 0)
		                       | (vscroll ? WS_VSCROLL : 0),
		                       r.left, r.top, r.right - r.left, r.bottom - r.top,
		                       gethWnd(), (HMENU)ID_EDITCHILD,
		                       getOsModuleHandle(), NULL);
		ASSERT(editWnd != NULL);

	if ((maxlen != 0) && (outbuf != NULL))
	{
		setBuffer(outbuf, maxlen);
	}

	// stash a pointer to us
	SetWindowLongPtrW(editWnd, GWLP_USERDATA, (LONG_PTR)this);
	// subclass the edit control -- either by 8 or by 16

		prevWndProc = (WNDPROC)SetWindowLongPtrW(editWnd, GWLP_WNDPROC, (LONG_PTR)static_editWndProc);


	SendMessageW(editWnd, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), FALSE);
	ShowWindow(editWnd, !getStartHidden() ? SW_NORMAL : SW_HIDE);
#endif

	return 1;
}

void EditWnd::onSetVisible(int show)
{
	EDITWND_PARENT::onSetVisible(show);
	if (editWnd == NULL) return ;
#ifdef WIN32
	ShowWindow(editWnd, show ? SW_NORMAL : SW_HIDE);
#endif
}

int EditWnd::onPaint(Canvas *canvas)
{
	//  if (!bordered) return EDITWND_PARENT::onPaint(canvas);

	PaintCanvas paintcanvas;
	if (canvas == NULL)
	{
		if (!paintcanvas.beginPaint(this)) return 0;
		canvas = &paintcanvas;
	}
	EDITWND_PARENT::onPaint(canvas);

	RECT r;
	getClientRect(&r);
	canvas->fillRect(&r, backgroundcolor);	//SKIN

#ifdef LINUX
	char *str = STRDUP((const char *)inbuf + viewstart);
	canvas->setTextColor(textcolor);
	canvas->setTextSize(r.bottom - r.top);
	canvas->setTextOpaque(FALSE);
	char save;
	if (selstart != selend)
	{
		RECT selrect = r;
		int start = MAX(MIN(selstart, selend) - viewstart, 0);
		int end = MAX(MAX(selstart, selend) - viewstart, 0);

		save = str[ start ];
		str[start] = '\0';
		selrect.left = r.left + canvas->getTextWidth(str);
		str[start] = save;

		save = str[ end ];
		str[end] = '\0';
		selrect.right = r.left + canvas->getTextWidth(str);
		str[end] = save;

		canvas->fillRect(&selrect, selectioncolor);
	}

	save = str[cursorpos - viewstart];
	str[cursorpos - viewstart] = '\0';
	RECT cursor = r;
	cursor.left = cursor.right = r.left + canvas->getTextWidth(str);
	str[cursorpos - viewstart] = save;
	canvas->drawRect(&cursor, TRUE, 0xffffff);

	canvas->textOut(r.left, r.top, r.right - r.left, r.bottom - r.top, str);

	FREE(str);
#endif

	return 1;
}

int EditWnd::onResize()
{
	EDITWND_PARENT::onResize();
#ifdef WIN32
	RECT r = clientRect();
	if (1 /*bordered*/)
	{
		r.top++;
		r.bottom--;
		r.left++;
		r.right--;
	}
	MoveWindow(editWnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);

	if (beforefirstresize)
	{
		ShowWindow(editWnd, SW_NORMAL); beforefirstresize = 0;
		if (modal)
		{
			SetFocus(editWnd);
			if (getAutoSelect())
				SendMessageW(editWnd, EM_SETSEL, 0, -1);
		}
	}
#endif

	return TRUE;
}

#ifdef WIN32
LRESULT EditWnd::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CTLCOLOREDIT:
		{
			HDC hdc = (HDC)wParam;
			SetTextColor(hdc, textcolor);
			SetBkColor(hdc, backgroundcolor);
			if (oldbrush != NULL)
			{
				DeleteObject(oldbrush);
				oldbrush = NULL;
			}
			oldbrush = CreateSolidBrush(backgroundcolor);
			return (LRESULT)oldbrush;
		}

	case WM_MOUSEACTIVATE:
		WASABI_API_WND->popupexit_check(this);
		break;

	case WM_COMMAND:
		{
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				{
					if (maxlen > 0)
					{
						GetWindowTextW(editWnd, outbuf, maxlen);
						onEditUpdate();
					}
				}
				break;
			case EN_SETFOCUS:
				if (getAutoSelect())
					SendMessageW(editWnd, EM_SETSEL, (WPARAM)0, (LPARAM) - 1);
				break;
			case EN_KILLFOCUS:
				onLoseFocus();
				break;
			}
		}
		break;
	}

	return EDITWND_PARENT::wndProc(hWnd, uMsg, wParam, lParam);
}
#endif

void EditWnd::setBuffer(wchar_t *buffer, int len)
{
#ifdef LINUX
	if (buffer == NULL || len <= 1)
	{
		inbuf = "";
		return ;
	}
#endif
	if (buffer == NULL || len <= 1) return ;
	ASSERT(len > 1);
	ASSERT(len < 0x7ffe);
	ASSERT((int)wcslen(buffer) <= len);

#ifdef WIN32
#define USE_INTERNAL_BUFFER 0

#if USE_INTERNAL_BUFFER
	buffer8.setSize(len + 1);
	outbuf = buffer8.getMemory();
	if (len)
	{
		STRNCPY(outbuf, buffer, len);
	}
	outbuf[len] = 0;
#else
	outbuf = buffer;
#endif

	if (editWnd != NULL)
	{
				SetWindowTextW(editWnd, buffer);

		// This is going to be problematic.  This is where utf8 sucks.
		// Just how many characters CAN we save in our buffer, eh?
		// (shrug) Oh well.  Can't be helped.   At most this many.
		SendMessageW(editWnd, EM_LIMITTEXT, (WPARAM)len - 1, (LPARAM)0);
		// hooray for halcyon7

		/*    if (getAutoSelect()) {
		      SetFocus(editWnd);
		      SendMessageW(editWnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		    }*/
	}

	maxlen = len;
#else
	outbuf = buffer;
	maxlen = len;
	inbuf = buffer;
	cursorpos = len;
	invalidate();
#endif
}

void EditWnd::selectAll()
{
#ifdef WIN32
	PostMessage(editWnd, EM_SETSEL, 0, -1);
#else
	selstart = 0; selend = inbuf.len();
#endif
}

void EditWnd::enter()
{
	onEnter();
}

void EditWnd::getBuffer(wchar_t *buf, int _len)
{
	if (_len > maxlen) _len = maxlen;
	//  SendMessageW(editWnd, WM_GETTEXT, (WPARAM)_len, (LPARAM)buf);
	WCSCPYN(buf, outbuf, _len);
}

void EditWnd::setModal(int _modal)
{
	modal = _modal;
}

void setBorder(int border)
{
	//  bordered = border;
}

int EditWnd::isEditorKey(int vk)
{
	if (vk >= VK_F1) return 0;
	if ((vk == VK_UP || vk == VK_DOWN) || ((Std::keyDown(VK_CONTROL) || Std::keyDown(VK_MENU)) && (vk == VK_LEFT || vk == VK_RIGHT)))
		return 0;
	if (vk == VK_RETURN && Std::keyDown(VK_CONTROL)) return 0;
	if (vk == VK_CONTROL || vk == VK_MENU) return 0;
	return 1;
}

LRESULT EditWnd::editWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (!isEditorKey((int)wParam) && !onKeyDown((int)wParam))
		{
#ifdef WASABI_COMPILE_WND
			WASABI_API_WND->forwardOnKeyDown(this, (int) wParam, (int)lParam);
#endif

		}
		break;
	case WM_KEYUP:
		if (!isEditorKey((int)wParam) && !onKeyUp((int)wParam))
		{
#ifdef WASABI_COMPILE_WND
			WASABI_API_WND->forwardOnKeyUp(this, (int) wParam, (int)lParam);
#endif

		}
		break;
	case WM_CHAR:
		if (!(wParam == VK_RETURN && nextenterfaked && !autoenter))
		{
			notifyParent(ChildNotify::EDITWND_KEY_PRESSED, wParam);
			onChar((TCHAR)wParam);
		}
		if (wParam == VK_RETURN)
		{
			if (!(nextenterfaked && !autoenter))
				if (onEnter()) return 0;
			nextenterfaked = 0;
			return 0;
		}
		else if (wParam == VK_ESCAPE)
		{
			if (onAbort()) return 0;
		}
		else if (wParam == VK_TAB && multiline)
		{
			return 0;
		}
		break;
	case WM_SETFOCUS:
		onSetRootFocus(this);
		// fall thru
	case WM_KILLFOCUS:
		invalidate();
		break;
	}
#ifdef WIN32
	return CallWindowProc(prevWndProc, hWnd, uMsg, wParam, lParam);
#else
	DebugString("portme -- EditWnd::editWndProc\n");
	return 0;
#endif
}

void EditWnd::timerCallback(int id)
{
	switch (id)
	{
	case IDLETIMER:
		killTimer(IDLETIMER);
		if (idleenabled) onIdleEditUpdate();
		break;
	case DELETETIMER:
		killTimer(DELETETIMER);
		delete this;
		break;
	default:
		EDITWND_PARENT::timerCallback(id);
	}
}

void EditWnd::onEditUpdate()
{
#ifdef LINUX
	STRNCPY(outbuf, inbuf, maxlen);
	outbuf[maxlen] = '\0';

	RECT r;
	getClientRect(&r);

	SysCanvas sysc;
	sysc.setTextSize(r.bottom - r.top);
	sysc.getTextWidth(inbuf);

	char *str = STRDUP(inbuf);

	if (cursorpos < viewstart)
		viewstart = cursorpos;

	char save = str[cursorpos];
	str[cursorpos] = '\0';
	while (sysc.getTextWidth(str + viewstart) > r.right - r.left)
	{
		viewstart++;
	}
	str[cursorpos] = save;

	invalidate();
#endif
	killTimer(IDLETIMER);
	setTimer(IDLETIMER, idletimelen);
	notifyParent(ChildNotify::EDITWND_DATA_MODIFIED);
}

void EditWnd::onIdleEditUpdate()
{
	notifyParent(ChildNotify::EDITWND_DATA_MODIFIED_ONIDLE);
}

int EditWnd::onEnter()
{
	notifyParent(ChildNotify::EDITWND_ENTER_PRESSED);
	if (modal)
	{
		retcode = EDITWND_RETURN_OK;
		delete this;
		//CUT    setTimer(DELETETIMER, 1);
		return 1;
	}
	return 0;
}

int EditWnd::onAbort()
{
	notifyParent(ChildNotify::EDITWND_CANCEL_PRESSED);
	if (modal)
	{
		retcode = EDITWND_RETURN_CANCEL;
		delete this;
		//CUT    setTimer(DELETETIMER, 1);
		return 1;
	}
	return 0;
}

int EditWnd::onLoseFocus()
{	// fake an onEnter()
#ifdef WIN32
	if (autoenter)
	{
		nextenterfaked = 1;
		PostMessage(editWnd, WM_CHAR, VK_RETURN, 0);
	}
#else
	invalidate();
	selstart = selend = 0;
#endif
	return 0;
}

void EditWnd::setAutoEnter(int a)
{
	autoenter = a;
}

void EditWnd::setAutoSelect(int a)
{
	autoselect = a;
};

void EditWnd::setIdleTimerLen(int ms)
{
	if (ms < 0) ms = 0;
	idletimelen = ms;
}

int EditWnd::getTextLength()
{ // TOTALLY NONPORTABLE AND TOTALLY DIRTY
#ifdef WIN32
	HFONT font = (HFONT)SendMessageW(editWnd, WM_GETFONT, 0, 0);

	HDC sdc = GetDC(NULL);
	HDC dc = CreateCompatibleDC(sdc);
	ReleaseDC(NULL, sdc);

	HFONT oldfont = (HFONT)SelectObject(dc, font);

	SIZE s;
	GetTextExtentPoint32W(dc, outbuf, wcslen(outbuf), &s);
	SelectObject(dc, oldfont);
	DeleteDC(dc);
	return s.cx + SendMessageW(editWnd, EM_GETMARGINS, 0, 0)*2 + 2;
#else
	if (inbuf.isempty())
		return 0;

	RECT r;
	getClientRect(&r);

	SysCanvas sysc;
	sysc.setTextSize(r.bottom - r.top);
	return sysc.getTextWidth(inbuf);
#endif
}

HWND EditWnd::getEditWnd()
{
	return editWnd;
}

#ifndef WIN32
enum {
    ES_MULTILINE,
    ES_WANTRETURN,
    ES_AUTOHSCROLL,
    ES_AUTOVSCROLL,
    WS_VSCROLL,
};
#endif

void EditWnd::setMultiline(int ml)
{
	multiline = ml;
	setStyle(ES_MULTILINE | ES_WANTRETURN, ml);
}

void EditWnd::setReadOnly(int ro)
{
	readonly = ro;
	setStyle(ES_READONLY, ro);
}

void EditWnd::setPassword(int pw)
{
	password = pw;
	setStyle(ES_PASSWORD, pw);
}

void EditWnd::setAutoHScroll(int hs)
{
	autohscroll = hs;
	setStyle(ES_AUTOHSCROLL, hs);
}

void EditWnd::setAutoVScroll(int vs)
{
	autovscroll = vs;
	setStyle(ES_AUTOVSCROLL, vs);
}

void EditWnd::setVScroll(int vs)
{
	vscroll = vs;
	setStyle(WS_VSCROLL, vs);
}

void EditWnd::setStyle(LONG style, int set)
{
#ifdef WIN32
	if (editWnd)
	{
		LONG s = GetWindowLong(editWnd, GWL_STYLE);
		if (set) s |= style;
		else s &= ~style;
		SetWindowLong(editWnd, GWL_STYLE, s);
	}
#else
	DebugString("portme -- EditWnd::setStyle\n");
#endif
}

int EditWnd::onGetFocus()
{
	int r = EDITWND_PARENT::onGetFocus();
#ifdef WIN32
	if (editWnd != NULL)
		SetFocus(editWnd);
#endif
	return r;
}

int EditWnd::wantFocus()
{
	return wantfocus;
}

int EditWnd::gotFocus()
{
	return (GetFocus() == editWnd);
}

void EditWnd::setBackgroundColor(COLORREF c)
{
	backgroundcolor.setColor(c);
}

void EditWnd::setTextColor(COLORREF c)
{
	textcolor.setColor(c);
}

void EditWnd::invalidate()
{
	EDITWND_PARENT::invalidate();
	InvalidateRect(editWnd, NULL, TRUE);
}

#ifdef LINUX
int EditWnd::textposFromCoord(int x, int y)
{
	RECT r;
	getClientRect(&r);

	SysCanvas canvas;

	canvas.setTextColor(textcolor);
	canvas.setTextSize(r.bottom - r.top);
	canvas.setTextOpaque(FALSE);

	x -= r.left;

	int i;

	char *str = STRDUP(inbuf);

	if (x > canvas.getTextWidth(str + viewstart))
		return inbuf.len();

	for (i = viewstart + 1; str[i]; i++)
	{
		char save = str[i];
		str[i] = '\0';
		if (x < canvas.getTextWidth(str + viewstart))
		{
			str[i] = save;
			break;
		}
		str[i] = save;
	}

	FREE(str);

	return i - 1;
}

int EditWnd::onLeftButtonDown(int x, int y)
{
	EDITWND_PARENT::onLeftButtonDown(x, y);

	// Add check for double/triple click...

	cursorpos = textposFromCoord(x, y);
	selstart = selend = cursorpos;

	selectmode = 1;

	return 1;
}

int EditWnd::onLeftButtonUp(int x, int y)
{
	EDITWND_PARENT::onLeftButtonUp(x, y);

	selectmode = 0;

	return 1;
}

int EditWnd::onMouseMove(int x, int y)
{
	EDITWND_PARENT::onMouseMove(x, y);

	switch (selectmode)
	{
	case 0:
		// Do nothing
		break;
	case 1:
		selend = textposFromCoord(x, y);
		cursorpos = selend;
		onEditUpdate();
		break;
	default:
		DebugString("selectmode %d not available\n", selectmode);
		break;
	}

	return selectmode;
}

int EditWnd::onKeyDown(int key)
{
	EDITWND_PARENT::onKeyDown(key);

	if (Std::keyDown(VK_CONTROL))
	{
		switch (key)
		{
		case 'a':
		case 'A':
			selectAll();
			break;

		default:
			return 0;
		}
	}
	else
	{
		switch (key)
		{
		case XK_Home:
			if (Std::keyDown(VK_SHIFT))
			{
				if (selstart == selend)
				{
					selstart = selend = cursorpos;
				}
				selend = 0;
			}
			else
			{
				selstart = selend = 0;
			}
			cursorpos = 0;
			break;

		case XK_End:
			if (Std::keyDown(VK_SHIFT))
			{
				if (selstart == selend)
				{
					selstart = selend = cursorpos;
				}
				selend = inbuf.len();
			}
			else
			{
				selstart = selend = 0;
			}
			cursorpos = inbuf.len();
			break;

		case XK_Right:
			if (Std::keyDown(VK_SHIFT))
			{
				if (selstart == selend)
				{
					selstart = selend = cursorpos;
				}
				selend++;
				if (selend > inbuf.len()) selend = inbuf.len();
			}
			else
			{
				selstart = selend = 0;
			}
			cursorpos++;
			if (cursorpos > inbuf.len()) cursorpos = inbuf.len();
			break;

		case XK_Left:
			if (Std::keyDown(VK_SHIFT))
			{
				if (selstart == selend)
				{
					selstart = selend = cursorpos;
				}
				selend--;
				if (selend < 0) selend = 0;
			}
			else
			{
				selstart = selend = 0;
			}
			cursorpos--;
			if (cursorpos < 0) cursorpos = 0;
			break;

		case XK_Escape:
			onAbort();
			break;

		case XK_Return:
			onEnter();
			break;

		case XK_Delete:
			if (selstart != selend)
			{
				int start = MIN(selstart, selend);
				int end = MAX(selstart, selend);

				String add;

				if (end < inbuf.len())
				{
					add = (const char *)inbuf + end;
				}
				else
				{
					add = "";
				}

				inbuf.trunc(start);
				inbuf += add;

				cursorpos = start;
				selstart = selend = 0;

			}
			else
			{
				if (cursorpos >= 0)
				{
					if (cursorpos < inbuf.len() - 1)
					{
						String tmp = inbuf;
						tmp.trunc(cursorpos);
						inbuf = tmp + ((const char *)inbuf + cursorpos + 1);
					}
					else if (cursorpos == inbuf.len() - 1)
					{
						inbuf.trunc(cursorpos);
					}
				}
			}
			break;

		case VK_BACK:
			if (selstart != selend)
			{
				int start = MIN(selstart, selend);
				int end = MAX(selstart, selend);

				String add;

				if (end < inbuf.len())
				{
					add = (const char *)inbuf + end;
				}
				else
				{
					add = "";
				}

				inbuf.trunc(start);
				inbuf += add;

				cursorpos = start;
				selstart = selend = 0;
			}
			else
			{
				if (cursorpos > 0)
				{
					if (cursorpos >= inbuf.len())
					{
						inbuf.trunc(cursorpos - 1);
						cursorpos--;
					}
					else
					{
						String tmp = inbuf;
						tmp.trunc(cursorpos - 1);
						inbuf = tmp + ((const char *)inbuf + cursorpos);
						cursorpos--;
					}
				}
			}
			break;

		default:
			if (key < 0x20 || key > 0x7e)
				return 0;

			if (selstart != selend)
			{
				int start = MIN(selstart, selend);
				int end = MAX(selstart, selend);

				String add;

				if (end < inbuf.len())
				{
					add = (const char *)inbuf + end;
				}
				else
				{
					add = "";
				}

				inbuf.trunc(start);
				inbuf += add;

				cursorpos = start;
				selstart = selend = 0;
			}

			String tmp;

			if (cursorpos >= inbuf.len())
			{
				tmp = "";
			}
			else
			{
				tmp = (const char *)inbuf + cursorpos;
			}

			inbuf.trunc(cursorpos);

			inbuf += (char)key;
			inbuf += tmp;

			cursorpos++;
		}
	}

	onEditUpdate();

	return 1;
}
#endif
