#include <precomp.h>
#include "SelItemList.h"
#include "listwnd.h"

#define SELITEMEXPAND 2040

SelItemList::SelItemList(ListWnd *parent) :
listwnd(parent), list(SELITEMEXPAND), num_selected(0) { }

void SelItemList::setSelected(int pos, int selected, int cb)
{
	int s = list.getSize();
	if (pos >= s)
	{
		if (!selected)
		{
			// turning off something that's out of range is a no-brainer
			return ;
		}
		s = ((s / SELITEMEXPAND) + 1) * SELITEMEXPAND;
		list.setSize(s);
	}

	char *l = list.getMemory();
	if (selected)
	{
		// quick and dirty way to prevent more than one item from
		// ever being selected?
		if (listwnd->getPreventMultipleSelection())
		{
			listwnd->deselectAll(cb);
		}
		if (!isSelected(pos))
		{
			l[pos] = 1;
			num_selected++;
			if (cb) listwnd->itemSelection(pos, TRUE);
			listwnd->invalidateItem(pos);
		}
	}
	else
	{
		if (isSelected(pos))
		{
			l[pos] = 0;
			num_selected--;
			if (cb) listwnd->itemSelection(pos, FALSE);
			listwnd->invalidateItem(pos);
		}
	}
}
int SelItemList::isSelected(int pos)
{
	if (pos >= list.getSize()) return 0;
	return list.getMemory()[pos];
}
int SelItemList::getNumSelected() { return num_selected; }

void SelItemList::deleteByPos(int pos)
{
	ASSERT(pos >= 0);
	int s = list.getSize();
	if (pos >= s) return ;
	num_selected -= isSelected(pos);
	char *m = list.getMemory() + pos;
	MEMCPY(m, m + 1, s - (pos + 1));
}
void SelItemList::deselectAll()
{	// for internal use, doesn't send callbacks
	list.zeroMemory();
	num_selected = 0;
}
