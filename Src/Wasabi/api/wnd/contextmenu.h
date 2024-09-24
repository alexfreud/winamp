#ifndef _CONTEXTMENU_H
#define _CONTEXTMENU_H

#include <api/wnd/popup.h>
#include <bfc/common.h>

class DragItem;
class ContextMenuEntry;
class ContextMenuEntryCompare;
class svc_contextCmd;

class ContextMenu : private PopupMenu, private PopupMenuCallback 
{
public:
  ContextMenu(ifc_window *sourceWnd, DragItem *item, bool autopop=TRUE, const wchar_t *menu_path=NULL);
  ContextMenu(ifc_window *sourceWnd, int x, int y, DragItem *item, bool autopop=TRUE, const wchar_t *menu_path=NULL);
  virtual ~ContextMenu();

  void addDragItem(DragItem *item, const wchar_t *menu_path=NULL);

  using PopupMenu::popAtXY;
  using PopupMenu::popAtMouse;
  using PopupMenu::addCommand;
  using PopupMenu::addSeparator;
  using PopupMenu::getNumCommands;

protected:
  ContextMenu(DragItem *item, const wchar_t *menu_path);

private:
  virtual void onPostPop(intptr_t result);

  void populate();
  virtual PopupMenu *popupMenuCallback(PopupMenu *parent, intptr_t param);

  DragItem *item;
  StringW menu_path;
  PtrList<svc_contextCmd> svclist;
  PtrListQuickSorted<ContextMenuEntry, ContextMenuEntryCompare> entries;
};

#endif
