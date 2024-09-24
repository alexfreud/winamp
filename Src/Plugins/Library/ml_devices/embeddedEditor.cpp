#include "main.h"
#include "./embeddedEditor.h"
#include <vector>

#define  EMBEDDEDEDITOR_FRAME_LEFT		1
#define  EMBEDDEDEDITOR_FRAME_TOP		1
#define  EMBEDDEDEDITOR_FRAME_RIGHT		1
#define  EMBEDDEDEDITOR_FRAME_BOTTOM	1

#define  EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_LEFT		1
#define  EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_TOP		1
#define  EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_RIGHT	1
#define  EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_BOTTOM	1

#define  EMBEDDEDEDITOR_FRAME_INNER_SPACE_LEFT		0
#define  EMBEDDEDEDITOR_FRAME_INNER_SPACE_TOP		0
#define  EMBEDDEDEDITOR_FRAME_INNER_SPACE_RIGHT		0
#define  EMBEDDEDEDITOR_FRAME_INNER_SPACE_BOTTOM	0

#define  EMBEDDEDEDITOR_MARGIN_LEFT					4
#define  EMBEDDEDEDITOR_MARGIN_TOP					1
#define  EMBEDDEDEDITOR_MARGIN_RIGHT				4
#define  EMBEDDEDEDITOR_MARGIN_BOTTOM				1


#define  EMBEDDEDEDITOR_BORDER_LEFT		(EMBEDDEDEDITOR_FRAME_LEFT + \
										 EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_LEFT + \
										 EMBEDDEDEDITOR_FRAME_INNER_SPACE_LEFT)

#define  EMBEDDEDEDITOR_BORDER_TOP		(EMBEDDEDEDITOR_FRAME_TOP + \
										 EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_TOP + \
										 EMBEDDEDEDITOR_FRAME_INNER_SPACE_TOP)

#define  EMBEDDEDEDITOR_BORDER_RIGHT	(EMBEDDEDEDITOR_FRAME_RIGHT + \
										 EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_RIGHT + \
										 EMBEDDEDEDITOR_FRAME_INNER_SPACE_RIGHT)

#define  EMBEDDEDEDITOR_BORDER_BOTTOM	(EMBEDDEDEDITOR_FRAME_BOTTOM + \
										 EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_BOTTOM + \
										 EMBEDDEDEDITOR_FRAME_INNER_SPACE_BOTTOM)


typedef struct EmbeddedEditor
{
	WNDPROC originalProc;
	COLORREF textColor;
	COLORREF backColor;
	COLORREF borderColor;
	HBRUSH backBrush;
	HBRUSH borderBrush;
	EmbeddedEditorFinishCb callback; 
	void *user;
	POINT anchorPoint;
	SIZE  maximumSize;
	wchar_t *buffer;
	size_t bufferSize;
	long spacing;
	long lineHeight;
} EmbeddedEditor;

typedef std::vector<HWND> WindowList;

typedef struct EmbeddedEditorParent
{
	WNDPROC originalProc;
	WindowList editorList;
} EmbeddedEditorParent;


typedef struct EmbeddedEditorThread
{
	HHOOK hook;
	HWND window;
} EmbeddedEditorThread;

static size_t editorTls = ((size_t)-1);

static ATOM EMBEDDEDEDITOR_PROP = 0;

#define EMBEDDEDEDITOR(_hwnd) ((EmbeddedEditor*)GetPropW((_hwnd), MAKEINTATOM(EMBEDDEDEDITOR_PROP)))
#define EMBEDDEDEDITOR_RET_VOID(_self, _hwnd) { (_self) = EMBEDDEDEDITOR((_hwnd)); if (NULL == (_self)) return; }
#define EMBEDDEDEDITOR_RET_VAL(_self, _hwnd, _error) { (_self) = EMBEDDEDEDITOR((_hwnd)); if (NULL == (_self)) return (_error); }

#define EMBEDDEDEDITOR_PARENT(_hwnd) ((EmbeddedEditorParent*)GetPropW((_hwnd), MAKEINTATOM(EMBEDDEDEDITOR_PROP)))

static LRESULT WINAPI 
EmbeddedEditor_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI 
EmbeddedEditorParent_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK
EmbeddidEditorThread_MouseProc(int code, unsigned int messageId, MOUSEHOOKSTRUCT *mouse);

static BOOL
EmbeddidEditorThread_BeginMouseMonitor(HWND editorWindow)
{
	EmbeddedEditorThread *threadData;

	if (NULL == WASABI_API_APP ||
		NULL == editorWindow)
	{
		return FALSE;
	}
	
	if ((size_t)-1 == editorTls)
	{
		editorTls = WASABI_API_APP->AllocateThreadStorage();
		if ((size_t)-1 == editorTls)
			return FALSE;

		threadData = NULL;
	}
	else
	{
		threadData = (EmbeddedEditorThread*)WASABI_API_APP->GetThreadStorage(editorTls);
		WASABI_API_APP->SetThreadStorage(editorTls, NULL);
	}
	
	if (NULL != threadData)
	{
		if (NULL != threadData->window)
			DestroyWindow(threadData->window);
	}
	else
	{
		threadData = (EmbeddedEditorThread*)malloc(sizeof(EmbeddedEditorThread));
		if (NULL == threadData)
			return FALSE;

		threadData->hook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)EmbeddidEditorThread_MouseProc, NULL, GetCurrentThreadId());
		if (NULL == threadData->hook)
		{
			free(threadData);
			return FALSE;
		}

	}

	threadData->window = editorWindow;
	WASABI_API_APP->SetThreadStorage(editorTls, threadData);
		
	return TRUE;
}

static void
EmbeddidEditorThread_EndMouseMonitor(HWND editorWindow)
{
	EmbeddedEditorThread *threadData;

	if (NULL == WASABI_API_APP ||
		(size_t)-1 == editorTls ||
		NULL == editorWindow)
	{
		return;
	}
	
	threadData = (EmbeddedEditorThread*)WASABI_API_APP->GetThreadStorage(editorTls);
	WASABI_API_APP->SetThreadStorage(editorTls, NULL);
	
	if (NULL != threadData)
	{
		if (NULL != threadData->hook)
			UnhookWindowsHookEx(threadData->hook);

		free(threadData);
	}
}

static BOOL
EmbeddedEditorParent_AddChild(HWND hwnd, HWND child)
{
	EmbeddedEditorParent *self;

	if (NULL == hwnd || NULL == child)
		return FALSE;

	self = EMBEDDEDEDITOR_PARENT(hwnd);

	if (NULL == self)
	{
		self = new EmbeddedEditorParent();
		if (NULL == self)
			return FALSE;

		self->originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC,
												(LONGX86)(LONG_PTR)EmbeddedEditorParent_WindowProc);
	
		if (NULL != self->originalProc && 
			FALSE == SetProp(hwnd, MAKEINTATOM(EMBEDDEDEDITOR_PROP), self))
		{
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)self->originalProc);
			delete self;
			return FALSE;
		}
	}
	else
	{
		size_t index;
		index = self->editorList.size();
		while(index--)
		{
			if (self->editorList[index] == child)
				return FALSE;
		}
	}

	self->editorList.push_back(child);
	return TRUE;
}

static BOOL
EmbeddedEditorParent_RemoveChild(HWND hwnd, HWND child)
{
	size_t index;
	EmbeddedEditorParent *self;

	if (NULL == hwnd || NULL == child)
		return FALSE;

	self = EMBEDDEDEDITOR_PARENT(hwnd);
	if (NULL == self)
		return FALSE;

	index = self->editorList.size();
	while(index--)
	{
		if (self->editorList[index] == child)
			break;
	}

	if (((size_t)-1) == index)
		return FALSE;
	
	self->editorList.erase(self->editorList.begin() + index);
	if (0 == self->editorList.size())
	{
		RemoveProp(hwnd, MAKEINTATOM(EMBEDDEDEDITOR_PROP));

		if (NULL != self->originalProc)
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)self->originalProc);
		
		delete self;
	}

	return TRUE;

}

BOOL
EmbeddedEditor_Attach(HWND hwnd, EmbeddedEditorFinishCb callback, void *user)
{
	HWND parent;
	EmbeddedEditor *self;

	if (NULL == hwnd)
		return FALSE;

	if (0 == EMBEDDEDEDITOR_PROP)
	{
		EMBEDDEDEDITOR_PROP = GlobalAddAtom(TEXT("EmdeddedEditorProp"));
		if (0 == EMBEDDEDEDITOR_PROP)
			return FALSE;
	}
	
	self = (EmbeddedEditor*)malloc(sizeof(EmbeddedEditor));
	if (NULL == self)
		return FALSE;

	memset(self, 0, sizeof(EmbeddedEditor));

	self->originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC,
												(LONGX86)(LONG_PTR)EmbeddedEditor_WindowProc);
	
	if (NULL != self->originalProc && 
		FALSE == SetProp(hwnd, MAKEINTATOM(EMBEDDEDEDITOR_PROP), self))
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)self->originalProc);
		free(self);
		return FALSE;
	}

	self->callback = callback;
	self->user = user;

	self->backColor = RGB(0, 0, 255); //GetSysColor(COLOR_WINDOW);
	self->backBrush = NULL;
	self->textColor = RGB(255, 255, 0); //GetSysColor(COLOR_WINDOWTEXT);
	self->borderColor = RGB(255, 0, 0); //GetSysColor(COLOR_WINDOWFRAME);
	self->borderBrush = NULL;
	self->spacing = -1;
	self->lineHeight = -1;

	parent = GetAncestor(hwnd, GA_PARENT);
	if(NULL != parent)
		EmbeddedEditorParent_AddChild(parent, hwnd);

	EmbeddidEditorThread_BeginMouseMonitor(hwnd);
		
	RECT rect, formatRect;
	GetWindowRect(hwnd, &rect);
	SetRect(&formatRect, 0, 0, RECTWIDTH(rect) - 6, RECTHEIGHT(rect) - 6);
	SendMessage(hwnd, EM_SETRECTNP, 0, (LPARAM)&formatRect);

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

	
	return TRUE;
}

BOOL
EmbeddedEditor_AdjustWindowRectEx(RECT *rect, unsigned long styleEx, unsigned long style)
{
	if (NULL == rect)
		return FALSE;

	if (0 != ((WS_EX_STATICEDGE | WS_EX_CLIENTEDGE) & styleEx) || 
		0 != (WS_BORDER & style))
	{
		rect->left -= (EMBEDDEDEDITOR_BORDER_LEFT + EMBEDDEDEDITOR_MARGIN_LEFT);
		rect->top -= (EMBEDDEDEDITOR_BORDER_TOP + EMBEDDEDEDITOR_MARGIN_TOP);
		rect->right += (EMBEDDEDEDITOR_BORDER_RIGHT + EMBEDDEDEDITOR_MARGIN_RIGHT);
		rect->bottom += (EMBEDDEDEDITOR_BORDER_BOTTOM + EMBEDDEDEDITOR_MARGIN_BOTTOM);
	}
	
	return TRUE;
}

static HBRUSH
EmbeddedEditor_GetBackBrush(EmbeddedEditor *self)
{
	if (NULL == self->backBrush)
		self->backBrush = CreateSolidBrush(self->backColor);

	return self->backBrush;
}

static HBRUSH
EmbeddedEditor_GetBorderBrush(EmbeddedEditor *self)
{
	if (NULL == self->borderBrush)
		self->borderBrush = CreateSolidBrush(self->borderColor);

	return self->borderBrush;
}

static BOOL
EmbdeddedEditor_GetBorderEnabled(HWND hwnd)
{
	unsigned long style;
	
	style = GetWindowStyleEx(hwnd);
	if (0 != ((WS_EX_STATICEDGE | WS_EX_CLIENTEDGE) & style))
		return TRUE;

	style = GetWindowStyle(hwnd);
	if (0 != (WS_BORDER & style))
		return TRUE;

	return FALSE;
}


static BOOL
EmbdeddedEditor_DrawBorder(EmbeddedEditor *self, HWND hwnd, HDC hdc)
{
	RECT windowRect, clientRect;
	HRGN borderRgn;
	POINT polygons[16] = {0};
	const int polygonsPointCount[] = {4, 4, 4, 4};

	if (FALSE == GetWindowRect(hwnd, &windowRect) || 
		FALSE == GetClientRect(hwnd, &clientRect))
	{
		return FALSE;
	}

	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&clientRect, 2);

	OffsetRect(&clientRect, -windowRect.left, -windowRect.top);
	OffsetRect(&windowRect, -windowRect.left, -windowRect.top);

	MakeRectPolygon(&polygons[0], windowRect.left, windowRect.top, windowRect.right, clientRect.top);
	MakeRectPolygon(&polygons[4], clientRect.right, clientRect.top, windowRect.right, clientRect.bottom);
	MakeRectPolygon(&polygons[8], windowRect.left, clientRect.bottom, windowRect.right, windowRect.bottom);
	MakeRectPolygon(&polygons[12], windowRect.left, clientRect.top, clientRect.left, clientRect.bottom);
	
	borderRgn = CreatePolyPolygonRgn(polygons, 
									 polygonsPointCount, 
									 ARRAYSIZE(polygonsPointCount),
									 WINDING);

	if (NULL != borderRgn)
	{
		MakeRectPolygon(&polygons[0], 
						windowRect.left + EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_LEFT, 
						windowRect.top + EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_TOP, 
						windowRect.right - EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_RIGHT, 
						windowRect.top + (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_TOP + EMBEDDEDEDITOR_FRAME_TOP));
		MakeRectPolygon(&polygons[4], 
						windowRect.right - (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_RIGHT + EMBEDDEDEDITOR_FRAME_RIGHT), 
						windowRect.top + (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_TOP + EMBEDDEDEDITOR_FRAME_TOP), 
						windowRect.right - EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_RIGHT, 
						windowRect.bottom - (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_BOTTOM + EMBEDDEDEDITOR_FRAME_BOTTOM));
		MakeRectPolygon(&polygons[8], 
						windowRect.left + EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_LEFT, 
						windowRect.bottom - (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_BOTTOM + EMBEDDEDEDITOR_FRAME_BOTTOM), 
						windowRect.right - EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_RIGHT, 
						windowRect.bottom - EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_BOTTOM);
		MakeRectPolygon(&polygons[12], 
						windowRect.left + EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_LEFT, 
						windowRect.top + (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_TOP + EMBEDDEDEDITOR_FRAME_TOP), 
						windowRect.left + (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_LEFT + EMBEDDEDEDITOR_FRAME_LEFT), 
						windowRect.bottom - (EMBEDDEDEDITOR_FRAME_OUTTER_SPACE_BOTTOM + EMBEDDEDEDITOR_FRAME_BOTTOM));

		HRGN frameRgn = CreatePolyPolygonRgn(polygons, 
										 polygonsPointCount, 
										 ARRAYSIZE(polygonsPointCount),
										 WINDING);
		if (NULL != frameRgn)
		{
			int combineResult = CombineRgn(frameRgn, borderRgn, frameRgn, RGN_AND);
			if (NULLREGION != combineResult && ERROR != combineResult)
			{
				FillRgn(hdc, frameRgn, EmbeddedEditor_GetBorderBrush(self));
				combineResult = CombineRgn(borderRgn, borderRgn, frameRgn, RGN_DIFF);
			}
			else
				combineResult = COMPLEXREGION;

			if (NULLREGION != combineResult && ERROR != combineResult)
				FillRgn(hdc, borderRgn, EmbeddedEditor_GetBackBrush(self));
		
			DeleteObject(frameRgn);
		}
		DeleteObject(borderRgn);
	}

	

	return TRUE;
}

static BOOL
EmbeddedEditor_InvokeCallback(EmbeddedEditor *self, HWND hwnd, BOOL cancelMode, BOOL removeCallback)
{
	wchar_t *text;
	unsigned int length;
	EmbeddedEditorFinishCb callback;

	if (NULL == self || NULL == self->callback)
		return FALSE;
	
	
	length = (unsigned int)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0L);
	text = String_Malloc(++length);
	if (NULL == text)
		return FALSE;
	
	SendMessage(hwnd, WM_GETTEXT, (WPARAM)length, (LPARAM)text);

	callback = self->callback;
	if (FALSE != removeCallback)
		self->callback = NULL;

	callback(hwnd, cancelMode, text, self->user);
	
	return TRUE;
}


static const wchar_t *
EmbeddedEditor_GetWindowText(EmbeddedEditor *self, HWND hwnd, size_t *lengthOut)
{
	size_t length;

	length = (size_t)SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0L);
	if (length >= self->bufferSize)
	{
		size_t size;

		size = (((length + 1)/128) + 1) * 128;

		String_Free(self->buffer);
		self->buffer = String_Malloc(size);
		if (NULL == self->buffer)
			return FALSE;

		self->bufferSize = size;
	}

	if (0 != length)
		length = SendMessage(hwnd, WM_GETTEXT, (WPARAM)self->bufferSize, (LPARAM)self->buffer);
	else
		self->buffer[0] = L'\0';

	if (NULL != lengthOut)
		*lengthOut = length;

	return self->buffer;
}

static BOOL
EmbeddedEditor_Resize(EmbeddedEditor *self, HWND hwnd, BOOL redraw)
{
	size_t textLength;
	const wchar_t *text;
	HDC hdc;
	SIZE maximumSize;
	BOOL borderEnabled;
	HWND parentWindow;
	RECT rect, parentRect, marginRect;
	SIZE parentSize;
	BOOL result;
	unsigned int windowStyle;
	HFONT font, prevFont;

	parentWindow = GetAncestor(hwnd, GA_PARENT);
	if (NULL == parentWindow || 
		FALSE == GetClientRect(parentWindow, &parentRect))
	{
		return FALSE;
	}

	parentSize.cx = RECTWIDTH(parentRect);
	parentSize.cy = RECTHEIGHT(parentRect);

	windowStyle = GetWindowStyle(hwnd);
	borderEnabled = EmbdeddedEditor_GetBorderEnabled(hwnd);

	text = EmbeddedEditor_GetWindowText(self, hwnd, &textLength);
	if (NULL == text)
		return FALSE;
	
	hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc)
		return FALSE;

	result = FALSE;

	font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	prevFont = SelectFont(hdc, font);

	if (-1 == self->spacing ||
		-1 == self->lineHeight)
	{
		TEXTMETRIC textMetrics;
		if (FALSE != GetTextMetrics(hdc, &textMetrics))
		{
			self->spacing = textMetrics.tmAveCharWidth;
			self->lineHeight = textMetrics.tmHeight;
		}
	}

	if (0 == (ES_MULTILINE & windowStyle))
	{
		unsigned long margins;
		margins = (unsigned long)SendMessage(hwnd, EM_GETMARGINS, 0, 0L);
		SetRect(&marginRect, (short)LOWORD(margins), 0, (short)HIWORD(margins), 0);
	}
	else
	{
		RECT clientRect;
		if (FALSE == GetClientRect(hwnd, &clientRect))
			SetRectEmpty(&clientRect);
		
		if (clientRect.right < clientRect.left)
			clientRect.right = clientRect.left;
		if (clientRect.bottom < (clientRect.top + self->lineHeight))
			clientRect.bottom = clientRect.top + self->lineHeight;

		SendMessage(hwnd, EM_GETRECT, 0, (LPARAM)&marginRect);
	
		marginRect.left = (marginRect.left > clientRect.left) ?
						(marginRect.left - clientRect.left) : 1;

		marginRect.top = (marginRect.top > clientRect.top) ?
						(marginRect.top - clientRect.top) : 1;

		marginRect.right = (marginRect.right < clientRect.right) ?
						(clientRect.right - marginRect.right) : 1;

		marginRect.bottom = (marginRect.bottom < clientRect.bottom) ?
						(clientRect.bottom - marginRect.bottom) : 1;

		if (SendMessage(hwnd, EM_GETLINECOUNT, 0, 0L) > 1)
		{
			if (marginRect.bottom >= (self->lineHeight - 1))
				marginRect.bottom -= (self->lineHeight - 2);
		}

		SetRect(&marginRect, EMBEDDEDEDITOR_MARGIN_LEFT, EMBEDDEDEDITOR_MARGIN_TOP, 
						EMBEDDEDEDITOR_MARGIN_RIGHT, EMBEDDEDEDITOR_MARGIN_BOTTOM);
	}
		
	maximumSize.cx = (self->maximumSize.cx > 0) ? MIN(parentSize.cx, self->maximumSize.cx) : parentSize.cx;
	maximumSize.cx -= (marginRect.left + marginRect.right);
	if (FALSE != borderEnabled)
		maximumSize.cx -= (EMBEDDEDEDITOR_BORDER_LEFT + EMBEDDEDEDITOR_BORDER_RIGHT);

	if (maximumSize.cx < 0)
		maximumSize.cx = 0;

	maximumSize.cy = (self->maximumSize.cy > 0) ? MIN(parentSize.cy, self->maximumSize.cy) : parentSize.cy;
	maximumSize.cy -= (marginRect.top + marginRect.bottom);
	if (FALSE != borderEnabled)
		maximumSize.cy -= (EMBEDDEDEDITOR_BORDER_TOP + EMBEDDEDEDITOR_BORDER_BOTTOM);

	if (maximumSize.cy < 0)
		maximumSize.cy = 0;
	
	if (0 == textLength)
	{
		SetRectEmpty(&rect);
		result = TRUE;
	}
	else
	{
		SetRect(&rect, 0, 0, maximumSize.cx, maximumSize.cy);
		result = DrawText(hdc, text, (int)textLength, &rect, 
							DT_CALCRECT | DT_LEFT | DT_TOP | DT_WORDBREAK | 
							DT_EDITCONTROL | DT_NOPREFIX);
	}

	SelectFont(hdc, prevFont);
	ReleaseDC(hwnd, hdc);


	if (FALSE != result)
	{
		unsigned int flags;
	
		rect.right += 2 * self->spacing;

		if (RECTHEIGHT(rect) < self->lineHeight)
			rect.bottom  = rect.top + self->lineHeight;
		else if (RECTHEIGHT(rect) > self->lineHeight)
				rect.right = rect.left + maximumSize.cx;

		rect.right += (marginRect.left + marginRect.right);
		rect.bottom += (marginRect.top + marginRect.bottom);
		
		if (FALSE != borderEnabled)
		{
			rect.right += (EMBEDDEDEDITOR_BORDER_LEFT + EMBEDDEDEDITOR_BORDER_RIGHT);
			rect.bottom += (EMBEDDEDEDITOR_BORDER_TOP + EMBEDDEDEDITOR_BORDER_BOTTOM);
		}

		flags = SWP_NOACTIVATE | SWP_NOZORDER;
		if (FALSE == redraw)
			flags |= SWP_NOREDRAW;
	
		OffsetRect(&rect, self->anchorPoint.x, self->anchorPoint.y);

		long offsetX = 0, offsetY = 0;

		if (0 != (ES_CENTER & windowStyle))
		{
			if (self->maximumSize.cx > 0)
			{
				offsetX += (self->maximumSize.cx - RECTWIDTH(rect))/2;
			}
		}
			
		if (rect.right > parentSize.cx)
			offsetX -= (rect.right - parentSize.cx);

		if (rect.bottom > parentSize.cy)
			offsetY -= (rect.bottom - parentSize.cy);

		OffsetRect(&rect, offsetX, offsetY);

		result = SetWindowPos(hwnd, NULL, rect.left, rect.top, RECTWIDTH(rect), RECTHEIGHT(rect), flags);
	}
	
	return result;
}

static void
EmbdeddedEditor_OnDestroy(EmbeddedEditor *self, HWND hwnd)
{
	HWND parent;

	if (NULL != self &&
		NULL != self->callback)
	{
		EmbeddedEditor_InvokeCallback(self, hwnd, TRUE, TRUE);
	}

	RemoveProp(hwnd, MAKEINTATOM(EMBEDDEDEDITOR_PROP));
	if (NULL == self)
		return;

	if (NULL != self->originalProc)
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)self->originalProc);
		CallWindowProc(self->originalProc, hwnd, WM_DESTROY, 0, 0L);
	}

	if(NULL != self->backBrush)
		DeleteObject(self->backBrush);

	if (NULL != self->borderBrush)
		DeleteObject(self->borderBrush);

	String_Free(self->buffer);

	free(self);

	parent = GetAncestor(hwnd, GA_PARENT);
	if(NULL != parent)
		EmbeddedEditorParent_RemoveChild(parent, hwnd);

	EmbeddidEditorThread_EndMouseMonitor(hwnd);
}

static BOOL
EmbdeddedEditor_OnNcCalcSize(EmbeddedEditor *self, HWND hwnd, BOOL calcValidRects, 
							 NCCALCSIZE_PARAMS *params, LRESULT *result)
{

	if (FALSE != EmbdeddedEditor_GetBorderEnabled(hwnd))
	{
		if (FALSE != calcValidRects)
		{
			SetRect(&params->rgrc[0], 
					params->lppos->x + EMBEDDEDEDITOR_BORDER_LEFT, 
					params->lppos->y + EMBEDDEDEDITOR_BORDER_TOP, 
					params->lppos->x + params->lppos->cx - EMBEDDEDEDITOR_BORDER_RIGHT,  
					params->lppos->y + params->lppos->cy - EMBEDDEDEDITOR_BORDER_BOTTOM);

			*result = WVR_ALIGNTOP | WVR_ALIGNLEFT;
		}
		else
		{
			if (FALSE != GetWindowRect(hwnd, &params->rgrc[0]))
			{
				params->rgrc[0].left += EMBEDDEDEDITOR_BORDER_LEFT;
				params->rgrc[0].top += EMBEDDEDEDITOR_BORDER_TOP;
				params->rgrc[0].right -= EMBEDDEDEDITOR_BORDER_RIGHT;
				params->rgrc[0].bottom -= EMBEDDEDEDITOR_BORDER_BOTTOM;
			}
		}
	}

	return TRUE;
}
static BOOL
EmbdeddedEditor_OnNcPaint(EmbeddedEditor *self, HWND hwnd, HRGN updateRegion)
{		
	BOOL result;

	if (FALSE != EmbdeddedEditor_GetBorderEnabled(hwnd))
	{
		HDC hdc;
		unsigned int flags;

		flags = DCX_PARENTCLIP | DCX_CACHE | DCX_WINDOW | DCX_CLIPSIBLINGS |
				DCX_INTERSECTUPDATE | DCX_VALIDATE;

		hdc = GetDCEx(hwnd, ((HRGN)NULLREGION != updateRegion) ? updateRegion : NULL, flags);
		if (NULL != hdc)
		{
			result = EmbdeddedEditor_DrawBorder(self, hwnd, hdc);
			ReleaseDC(hwnd, hdc);
		}
		else
			result = FALSE;
	}
	else 
		result = TRUE;
	
	return result;
}

static BOOL
EmbeddedEditor_OnSetCursor(EmbeddedEditor *self, HWND hwnd, HWND cursorWindow, int hitTest, int mouseMessage, LRESULT *result)
{
	HCURSOR cursor;

	if (cursorWindow != hwnd || 
		HTCLIENT != hitTest)
	{
		return FALSE;
	}

	cursor = LoadCursor(NULL, IDC_IBEAM);
	if (NULL == cursor)
		return FALSE;

	SetCursor(cursor);

	*result = TRUE;
	return TRUE;
}


static BOOL
EmbeddedEditor_OnGetDlgCode(EmbeddedEditor *self, HWND hwnd, unsigned int vKey, MSG *message, LRESULT *result)
{
	if (NULL != message)
	{
		switch(vKey)
		{
			case VK_TAB:
			case VK_ESCAPE:
				EmbeddedEditor_InvokeCallback(self, hwnd, TRUE, TRUE);
				DestroyWindow(hwnd);
				*result = DLGC_WANTMESSAGE;
				return TRUE;
			case VK_RETURN:
				EmbeddedEditor_InvokeCallback(self, hwnd, FALSE, TRUE);
				DestroyWindow(hwnd);
				*result = DLGC_WANTMESSAGE;
				return TRUE;
		}
	}

	if (NULL != self->originalProc)
		*result = CallWindowProc(self->originalProc, hwnd, WM_GETDLGCODE, (WPARAM)vKey, (LPARAM)message);
	else
		*result = 0;

	if (NULL == message)
		*result |= DLGC_WANTMESSAGE;


	return TRUE;
}

static BOOL
EmbeddedEditor_OnKillFocus(EmbeddedEditor *self, HWND hwnd, HWND focusedWindow)
{
	EmbeddedEditor_InvokeCallback(self, hwnd, FALSE, TRUE);
	DestroyWindow(hwnd);
	return TRUE;
}

static BOOL
EmbeddedEditor_OnMouseWheel(EmbeddedEditor *self, HWND hwnd, int virtualKeys, int distance, long pointer_s)
{
	HWND parentWindow;

	parentWindow = GetAncestor(hwnd, GA_PARENT);

	EmbeddedEditor_InvokeCallback(self, hwnd, TRUE, TRUE);
	DestroyWindow(hwnd);
		
	if (NULL != parentWindow)
	{
		SendMessage(parentWindow, WM_MOUSEWHEEL, 
			MAKEWPARAM(virtualKeys, distance), (LPARAM)pointer_s);
	}

	return TRUE;
}

static void
EmbeddedEditor_OnCommand(EmbeddedEditor *self, HWND hwnd, int eventId)
{
	switch(eventId)
	{
		case EN_UPDATE:
			EmbeddedEditor_Resize(self, hwnd, TRUE);
			break;
	}
}
static BOOL
EmbeddedEditor_OnSetFont(EmbeddedEditor *self, HWND hwnd, HFONT font, BOOL redraw)
{
	if (NULL != self->originalProc)
		CallWindowProc(self->originalProc, hwnd, WM_SETFONT, (WPARAM)font, MAKELPARAM(redraw, 0));

	self->spacing = -1;
	self->lineHeight = -1;

	EmbeddedEditor_Resize(self, hwnd, redraw);
	return TRUE;
}


static BOOL
EmbeddedEditor_OnWindowPosChanged(EmbeddedEditor *self, HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
	{
		RECT formatRect;
		if (FALSE != GetClientRect(hwnd, &formatRect))
		{
			formatRect.left += EMBEDDEDEDITOR_MARGIN_LEFT;
			formatRect.top += EMBEDDEDEDITOR_MARGIN_TOP;
			formatRect.right -= EMBEDDEDEDITOR_MARGIN_RIGHT;
			formatRect.bottom -= EMBEDDEDEDITOR_MARGIN_BOTTOM;
			
			if (formatRect.right < formatRect.left)
				formatRect.right = formatRect.left;

			if (formatRect.bottom < formatRect.top)
				formatRect.bottom  = formatRect.top;

			SendMessage(hwnd, EM_SETRECTNP, 0, (LPARAM)&formatRect); 
		}
	}

	if (NULL != self->originalProc)
		CallWindowProc(self->originalProc, hwnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)pwp);

	return TRUE;
}

static void 
EmbeddedEditor_OnSetTextColor(EmbeddedEditor *self, HWND hwnd, COLORREF color, LRESULT *result)
{
	*result = (LRESULT)self->textColor;
	if (self->textColor != color)
		self->textColor = color;
}

static void
EmbeddedEditor_OnGetTextColor(EmbeddedEditor *self, HWND hwnd, LRESULT *result)
{
	*result = (LRESULT)self->textColor;
}

static void
EmbeddedEditor_OnSetBackColor(EmbeddedEditor *self, HWND hwnd, COLORREF color, LRESULT *result)
{
	*result = (LRESULT)self->backColor;
	if (self->backColor != color)
	{
		self->backColor = color;
		if (NULL != self->backBrush)
		{
			DeleteObject(self->backBrush);
			self->backBrush = NULL;
		}
	}
}

static void 
EmbeddedEditor_OnGetBackColor(EmbeddedEditor *self, HWND hwnd, LRESULT *result)
{
	*result = (LRESULT)self->backColor;
}

static void 
EmbeddedEditor_OnSetBorderColor(EmbeddedEditor *self, HWND hwnd, COLORREF color, LRESULT *result)
{
	*result = (LRESULT)self->borderColor;
	if (self->borderColor != color)
	{
		self->borderColor = color;
		if (NULL != self->borderBrush)
		{
			DeleteObject(self->borderBrush);
			self->borderBrush = NULL;
		}
	}
}

static void 
EmbeddedEditor_OnGetBorderColor(EmbeddedEditor *self, HWND hwnd, LRESULT *result)
{
	*result = (LRESULT)self->borderColor;
}

static void
EmbeddedEditor_OnSetUserData(EmbeddedEditor *self, HWND hwnd, void *user, LRESULT *result)
{
	*result = (LRESULT)self->user;
	self->user = user;
}

static void
EmbeddedEditor_OnGetUserData(EmbeddedEditor *self, HWND hwnd, LRESULT *result)
{
	*result = (LRESULT)self->user;
}

static void
EmbeddedEditor_OnSetAnchorPoint(EmbeddedEditor *self, HWND hwnd, long x, long y, LRESULT *result)
{
	self->anchorPoint.x = x;
	self->anchorPoint.y = y;
	*result = TRUE;
}

static void
EmbeddedEditor_OnGetAnchorPoint(EmbeddedEditor *self, HWND hwnd, long *x, long *y, LRESULT *result)
{
	if (NULL != x)
		*x = self->anchorPoint.x;

	if (NULL != y)
		*y = self->anchorPoint.y;
	
	*result = TRUE;
}

static void
EmbeddedEditor_OnSetMaxSize(EmbeddedEditor *self, HWND hwnd, long width, long height, LRESULT *result)
{
	self->maximumSize.cx = width;
	self->maximumSize.cy = height;
	*result = TRUE;
}

static void
EmbeddedEditor_OnGetMaxSize(EmbeddedEditor *self, HWND hwnd, long *width, long *height, LRESULT *result)
{
	if (NULL != width)
		*width = self->maximumSize.cx;

	if (NULL != height)
		*height = self->maximumSize.cy;
	
	*result = TRUE;
}

static void
EmbeddedEditor_OnEndEditing(EmbeddedEditor *self, HWND hwnd, BOOL cancel)
{
	EmbeddedEditor_InvokeCallback(self, hwnd, cancel, TRUE);
	DestroyWindow(hwnd);
}

static BOOL
EmbeddedEditor_MessageProc(EmbeddedEditor *self, HWND hwnd, UINT uMsg, 
						   WPARAM wParam, LPARAM lParam, LRESULT *result)
{

	switch(uMsg)
	{
		case WM_DESTROY: 
			EmbdeddedEditor_OnDestroy(self, hwnd);
			return TRUE;
		case WM_NCCALCSIZE:
			return EmbdeddedEditor_OnNcCalcSize(self, hwnd, (BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam, result);
		case WM_NCPAINT:
			return EmbdeddedEditor_OnNcPaint(self, hwnd, (HRGN)wParam);
		case WM_SETCURSOR:
			return EmbeddedEditor_OnSetCursor(self, hwnd, (HWND)wParam, 
							LOWORD(lParam), HIWORD(lParam), result);
		case WM_GETDLGCODE:
			return EmbeddedEditor_OnGetDlgCode(self, hwnd, (unsigned int)wParam, (MSG*)lParam, result);
		case WM_KILLFOCUS:
			EmbeddedEditor_OnKillFocus(self, hwnd, (HWND)wParam);
			return 0;
		case WM_MOUSEWHEEL:
			return EmbeddedEditor_OnMouseWheel(self, hwnd, LOWORD(wParam), (short)HIWORD(wParam), (LONG)lParam);
		case WM_SETFONT:
			return EmbeddedEditor_OnSetFont(self, hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam));
		case WM_WINDOWPOSCHANGED:
			return EmbeddedEditor_OnWindowPosChanged(self, hwnd, (WINDOWPOS*)lParam);

		case EMBEDDEDEDITOR_WM_SET_TEXT_COLOR:	EmbeddedEditor_OnSetTextColor(self, hwnd, (COLORREF)lParam, result); return TRUE;
		case EMBEDDEDEDITOR_WM_GET_TEXT_COLOR:	EmbeddedEditor_OnGetTextColor(self, hwnd, result); return TRUE;
		case EMBEDDEDEDITOR_WM_SET_BACK_COLOR:	EmbeddedEditor_OnSetBackColor(self, hwnd, (COLORREF)lParam, result); return TRUE;
		case EMBEDDEDEDITOR_WM_GET_BACK_COLOR:	EmbeddedEditor_OnGetBackColor(self, hwnd, result); return TRUE;
		case EMBEDDEDEDITOR_WM_SET_BORDER_COLOR: EmbeddedEditor_OnSetBorderColor(self, hwnd, (COLORREF)lParam, result); return TRUE;
		case EMBEDDEDEDITOR_WM_GET_BORDER_COLOR: EmbeddedEditor_OnGetBorderColor(self, hwnd, result); return TRUE;
		case EMBEDDEDEDITOR_WM_SET_USER_DATA:	EmbeddedEditor_OnSetUserData(self, hwnd, (void*)lParam, result); return TRUE;
		case EMBEDDEDEDITOR_WM_GET_USER_DATA:	EmbeddedEditor_OnGetUserData(self, hwnd, result); return TRUE;
		case EMBEDDEDEDITOR_WM_SET_ANCHOR_POINT: EmbeddedEditor_OnSetAnchorPoint(self, hwnd, (long)wParam, (long)lParam, result); return TRUE;
		case EMBEDDEDEDITOR_WM_GET_ANCHOR_POINT: EmbeddedEditor_OnGetAnchorPoint(self, hwnd, (long*)wParam, (long*)lParam, result);	return TRUE;
		case EMBEDDEDEDITOR_WM_SET_MAX_SIZE: EmbeddedEditor_OnSetMaxSize(self, hwnd, (long)wParam, (long)lParam, result); return TRUE;
		case EMBEDDEDEDITOR_WM_GET_MAX_SIZE: EmbeddedEditor_OnGetMaxSize(self, hwnd, (long*)wParam, (long*)lParam, result); return TRUE;
		case EMBEDDEDEDITOR_WM_END_EDITING:	EmbeddedEditor_OnEndEditing(self, hwnd, (BOOL)wParam); return TRUE;

	}
	return FALSE;
}

static LRESULT WINAPI 
EmbeddedEditor_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EmbeddedEditor *self;
	LRESULT result;

	self = EMBEDDEDEDITOR(hwnd);

	if (NULL == self || 
		NULL == self->originalProc)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	result = 0;
	if (FALSE != EmbeddedEditor_MessageProc(self, hwnd, uMsg, wParam, lParam, &result))
	{
		return result;
	}

	return CallWindowProc(self->originalProc, hwnd, uMsg, wParam, lParam);
}


static void
EmbdeddedEditorParent_OnDestroy(EmbeddedEditorParent *self, HWND hwnd)
{
	RemoveProp(hwnd, MAKEINTATOM(EMBEDDEDEDITOR_PROP));
	if (NULL == self)
		return;

	if (NULL != self->originalProc)
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)self->originalProc);
		CallWindowProc(self->originalProc, hwnd, WM_DESTROY, 0, 0L);
	}

	delete self;
}

static BOOL
EmbdeddedEditorParent_OnGetEditColors(EmbeddedEditorParent *self, HWND hwnd, HDC hdc, HWND control, LRESULT *result)
{
	size_t index;
	index = self->editorList.size();
	while(index--)
	{
		if (control == self->editorList[index])
		{
			EmbeddedEditor *editor = EMBEDDEDEDITOR(control);
			if (NULL != editor)
			{
				SetTextColor(hdc, editor->textColor);
				SetBkColor(hdc, editor->backColor);
				*result = (LRESULT)EmbeddedEditor_GetBackBrush(editor);
				return TRUE;
			}
		}
	}

	return FALSE;
}

static void
EmbdeddedEditorParent_OnCommand(EmbeddedEditorParent *self, HWND hwnd, int commandId, int eventId, HWND control)
{
	size_t index;

	if (NULL == control)
		return;

	index = self->editorList.size();
	while(index--)
	{
		if (control == self->editorList[index])
		{
			EmbeddedEditor *editor = EMBEDDEDEDITOR(control);
			if (NULL != editor)
			{
				EmbeddedEditor_OnCommand(editor, control, eventId);
			}
		}
	}

}

static BOOL
EmbeddedEditorParent_MessageProc(EmbeddedEditorParent *self, HWND hwnd, UINT uMsg, 
						   WPARAM wParam, LPARAM lParam, LRESULT *result)
{

	switch(uMsg)
	{
		case WM_DESTROY: 
			EmbdeddedEditorParent_OnDestroy(self, hwnd);
			return TRUE;

		case WM_CTLCOLOREDIT:
			return EmbdeddedEditorParent_OnGetEditColors(self, hwnd, (HDC)wParam, (HWND)lParam, result);

		case WM_COMMAND:
			EmbdeddedEditorParent_OnCommand(self, hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			break;
	}
	return FALSE;
}

static LRESULT WINAPI 
EmbeddedEditorParent_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EmbeddedEditorParent *self;
	LRESULT result;

	self = EMBEDDEDEDITOR_PARENT(hwnd);

	if (NULL == self || 
		NULL == self->originalProc)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	result = 0;
	if (FALSE != EmbeddedEditorParent_MessageProc(self, hwnd, uMsg, wParam, lParam, &result))
	{
		return result;
	}

	return CallWindowProc(self->originalProc, hwnd, uMsg, wParam, lParam);
}


static LRESULT CALLBACK
EmbeddidEditorThread_MouseProc(int code, unsigned int messageId, MOUSEHOOKSTRUCT *mouse)
{		
	if ((size_t)-1 != editorTls)
	{	
		EmbeddedEditorThread *threadData;
		threadData = (EmbeddedEditorThread*)WASABI_API_APP->GetThreadStorage(editorTls);

		if (NULL != threadData)
		{
			LRESULT result;
			if (NULL != threadData->hook)
				result = CallNextHookEx(threadData->hook, code, (WPARAM)messageId, (LPARAM)mouse);
			else
				result = 0;

			if (code >= 0)
			{
				if ((messageId >= WM_LBUTTONDOWN && messageId <= 0x20E && mouse->hwnd != threadData->window) ||
					(messageId >= WM_NCLBUTTONDOWN && messageId <= 0x00AD))
				{
					
					HWND editorWindow;
					editorWindow = threadData->window;
					if (NULL != editorWindow)
						PostMessage(editorWindow, EMBEDDEDEDITOR_WM_END_EDITING, FALSE, 0L);
				}
			}
			
			return result;
		}
	}

	return 0;
}
