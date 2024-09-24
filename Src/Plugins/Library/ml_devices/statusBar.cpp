#include "main.h"
#include "./statusBar.h"
#include <vector>

#include <strsafe.h>

typedef struct StatusTextBlock
{
	wchar_t *text;
	size_t	length;
	long	width;
} StatusTextBlock;

typedef struct StaturRecord
{
	unsigned int id;
	StatusTextBlock mainBlock;
	StatusTextBlock rightBlock;
} StatusRecord;
typedef std::vector<StatusRecord*> StatusRecordList;

typedef struct StatusBar
{
	HFONT font;
	HBRUSH backBrush;
	COLORREF textColor;
	COLORREF backColor;
	StatusRecordList list;
	int idealHeight;
	long spacing;
	long leftBlockMinWidth;
	BackBuffer backBuffer;
} StatusBar;

#define STATUSBAR(_hwnd) ((StatusBar*)(LONGX86)GetWindowLongPtrW((_hwnd), 0))
#define STATUSBAR_RET_VOID(_self, _hwnd) {(_self) = STATUSBAR((_hwnd)); if (NULL == (_self)) return;}
#define STATUSBAR_RET_VAL(_self, _hwnd, _error) {(_self) = STATUSBAR((_hwnd)); if (NULL == (_self)) return (_error);}

static LRESULT CALLBACK 
StatusBar_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);


static ATOM 
StatusBar_GetClassAtom(HINSTANCE instance)
{
	WNDCLASSEXW klass;
	ATOM klassAtom;

	klassAtom = (ATOM)GetClassInfoExW(instance, STATUSBAR_WINDOW_CLASS, &klass);
	if (0 != klassAtom)
		return klassAtom;

	memset(&klass, 0, sizeof(klass));
	klass.cbSize = sizeof(klass);
	klass.style = 0;
	klass.lpfnWndProc = StatusBar_WindowProc;
	klass.cbClsExtra = 0;
	klass.cbWndExtra = sizeof(StatusBar*);
	klass.hInstance = instance;
	klass.hIcon = NULL;
	klass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	klass.hbrBackground = NULL;
	klass.lpszMenuName = NULL;
	klass.lpszClassName = STATUSBAR_WINDOW_CLASS;
	klass.hIconSm = NULL;
	klassAtom = RegisterClassExW(&klass);
	
	return klassAtom;
}

HWND 
StatusBar_CreateWindow(unsigned long windowExStyle, const wchar_t *text, unsigned long windowStyle, 
						int x, int y, int width, int height, 
						HWND parentWindow, unsigned int controlId)
{
	HINSTANCE instance = GetModuleHandleW(NULL);
	ATOM klassAtom = StatusBar_GetClassAtom(instance);
	if (0 == klassAtom)
		return NULL;

	return CreateWindowExW(WS_EX_NOPARENTNOTIFY | windowExStyle, (LPCWSTR)MAKEINTATOM(klassAtom), text,
						   WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | windowStyle,
						   x, y, width, height, parentWindow, (HMENU)controlId, instance, NULL);
}

static void
StatusBar_InitTextBlock(StatusTextBlock *block)
{
	if (NULL != block)
	{
		block->text = NULL;
		block->length = ((size_t)-1);
		block->width = -1;
	}
}

static void
StatusBar_FreeTextBlock(StatusTextBlock *block)
{
	if (NULL != block)
	{
		ResourceString_Free(block->text);
		StatusBar_InitTextBlock(block);
	}
}

static const wchar_t *
StatusBar_RetriveText(StatusTextBlock *block)
{
	const wchar_t *source;
	if (NULL == block)
		return NULL;

	source = block->text;
	if (FALSE != IS_INTRESOURCE(source) && 
		NULL != source)
	{
		wchar_t buffer[256] = {0};
		if (NULL == WASABI_API_LNG)
			return NULL;

		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)source, buffer, ARRAYSIZE(buffer));
		block->text = String_Duplicate(buffer);
	}
	return block->text;
}

static size_t
StatusBar_GetTextBlockLength(StatusTextBlock *block)
{
	const wchar_t *text;

	if (NULL == block)
		return NULL;

	if (((size_t)-1) != block->length)
	{
		text = StatusBar_RetriveText(block);
		block->length = (NULL != text) ? lstrlenW(text) : 0;
	}

	return block->length;
}

static const wchar_t *
StatusBar_GetTextBlockInfo(StatusTextBlock *block, HDC hdc)
{
	const wchar_t *text;
	
	text = StatusBar_RetriveText(block);
		
	if (((size_t)-1) == block->length)
	{
		
		if (FALSE == IS_STRING_EMPTY(text))
			block->length = lstrlenW(text);
		else
			block->length = 0;
	}

	if (-1 == block->width)
	{
		block->width = 0;

		if (0 != block->length)
		{
			RECT rect;
			DRAWTEXTPARAMS textParams;

			textParams.cbSize = sizeof(textParams);
			textParams.iTabLength = 4;
			textParams.iLeftMargin = 0;
			textParams.iRightMargin = 0;

			SetRect(&rect, 0, 0, 0x7FFFFFFF, 0);

			if (FALSE != DrawTextEx(hdc, (wchar_t*)text, -1, &rect, 
								DT_LEFT | DT_TOP | DT_TABSTOP | DT_SINGLELINE | 
								DT_NOPREFIX | DT_EXPANDTABS | DT_NOCLIP | DT_CALCRECT | DT_EDITCONTROL, 
								&textParams))
			{
				block->width = RECTWIDTH(rect);
			}
		}

		TEXTMETRIC metrics;
		if (FALSE != GetTextMetrics(hdc, &metrics))
		{
			block->width += metrics.tmAveCharWidth/2;
		}
	
	}

	return text;
}

static unsigned int
StatusBar_GetNextFreeId(StatusBar *self)
{
	unsigned int id;
	size_t index, count;
	StatusRecord *record;
	BOOL checkId;

	count = self->list.size();
	id = (unsigned int)count;
	do
	{
		if (0xFFFFFFFF == id)
			return STATUS_ERROR;

		checkId = FALSE;
		index = count;
		while(index--)
		{
			record = self->list[index];
			if (record->id == id)
			{
				id++;
				checkId = TRUE;
				break;
			}
		}

	} while(FALSE != checkId);

	return id;
}

static StatusRecord *
StatusBar_AddRecord(StatusBar *self, const wchar_t *text)
{	
	StatusRecord *record;
	unsigned int id;

	id = StatusBar_GetNextFreeId(self);
	if (STATUS_ERROR == id)
		return NULL;

	record = (StatusRecord*)malloc(sizeof(StatusRecord));
	if(NULL == record)
		return NULL;

	record->id = 0;
	StatusBar_InitTextBlock(&record->mainBlock);
	StatusBar_InitTextBlock(&record->rightBlock);
	
	record->mainBlock.text = ResourceString_Duplicate(text);
	
	self->list.push_back(record);

	return record;
}

static StatusRecord *
StatusBar_FindRecord(StatusBar *self, unsigned int id, size_t *indexOut)
{
	StatusRecord *record;
	size_t index;
	
	index = self->list.size();
	while(index--)
	{
		record = self->list[index];
		if (record->id == id)
		{
			if (NULL != indexOut)
				*indexOut = index;
			return record;
		}
	}

	return NULL;
}

static StatusRecord *
StatusBar_FindVisibleRecord(StatusBar *self, size_t *indexOut)
{	
	StatusRecord *record;
	size_t index;
	const wchar_t *text;
	
	index = self->list.size();
	while(index--)
	{
		record = self->list[index];

		text = StatusBar_RetriveText(&record->mainBlock);
		if (FALSE != IS_STRING_EMPTY(text))
			text = StatusBar_RetriveText(&record->rightBlock);

		if (FALSE == IS_STRING_EMPTY(text))
		{
			if (NULL != indexOut)
				*indexOut = index;
			return record;
		}
	}

	return NULL;
}

static void
StatusBar_Paint(HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	StatusBar *self;
	StatusRecord *record;
	const wchar_t *text;
				
	STATUSBAR_RET_VOID(self, hwnd);
	
	if (FALSE != erase)
	{
		FillRect(hdc, paintRect, self->backBrush);
	}

	record = StatusBar_FindVisibleRecord(self, NULL);
	if (NULL != record)
	{
		COLORREF prevBackColor, prevTextColor;
		HFONT prevFont;
		RECT clientRect, textRect;
		DRAWTEXTPARAMS textParams;
		long limit;

		prevBackColor = SetBkColor(hdc, self->backColor);
		prevTextColor = SetTextColor(hdc, self->textColor);
		prevFont = SelectFont(hdc, self->font);
		
		textParams.cbSize = sizeof(textParams);
		textParams.iTabLength = 4;
		textParams.iLeftMargin = 0;
		textParams.iRightMargin = 0;

		GetClientRect(hwnd, &clientRect);

		if (-1 == self->spacing)
			self->spacing = Graphics_GetAveStrWidth(hdc, 2);
		if (-1 == self->leftBlockMinWidth)
			self->leftBlockMinWidth = Graphics_GetAveStrWidth(hdc, 16);

		text = StatusBar_GetTextBlockInfo(&record->rightBlock, hdc);
		if (FALSE == IS_STRING_EMPTY(text))
		{
			CopyRect(&textRect, &clientRect);
			textRect.left = textRect.right - record->rightBlock.width;
			limit = clientRect.left + self->spacing + self->leftBlockMinWidth;
			if (textRect.left < limit)
				textRect.left = limit;

						
			if (textRect.left < textRect.right)
			{
				DrawTextEx(hdc, (wchar_t*)text, -1, &textRect, 
						DT_LEFT | DT_BOTTOM | DT_TABSTOP | DT_SINGLELINE | 
						DT_NOPREFIX | DT_EXPANDTABS, &textParams);
			}
		}

		text = StatusBar_GetTextBlockInfo(&record->mainBlock, hdc);
		if (FALSE == IS_STRING_EMPTY(text))
		{
			CopyRect(&textRect, &clientRect);
			textRect.right = textRect.left + record->mainBlock.width;
			limit = clientRect.right - record->rightBlock.width - 8;
			if (limit < self->leftBlockMinWidth)
				limit = self->leftBlockMinWidth;
			if (textRect.right > limit)
				textRect.right = limit;


			if (textRect.left < textRect.right)
			{
				DrawTextEx(hdc, (wchar_t*)text, -1, &textRect, 
						DT_LEFT | DT_BOTTOM | DT_TABSTOP | DT_SINGLELINE | 
						DT_NOPREFIX | DT_EXPANDTABS | DT_END_ELLIPSIS, &textParams);
			}
		}

		

		SetBkColor(hdc, prevBackColor);
		SetTextColor(hdc, prevTextColor);
		SelectFont(hdc, prevFont);
	}

	return;
}

static void
StatusBar_PaintBuffered(HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	StatusBar *self;
	HDC bufferDC, targetDC;
	RECT clientRect;
			
	STATUSBAR_RET_VOID(self, hwnd);
	
	GetClientRect(hwnd, &clientRect);

	if (FALSE != BackBuffer_EnsureSizeEx(&self->backBuffer, 
							RECTWIDTH(*paintRect), RECTHEIGHT(*paintRect), 
							RECTWIDTH(clientRect), RECTHEIGHT(clientRect)))
	{

		bufferDC = BackBuffer_GetDC(&self->backBuffer);
		if (NULL != bufferDC)
		{
			SetViewportOrgEx(bufferDC, -paintRect->left, -paintRect->top, NULL);
			targetDC = hdc;
			hdc = bufferDC;
		}
	}
	else
	{
		bufferDC = NULL;
		targetDC = NULL;
	}

	StatusBar_Paint(hwnd, hdc, paintRect, erase);

	if (NULL != bufferDC)
	{
		hdc  = targetDC;

		SetViewportOrgEx(bufferDC, 0, 0, NULL);

		BackBuffer_Copy(&self->backBuffer, hdc, 
			paintRect->left, paintRect->top, RECTWIDTH(*paintRect), RECTHEIGHT(*paintRect));
	}
	return;
}

static BOOL
StatusBar_InvalidateByIndex(StatusBar *self, HWND hwnd, size_t recordIndex)
{
	unsigned int windowStyle;
	size_t visibleIndex;

	windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_VISIBLE & windowStyle))
		return FALSE;
	
	if(NULL != StatusBar_FindVisibleRecord(self, &visibleIndex))
	{
		if (recordIndex == visibleIndex)
			return InvalidateRect(hwnd, NULL, FALSE);
	}

	return FALSE;

}

static BOOL
StatusBar_InvalidateByRecord(StatusBar *self, HWND hwnd, StatusRecord *record)
{
	unsigned int windowStyle;
	StatusRecord *visibleRecord;

	windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_VISIBLE & windowStyle))
		return FALSE;
	
	visibleRecord = StatusBar_FindVisibleRecord(self, NULL);
	if (record == visibleRecord && 
		NULL != visibleRecord)
	{		
		return InvalidateRect(hwnd, NULL, FALSE);
	}
	return FALSE;
}


static LRESULT
StatusBar_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{	
	StatusBar *self;
			
	self = new StatusBar();
	if (NULL == self)
		return -1;

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)self) && ERROR_SUCCESS != GetLastError())
	{
		delete self;
		return -1;
	}

	if (NULL != createStruct->lpszName)
		StatusBar_AddRecord(self, createStruct->lpszName);
	
	self->backBrush = GetSysColorBrush(COLOR_WINDOW);
	self->backColor = GetSysColor(COLOR_WINDOW);
	self->textColor = GetSysColor(COLOR_WINDOWTEXT);
	self->idealHeight = -1;
	self->spacing = -1;
	self->leftBlockMinWidth = -1;


	BackBuffer_Initialize(&self->backBuffer, hwnd);

	return 0;
}

static void
StatusBar_OnDestroy(HWND hwnd)
{
	StatusBar *self;
	size_t index;
	StatusRecord *record;

	self = STATUSBAR(hwnd);
	SetWindowLongPtr(hwnd, 0, 0);
	
	if (NULL == self)
		return;
	
	index = self->list.size();
	while(index--)
	{
		record = self->list[index];
		StatusBar_FreeTextBlock(&record->mainBlock);
		StatusBar_FreeTextBlock(&record->rightBlock);
	}

	BackBuffer_Uninitialize(&self->backBuffer);

	delete self;
}

static void
StatusBar_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	
	if (NULL != BeginPaint(hwnd, &ps))
	{		
		StatusBar_PaintBuffered(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void 
StatusBar_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	{
		StatusBar_Paint(hwnd, hdc, &clientRect, TRUE);
	}
}

static void
StatusBar_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if (SWP_NOSIZE != ((SWP_NOSIZE |  SWP_FRAMECHANGED) & windowPos->flags) &&
		0 == (SWP_NOREDRAW & windowPos->flags))
	{
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

static LRESULT 
StatusBar_OnSetText(HWND hwnd, LPCWSTR text)
{
	StatusBar *self;
	StatusRecord *record;
	size_t count;

	STATUSBAR_RET_VAL(self, hwnd, FALSE);
	
	count = self->list.size();
	if (0 == count)
	{
		if (NULL == text)
			return TRUE;

		record = StatusBar_AddRecord(self, text);
		if (NULL == record)
			return FALSE;
	}
	else
	{
		record = self->list[count - 1];
		StatusBar_FreeTextBlock(&record->mainBlock);
		StatusBar_FreeTextBlock(&record->rightBlock);

		record->mainBlock.text = ResourceString_Duplicate(text);
	}
	
	StatusBar_InvalidateByRecord(self, hwnd, record);
	
	return TRUE;
}

static LRESULT 
StatusBar_OnGetText(HWND hwnd, LPWSTR buffer, size_t bufferMax)
{
	StatusBar *self;
	StatusRecord *record;
	const wchar_t *lText, *rText;
	size_t count, remaining;
	wchar_t *cursor;
	HRESULT hr;

	STATUSBAR_RET_VAL(self, hwnd, 0);

	if (NULL == buffer)
		return 0;
	
	count = self->list.size();
	if (0 == count)
	{
		lText = NULL;
		rText = NULL;
	}
	else
	{
		record = self->list[count - 1];
		lText = StatusBar_RetriveText(&record->mainBlock);
		rText = StatusBar_RetriveText(&record->rightBlock);
	}
	
	cursor = buffer;
	remaining = bufferMax;
	
	if (NULL != lText)
		hr = StringCchCopyEx(cursor, bufferMax, lText, &cursor, &remaining, 0);
	else
		hr = S_OK;

	if (NULL != rText && SUCCEEDED(hr))
	{
		hr = StringCchCopyEx(cursor, bufferMax, L"\f", &cursor, &remaining, 0);
		if (SUCCEEDED(hr))
			hr = StringCchCopyEx(cursor, bufferMax, rText, &cursor, &remaining, 0);
	}

	return (bufferMax - remaining);
}

static LRESULT 
StatusBar_OnGetTextLength(HWND hwnd)
{
	StatusBar *self;
	StatusRecord *record;
	size_t length, r;

	STATUSBAR_RET_VAL(self, hwnd, 0);

	length = self->list.size();
	if (0 == length)
		return 0;
	
	record = self->list[length - 1];
	length = StatusBar_GetTextBlockLength(&record->mainBlock);
	r = StatusBar_GetTextBlockLength(&record->rightBlock);
	if (0 != r)
	{
		length += (r + 1);
	}

	return length;
}


static void 
StatusBar_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
{
	StatusBar *self;
	StatusRecord *record;
	size_t index;

	STATUSBAR_RET_VOID(self, hwnd);

	self->font = font;
	self->idealHeight = -1;
	self->spacing = -1;
	self->leftBlockMinWidth = -1;
	
	index = self->list.size();
	while(index--)
	{
		record = self->list[index];
		record->mainBlock.width = -1;
		record->rightBlock.width = -1;
	}

	if (NULL != redraw)
		InvalidateRect(hwnd, NULL, TRUE);
}

static LRESULT
StatusBar_OnGetFont(HWND hwnd)
{
	StatusBar *self;
	STATUSBAR_RET_VAL(self, hwnd, NULL);

	return (LRESULT)self->font;
}

static LRESULT 
StatusBar_OnSetBackBrush(HWND hwnd, HBRUSH brush, BOOL redraw)
{
	StatusBar *self;
	HBRUSH prevBrush;

	STATUSBAR_RET_VAL(self, hwnd, (LRESULT)GetSysColorBrush(COLOR_WINDOW));

	prevBrush = self->backBrush;
	self->backBrush = brush;

	if (NULL != redraw)
		InvalidateRect(hwnd, NULL, TRUE);

	return (LRESULT)prevBrush;
}

static LRESULT 
StatusBar_OnGetBackBrush(HWND hwnd)
{
	StatusBar *self;

	STATUSBAR_RET_VAL(self, hwnd, (LRESULT)GetSysColorBrush(COLOR_WINDOW));

	return (LRESULT)self->backBrush;
}

static LRESULT 
StatusBar_OnSetBackColor(HWND hwnd, COLORREF color, BOOL redraw)
{
	StatusBar *self;
	COLORREF prevColor;

	STATUSBAR_RET_VAL(self, hwnd, GetSysColor(COLOR_WINDOW));

	prevColor = self->backColor;
	self->backColor = color;

	if (NULL != redraw)
		InvalidateRect(hwnd, NULL, FALSE);

	return (LRESULT)prevColor;
}

static LRESULT 
StatusBar_OnGetBackColor(HWND hwnd)
{
	StatusBar *self;
	STATUSBAR_RET_VAL(self, hwnd, GetSysColor(COLOR_WINDOW));

	return (LRESULT)self->backColor;

}

static LRESULT 
StatusBar_OnSetTextColor(HWND hwnd, COLORREF color, BOOL redraw)
{
	StatusBar *self;
	COLORREF prevColor;

	STATUSBAR_RET_VAL(self, hwnd, GetSysColor(COLOR_WINDOWTEXT));

	prevColor = self->textColor;
	self->textColor = color;

	if (NULL != redraw)
		InvalidateRect(hwnd, NULL, FALSE);

	return (LRESULT)prevColor;
}

static LRESULT 
StatusBar_OnGetTextColor(HWND hwnd)
{
	StatusBar *self;
	STATUSBAR_RET_VAL(self, hwnd, GetSysColor(COLOR_WINDOWTEXT));

	return (LRESULT)self->textColor;
}

static LRESULT
StatusBar_OnGetIdealHeight(HWND hwnd)
{
	StatusBar *self;
	STATUSBAR_RET_VAL(self, hwnd, 0);

	if (self->idealHeight < 0)
	{
		HDC hdc;

		self->idealHeight = 0;

		hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			TEXTMETRIC metrics;
			HFONT font, prevFont;

			font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
			prevFont = SelectFont(hdc, font);
	
			if (FALSE != GetTextMetrics(hdc, &metrics))
				self->idealHeight = metrics.tmHeight;
					
			SelectFont(hdc, prevFont);
			ReleaseDC(hwnd, hdc);
		}
	}

	return (LRESULT)self->idealHeight;

}
static LRESULT
StatusBar_OnAddStatus(HWND hwnd, const wchar_t *text)
{
	StatusBar *self;
	StatusRecord *record;

	STATUSBAR_RET_VAL(self, hwnd, STATUS_ERROR);

	record = StatusBar_AddRecord(self, text);
	if (NULL == record)
		return STATUS_ERROR;
	
	StatusBar_InvalidateByRecord(self, hwnd, record);

	return record->id;
}

static LRESULT
StatusBar_OnRemoveStatus(HWND hwnd, unsigned int statusId)
{
	StatusBar *self;
	StatusRecord *record;
	size_t index;
	STATUSBAR_RET_VAL(self, hwnd, FALSE);

	record = StatusBar_FindRecord(self, statusId, &index);
	if (NULL == record)
		return FALSE;
	
	StatusBar_InvalidateByRecord(self, hwnd, record);
	
	self->list.erase(self->list.begin() + index);

	StatusBar_FreeTextBlock(&record->mainBlock);
	StatusBar_FreeTextBlock(&record->rightBlock);

	free(record);

	return (LRESULT)TRUE;
}

static LRESULT
StatusBar_OnSetStatusText(HWND hwnd, unsigned int statusId, const wchar_t *text)
{
	StatusBar *self;
	StatusRecord *record;
	size_t index;

	STATUSBAR_RET_VAL(self, hwnd, FALSE);

	record = StatusBar_FindRecord(self, statusId, &index);
	if (NULL == record)
		return FALSE;
	
	StatusBar_InvalidateByRecord(self, hwnd, record);

	StatusBar_FreeTextBlock(&record->mainBlock);
	record->mainBlock.text = ResourceString_Duplicate(text);
	
	StatusBar_InvalidateByRecord(self, hwnd, record);

	return (LRESULT)TRUE;
}

static LRESULT
StatusBar_OnSetStatusRightText(HWND hwnd, unsigned int statusId, const wchar_t *text)
{
	StatusBar *self;
	StatusRecord *record;
	size_t index;

	STATUSBAR_RET_VAL(self, hwnd, FALSE);

	record = StatusBar_FindRecord(self, statusId, &index);
	if (NULL == record)
		return FALSE;
	
	StatusBar_InvalidateByRecord(self, hwnd, record);

	StatusBar_FreeTextBlock(&record->rightBlock);
	record->rightBlock.text = ResourceString_Duplicate(text);
	
	StatusBar_InvalidateByRecord(self, hwnd, record);

	return (LRESULT)TRUE;
}


static LRESULT
StatusBar_OnMoveStatus(HWND hwnd, unsigned int statusId, unsigned int moveCommand)
{
	StatusBar *self;
	StatusRecord *record;
	size_t index, count, newIndex;

	STATUSBAR_RET_VAL(self, hwnd, FALSE);

	record = StatusBar_FindRecord(self, statusId, &index);
	if (NULL == record)
		return FALSE;

	count = self->list.size();
	
	switch(moveCommand)
	{
		
		case STATUS_MOVE_BOTTOM:
			if (0 == index)
				return TRUE;
			self->list.erase(self->list.begin() + index);
			self->list.insert(self->list.begin(), record);
			newIndex = 0;
			break;
			
		case STATUS_MOVE_TOP:		
			if (index == (count - 1))
				return TRUE;

			self->list.erase(self->list.begin() + index);
			self->list.push_back(record);
			newIndex = count - 1;
			break;

		case STATUS_MOVE_UP:
			if (index == (count - 1))
				return TRUE;
			
			self->list.erase(self->list.begin() + index);
			newIndex = index + 1;
			self->list.insert(self->list.begin() + newIndex, record);
			break;

		case STATUS_MOVE_DOWN:	
			if (index == 0)
				return TRUE;
			
			self->list.erase(self->list.begin() + index);
			newIndex = index - 1;
			self->list.insert(self->list.begin() + newIndex, record);
			break;
	}
	
	if (index != newIndex)
	{
		StatusBar_InvalidateByIndex(self, hwnd, index);
		StatusBar_InvalidateByIndex(self, hwnd, newIndex);
	}

	return (LRESULT)TRUE;
}

static LRESULT CALLBACK 
StatusBar_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return StatusBar_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			StatusBar_OnDestroy(hwnd); return 0;
		case WM_PAINT:				StatusBar_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		StatusBar_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_PRINT:				return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	StatusBar_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_MOVE:				return 0;
		case WM_SETTEXT:			return StatusBar_OnSetText(hwnd, (LPCWSTR)lParam);
		case WM_GETTEXT:			return StatusBar_OnGetText(hwnd, (LPWSTR)lParam, (INT)wParam);
		case WM_GETTEXTLENGTH:		return StatusBar_OnGetTextLength(hwnd);
		case WM_SETFONT:			StatusBar_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); return 0;
		case WM_GETFONT:			return StatusBar_OnGetFont(hwnd);
	
		case STATUSBAR_WM_SET_BACK_BRUSH:	return StatusBar_OnSetBackBrush(hwnd, (HBRUSH)lParam, (BOOL)wParam);
		case STATUSBAR_WM_GET_BACK_BRUSH:	return StatusBar_OnGetBackBrush(hwnd);
		case STATUSBAR_WM_SET_BACK_COLOR:	return StatusBar_OnSetBackColor(hwnd, (COLORREF)lParam, (BOOL)wParam);
		case STATUSBAR_WM_GET_BACK_COLOR:	return StatusBar_OnGetBackColor(hwnd);
		case STATUSBAR_WM_SET_TEXT_COLOR:	return StatusBar_OnSetTextColor(hwnd, (COLORREF)lParam, (BOOL)wParam);
		case STATUSBAR_WM_GET_TEXT_COLOR:	return StatusBar_OnGetTextColor(hwnd);
		case STATUSBAR_WM_GET_IDEAL_HEIGHT:	return StatusBar_OnGetIdealHeight(hwnd);
		case STATUSBAR_WM_ADD_STATUS:		return StatusBar_OnAddStatus(hwnd, (const wchar_t*)lParam);
		case STATUSBAR_WM_REMOVE_STATUS:	return StatusBar_OnRemoveStatus(hwnd, (unsigned int)(wParam));
		case STATUSBAR_WM_SET_STATUS_TEXT:	return StatusBar_OnSetStatusText(hwnd, (unsigned int)(wParam), (const wchar_t*)lParam);
		case STATUSBAR_WM_SET_STATUS_RTEXT:	return StatusBar_OnSetStatusRightText(hwnd, (unsigned int)(wParam), (const wchar_t*)lParam);
		case STATUSBAR_WM_MOVE_STATUS:		return StatusBar_OnMoveStatus(hwnd, (unsigned int)(wParam), (unsigned int)lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}