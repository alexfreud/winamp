#ifndef __WASABI_API_REGION_H
#define __WASABI_API_REGION_H

#include <bfc/dispatch.h>

class api_region : public Dispatchable
{
protected:
	api_region() {}
	virtual ~api_region() {}

public:
	DISPATCH_CODES
	{
	    REGION_GETOSHANDLE = 50,
	    REGION_CLONE = 100,
	    REGION_DISPOSECLONE = 110,
	    REGION_PTINREGION = 120,
	    REGION_OFFSET = 130,
	    REGION_GETBOX = 140,
	    REGION_SUBTRACTRGN = 150,
	    REGION_SUBTRACTRECT = 160,
	    REGION_ADDRECT = 170,
	    REGION_ADD = 180,
	    REGION_AND = 190,
	    REGION_SETRECT = 200,
	    REGION_EMPTY = 210,
	    REGION_ISEMPTY = 220,
	    REGION_EQUALS = 230,
	    REGION_ENCLOSED = 240,
	    REGION_INTERSECTRGN = 250,
	    REGION_DOESINTERSECTRGN = 251,
	    REGION_INTERSECTRECT = 260,
	    REGION_ISRECT = 270,
	    REGION_SCALE = 280,
	    REGION_DEBUG = 290,
	    REGION_MAKEWNDREGION = 300,
	    REGION_GETNUMRECTS = 310,
	    REGION_ENUMRECT = 320,
	};
public:
	OSREGIONHANDLE getOSHandle(); // avoid as much as you can, should be used only when you need to call the OS api

	api_region *clone();
	void disposeClone(api_region *r);
	bool ptInRegion(const POINT *pt);
	void offset(int x, int y);
	void getBox(RECT *r);
	void subtractRegion(const api_region *r);
	void subtractRgn(const api_region *r) { subtractRegion(r); } //DEPRECATED
	void subtractRect(const RECT *r);
	void addRect(const RECT *r);
	void addRegion(const api_region *r);
	void andRegion(const api_region *r);
	void setRect(const RECT *r);
	void empty();
	int isEmpty();
	int equals(const api_region *r);
	int enclosed(const api_region *r, api_region *outside = NULL);
	int intersectRgn(const api_region *r, api_region *intersection);
	int doesIntersectRgn(const api_region *r);
	int intersectRect(const RECT *r, api_region *intersection);

	int isRect();
	void scale(double sx, double sy, bool round = 0);
	void debug(int async = 0);
	OSREGIONHANDLE makeWindowRegion(); // gives you a handle to a clone of the OSREGION object so you can insert it into a window's region with SetWindowRgn. ANY other use is prohibited

	// this is how you can enumerate the subrects that compose to make up the
	// entire region
	int getNumRects();
	int enumRect(int n, RECT *r);
};

inline OSREGIONHANDLE api_region::getOSHandle()
{
	return _call(REGION_GETOSHANDLE, (OSREGIONHANDLE)NULL);
}

inline api_region *api_region::clone()
{
	return _call(REGION_CLONE, (api_region *)NULL);
}

inline void api_region::disposeClone(api_region *r)
{
	_voidcall(REGION_DISPOSECLONE, r);
}

inline bool api_region::ptInRegion(const POINT *pt)
{
	return _call(REGION_PTINREGION, false, pt);
}

inline void api_region::offset(int x, int y)
{
	_voidcall(REGION_OFFSET, x, y);
}

inline void api_region::getBox(RECT *r)
{
	_voidcall(REGION_GETBOX, r);
}

inline void api_region::subtractRegion(const api_region *reg)
{
	_voidcall(REGION_SUBTRACTRGN, reg);
}

inline void api_region::subtractRect(const RECT *r)
{
	_voidcall(REGION_SUBTRACTRECT, r);
}

inline void api_region::addRect(const RECT *r)
{
	_voidcall(REGION_ADDRECT, r);
}

inline void api_region::addRegion(const api_region *r)
{
	_voidcall(REGION_ADD, r);
}

inline void api_region::andRegion(const api_region *r)
{
	_voidcall(REGION_AND, r);
}

inline void api_region::setRect(const RECT *r)
{
	_voidcall(REGION_SETRECT, r);
}

inline void api_region::empty()
{
	_voidcall(REGION_EMPTY);
}

inline int api_region::isEmpty()
{
	return _call(REGION_ISEMPTY, 0);
}

inline int api_region::equals(const api_region *r)
{
	return _call(REGION_EQUALS, 0, r);
}

inline int api_region::enclosed(const api_region *r, api_region *outside)
{
	return _call(REGION_ENCLOSED, 0, r, outside);
}

inline int api_region::intersectRgn(const api_region *r, api_region *intersection)
{
	return _call(REGION_INTERSECTRGN, 0, r, intersection);
}

inline int api_region::doesIntersectRgn(const api_region *r)
{
	return _call(REGION_DOESINTERSECTRGN, 0, r);
}

inline int api_region::intersectRect(const RECT *r, api_region *intersection)
{
	return _call(REGION_INTERSECTRECT, 0, r, intersection);
}

inline int api_region::isRect()
{
	return _call(REGION_ISRECT, 0);
}

inline void api_region::scale(double sx, double sy, bool round)
{
	_voidcall(REGION_SCALE, sx, sy, round);
}

inline void api_region::debug(int async)
{
	_voidcall(REGION_DEBUG, async);
}

inline OSREGIONHANDLE api_region::makeWindowRegion()
{
	return _call(REGION_MAKEWNDREGION, (OSREGIONHANDLE)NULL);
}

inline int api_region::getNumRects()
{
	return _call(REGION_GETNUMRECTS, 0);
}

inline int api_region::enumRect(int n, RECT *r)
{
	return _call(REGION_ENUMRECT, 0, n, r);
}

#endif
