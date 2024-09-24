#include <precomp.h>

#include "cfglist.h"
#include <bfc/ptrlist.h>

void CfgList::addItem(CfgItem *cfgitem) 
{
  if (cfgitem == NULL ||
      cfgitem->getGuid() == INVALID_GUID ||
      list.haveItem(cfgitem)) return;
  viewer_addViewItem(cfgitem);
  list.addItem(cfgitem);

  cfgitem->onRegister();	// recurses children
}

void CfgList::delItem(CfgItem *cfgitem) 
{
  if (cfgitem == NULL || !list.haveItem(cfgitem)) return;

  list.removeItem(cfgitem);
  viewer_delViewItem(cfgitem);

  cfgitem->onDeregister();	// recurses children
}

int CfgList::getNumItems()
{
  return list.getNumItems();
}

CfgItem *CfgList::enumItem(int n) 
{
  return list[n];
}

CfgItem *CfgList::getByGuid(GUID g)
{
  if (g == INVALID_GUID) return NULL;
  foreach(list)
    if (list.getfor()->getGuid() == g) return list.getfor();
  endfor
  return NULL;
}

int CfgList::viewer_onItemDeleted(CfgItem *item) 
{
  list.removeItem(item);
  return 1;
}
