#include <precomp.h>
#include "snappnt.h"
#include <api/wndmgr/layout.h>
#include <api/wnd/wndtrack.h>
#include <api/config/items/cfgitem.h>

SnapPoint::SnapPoint(Layout *l, Container *c)
{
	points.addItem(this);
	setParentLayout(l);
	setParentContainer(c);
}

SnapPoint::~SnapPoint()
{
	points.removeItem(this);
}

void SnapPoint::removeAll()
{
	points.deleteAllSafe();
}

int SnapPoint::setXmlParam(const wchar_t *paramname, const wchar_t *strvalue)
{
	if (!WCSICMP(paramname, L"id")) id = strvalue;
	else if (!WCSICMP(paramname, L"x")) x = WTOI(strvalue);
	else if (!WCSICMP(paramname, L"y")) y = WTOI(strvalue);
	else if (!WCSICMP(paramname, L"relatx")) relatx = WTOI(strvalue);
	else if (!WCSICMP(paramname, L"relaty")) relaty = WTOI(strvalue);
	else return 0;
	return 1;
}

void SnapPoint::setParentLayout(Layout *l)
{
	playout = l;
}

void SnapPoint::setParentContainer(Container *c)
{
	pcontainer = c;
}

Container *SnapPoint::getParentContainer()
{
	return pcontainer;
}

Layout *SnapPoint::getParentLayout()
{
	return playout;
}

const wchar_t *SnapPoint::getId()
{
	return id;
}

int SnapPoint::getX()
{
	if (getParentLayout())
		return (int)((double)x * getParentLayout()->getRenderRatio());
	return x;
}

int SnapPoint::getY()
{
	if (getParentLayout())
		return (int)((double)y * getParentLayout()->getRenderRatio());
	return y;
}

PtrList<SnapPoint> SnapPoint::points;

int SnapPoint::match(ifc_window *master, RECT *z, ifc_window *slave, int flag, int *donex, int *doney, int w, int h)
{
	SnapPoint *pmast;
	SnapPoint *pslav;

	for (int i = 0;i < points.getNumItems();i++)
		if (points.enumItem(i)->getParentLayout() == master)
		{
			pmast = points.enumItem(i);
			for (int j = 0;j < points.getNumItems();j++)
				if (points.enumItem(j)->getParentLayout() == slave)
				{
					pslav = points.enumItem(j);
					int r = do_match(pmast, pslav, z, flag, donex, doney, w, h);
					if (r)
						return 1;
				}
		}
	return 0;
}

int SnapPoint::do_match(SnapPoint *pmast, SnapPoint *pslav, RECT *z, int mask, int *donex, int *doney, int w, int h)
{
	//#if 0//BU> lone needs to make this work again
	//fg> maybe it would have been a good idea to tell me about it... especially since it *works fine*

	ASSERT(pmast);
	ASSERT(pslav);
	int f = 0;

	if (((mask & KEEPSIZE) == 0) && !z)
	{
		ASSERTPR(0, "match resize with no rect");
	}

	if (!z)
	{ // just testing if docked

		if (!WCSICMP(pmast->getId(), pslav->getId()))
		{
			BaseWnd *wm = pmast->getParentLayout();
			BaseWnd *ws = pslav->getParentLayout();
			if (ws && ws)
			{
				RECT pmr, psr;
				wm->getWindowRect(&pmr);
				ws->getWindowRect(&psr);
				pmr.left += pmast->relatx ? pmast->getX() : (pmr.right - pmr.left + pmast->getX());
				pmr.top += pmast->relaty ? pmast->getY() : (pmr.bottom - pmr.top + pmast->getY());
				psr.left += pmast->relatx ? pslav->getX() : (psr.right - psr.left + pslav->getX());
				psr.top += pmast->relaty ? pslav->getY() : (psr.bottom - psr.top + pslav->getY());
				if (pmr.left == psr.left && pmr.top == psr.top)
				{
					if (donex) *donex = 1;
					if (doney) *doney = 1;
					return 1;
				}
			}
		}
	}
	else
	{

		ASSERT(donex);
		ASSERT(doney);

		if (!WCSICMP(pmast->getId(), pslav->getId()))
		{
			//CUT: BaseWnd *wm = pmast->getParentLayout();
			BaseWnd *ws = pslav->getParentLayout();
			if (ws && ws)
			{
				RECT pmr, psr, osr, omr;
				pmr = omr = *z;
				ws->getWindowRect(&psr);
				osr = psr;
				pmr.left += pmast->relatx ? pmast->getX() : (pmr.right - pmr.left + pmast->getX());
				pmr.top += pmast->relaty ? pmast->getY() : (pmr.bottom - pmr.top + pmast->getY());
				psr.left += pmast->relatx ? pslav->getX() : (psr.right - psr.left + pslav->getX());
				psr.top += pmast->relaty ? pslav->getY() : (psr.bottom - psr.top + pslav->getY());
				if (pmr.left > psr.left - 10 && pmr.left < psr.left + 10 && (mask & LEFT) && ! *donex)
				{
					*donex = 1;
					z->left = omr.left - (pmr.left - psr.left);
					if (mask & KEEPSIZE)
						z->right = z->left + w;
					f++;
				}
				if (pmr.top > psr.top - 10 && pmr.top < psr.top + 10 && (mask & TOP) && ! *doney)
				{
					z->top = omr.top - (pmr.top - psr.top);
					*doney = 1;
					if (mask & KEEPSIZE)
						z->bottom = z->top + h;
					f++;
				}
			}
		}

		return f > 0;
	}
	return 0;
}



