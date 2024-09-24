#ifndef _CFGLIST_H
#define _CFGLIST_H

#include <bfc/wasabi_std.h>
#include <bfc/depview.h>
#include <api/config/items/cfgitem.h>

class CfgList : public DependentViewerTPtr<CfgItem> {
public:
  void addItem(CfgItem *cfgitem);
  void delItem(CfgItem *cfgitem);

  int getNumItems();
  CfgItem *enumItem(int n);
  CfgItem *getByGuid(GUID g);

  virtual int viewer_onItemDeleted(CfgItem *item);

private:
  PtrList<CfgItem> list;
};

#endif
