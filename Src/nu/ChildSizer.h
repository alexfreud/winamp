#ifndef NULLSOFT_CHILDSIZERH
#define NULLSOFT_CHILDSIZERH

#include "MediaLibraryInterface.h"
typedef struct
{
	int id;
	int type; // 0xLTRB
	RECT rinfo;
}
ChildWndResizeItem;

enum
{
	Stationary = 0x0000,
	ResizeBottom = 0x0001,
	ResizeRight = 0x0010,
	ResizeTop = 0x0100,
	ResizeLeft=0x1000,
	DockToBottom = 0x0101,
	DockToBottomRight = 0x1111,
};
class ChildSizer
{
	typedef void (*ChildResizeFunc)(HWND, ChildWndResizeItem*, int);
public:
	ChildSizer();

	void Init(HWND dlg, ChildWndResizeItem *list, int count);

	void Resize(HWND dlg, ChildWndResizeItem *list, int count);
	static ChildWndResizeItem *Lookup(int id, ChildWndResizeItem *list, size_t numElements);

	ChildResizeFunc childresize_init, childresize_resize;

};
extern ChildSizer childSizer;
#endif
