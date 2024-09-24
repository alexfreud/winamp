#include <precomp.h>

#include "contextmenu.h"
#include <api/service/svcs/svc_contextcmd.h>
#include <bfc/string/StringW.h>

#define DD_CONTEXTMENUENTRY "ContextMenuEntry v1"

class ContextMenuEntry
{
public:
	ContextMenuEntry(DragItem *_item, svc_contextCmd *_svc, int _pos, const wchar_t *txt, int _sortval, int _addorder) :
	svc(_svc), item(_item), pos(_pos), text(txt), sortval(_sortval), addorder(_addorder) { }

	svc_contextCmd *svc;
	DragItem *item;
	int pos;
	StringW text, submenu_text;
	int sortval;
	int addorder;
};

class ContextMenuEntryCompare
{
public:
	static int compareItem(void *p1, void* p2)
	{
		ContextMenuEntry *e1 = static_cast<ContextMenuEntry*>(p1);
		ContextMenuEntry *e2 = static_cast<ContextMenuEntry*>(p2);
		int ret = CMP3(e1->sortval, e2->sortval);
		if (ret == 0) ret = CMP3(e1->addorder, e2->addorder);
		return ret;
	}
};

ContextMenu::ContextMenu(ifc_window *sourceWnd, DragItem *_item, bool autopop, const wchar_t *_menu_path)
		: PopupMenu(sourceWnd), item(_item), menu_path(_menu_path)
{
	populate();
	if (autopop) popAtMouse();
}

ContextMenu::ContextMenu(ifc_window *sourceWnd, int x, int y, DragItem *_item, bool autopop, const wchar_t *_menu_path)
		: PopupMenu(sourceWnd), item(_item), menu_path(_menu_path)
{
	populate();
	if (autopop) popAtXY(x, y);
}

ContextMenu::ContextMenu(DragItem *_item, const wchar_t *_menu_path)
		: item(_item), menu_path(_menu_path)
{
	populate();
}

ContextMenu::~ContextMenu()
{
	entries.deleteAll();

	// release all services
	for (int i = 0; i < svclist.getNumItems(); i++)
		SvcEnum::release(svclist.enumItem(i));
}

void ContextMenu::addDragItem(DragItem *_item, const wchar_t *_menu_path)
{
	menu_path = _menu_path;
	item = _item;
	populate();
}

void ContextMenu::populate()
{
	if (item == NULL) return ;

	ContextCmdEnum cce(item, menu_path);
	svc_contextCmd *svc;
	int i, j, addorder = 0;

	// make a list of all context cmd services that match the menu path
	for (i = 0; (svc = cce.getNext()) != NULL; i++)
	{
		for (j = 0; ; j++)
		{
			const wchar_t *text = svc->getCommand(item, j);
			if (text == NULL) break;
			if (!wcscmp(text, L"~~~SEP~~~")) text = NULL;	// sorry, magic value
			ContextMenuEntry *entry = new ContextMenuEntry(item, svc, j, text, svc->getSortVal(item, j), addorder++);
			entries.addItem(entry);
		}
		// save the service * to release later
		svclist.addItem(svc);
	}

	// sorting is implicit but just making sure
	entries.sort();

	PtrList<StringW> submenu_list;

#ifdef WASABI_COMPILE_COMPONENTS
	GUID prev = INVALID_GUID;
#endif
	// populate the menu from the list
	int n = entries.getNumItems();
	for (i = 0; i < n; i++)
	{
		ContextMenuEntry *entry = entries.enumItem(i);
		if (entry->text.isempty())
		{
			addSeparator();
		}
		else
		{
			svc_contextCmd *svc = entry->svc;
#ifdef WASABI_COMPILE_COMPONENTS
			GUID g = WASABI_API_SVC->service_getOwningComponent(svc);
			if (g != prev && prev != INVALID_GUID && i < n - 1)
				addSeparator();
			prev = g;
#endif
			if (!svc->getSubMenu(item, menu_path))
			{
				int checked = entry->svc->getChecked(item, entry->pos);
				int enabled = entry->svc->getEnabled(item, entry->pos);
				addCommand(entry->text, reinterpret_cast<intptr_t>(entry), checked, !enabled);
			}
			else
			{
				entry->submenu_text = svc->getSubMenuText(menu_path);
				if (!entry->submenu_text.isempty())
				{
					for (j = 0; j < submenu_list.getNumItems(); j++)
						if (!WCSICMP(*submenu_list[j], entry->submenu_text)) break;
					if (j >= submenu_list.getNumItems())
					{
						submenu_list.addItem(new StringW(entry->submenu_text));
						addSubMenuCallback(entry->submenu_text, this, reinterpret_cast<intptr_t>(entry));
					}
				}
			}
		}
	}
	submenu_list.deleteAll();
}

void ContextMenu::onPostPop(intptr_t result)
{
	//if (result == -1 || result == -2 || result == -3) return; //FUCKO need real enums
	if (result < 0) return ;
	ASSERT(result != 0xcccccccc);
	ContextMenuEntry *entry = reinterpret_cast<ContextMenuEntry*>(result);
	if (entry == NULL) return ;
	entry->svc->onCommand(entry->item, entry->pos);
}

PopupMenu *ContextMenu::popupMenuCallback(PopupMenu *parent, intptr_t param)
{
	ContextMenuEntry *entry = reinterpret_cast<ContextMenuEntry*>(param);
	StringW path = menu_path;
	if (!path.isempty())
		path.cat(L"/");
	path.cat(entry->submenu_text);
	ContextMenu *ret = new ContextMenu(entry->item, path);
	if (ret->getNumCommands() <= 0)
	{
		delete ret;
		ret = NULL;
	}
	return ret;
}
