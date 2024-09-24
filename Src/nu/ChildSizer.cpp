#include "ChildSizer.h"

ChildSizer childSizer;

ChildSizer::ChildSizer()
: childresize_init(0),
childresize_resize(0)
{}

void ChildSizer::Init(HWND dlg, ChildWndResizeItem *list, int count)
{
	if (!childresize_init)
		childresize_init = (ChildResizeFunc)mediaLibrary.GetWADLGFunc(32);
	childresize_init(dlg, list, count);
}

void ChildSizer::Resize(HWND dlg, ChildWndResizeItem *list, int count)
{
	if (!childresize_resize)
		childresize_resize = (ChildResizeFunc)mediaLibrary.GetWADLGFunc(33);
	childresize_resize(dlg, list, count);
}

ChildWndResizeItem *ChildSizer::Lookup(int id, ChildWndResizeItem *list, size_t numElements)
{
	for (size_t i=0;i!=numElements;i++)
	{
		if (list[i].id == id)
			return &list[i];
	}
	return 0;
}

