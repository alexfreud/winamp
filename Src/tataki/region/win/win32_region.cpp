#if defined _WIN64 || defined _WIN32
#include <tataki/api__tataki.h>
#include "region.h"
#include <api/imgldr/api_imgldr.h>
#include <tataki/region/api_region.h>
#include <tataki/canvas/ifc_canvas.h>
#include <api/wnd/basewnd.h>


#define GETOSHANDLE(x) (const_cast<api_region *>(x)->getOSHandle())

#define CBCLASS RegionI
START_DISPATCH;
CB(REGION_GETOSHANDLE, getOSHandle);
CB(REGION_CLONE, clone);
VCB(REGION_DISPOSECLONE, disposeClone);
CB(REGION_PTINREGION, ptInRegion);
VCB(REGION_OFFSET, offset);
VCB(REGION_GETBOX, getBox);
VCB(REGION_SUBTRACTRGN, subtractRegion);
VCB(REGION_SUBTRACTRECT, subtractRect);
VCB(REGION_ADDRECT, addRect);
VCB(REGION_ADD, addRegion);
VCB(REGION_AND, andRegion);
VCB(REGION_SETRECT, setRect);
VCB(REGION_EMPTY, empty);
CB(REGION_ISEMPTY, isEmpty);
CB(REGION_EQUALS, equals);
CB(REGION_ENCLOSED, enclosed);
CB(REGION_INTERSECTRECT, intersectRect);
CB(REGION_DOESINTERSECTRGN, doesIntersectRgn);
CB(REGION_INTERSECTRGN, intersectRgn);
CB(REGION_ISRECT, isRect);
VCB(REGION_SCALE, scale);
VCB(REGION_DEBUG, debug);
CB(REGION_MAKEWNDREGION, makeWindowRegion);
CB(REGION_GETNUMRECTS, getNumRects);
CB(REGION_ENUMRECT, enumRect);
END_DISPATCH;
#undef CBCLASS

#define CHECK_REGION \
	if (hrgn == NULL) hrgn = CreateRectRgn(0,0,0,0);

RegionI::RegionI()
{
	hrgn = CreateRectRgn(0, 0, 0, 0);
	init();
}

RegionI::RegionI(const RECT *r)
{
	hrgn = 0;
	init();
	optrect = *r;
	optimized = 1;
	//hrgn = CreateRectRgn(r->left,r->top,r->right,r->bottom);
	//if (!hrgn) hrgn = CreateRectRgn(0,0,0,0);
	//init();
	//optimize();
}

RegionI::RegionI(int l, int t, int r, int b)
{
	hrgn = 0;
	init();
	optrect.left = l;
	optrect.top = t;
	optrect.right = r;
	optrect.bottom = b;
	optimized = 1;

	//hrgn = CreateRectRgn(l,t,r,b);
	//if (!hrgn) hrgn = CreateRectRgn(0,0,0,0);
	//init();
	//optimize();
}

RegionI::RegionI(OSREGIONHANDLE r)
{
	OSREGIONHANDLE R = CreateRectRgn(0, 0, 0, 0);
	CombineRgn(R, r, r, RGN_COPY);
	hrgn = R;
	init();
	optimize();
}

RegionI::RegionI(const RegionI *copy)
{
	init();
	if (copy->optimized)
	{
		optrect = copy->optrect;
		optimized = copy->optimized;
		hrgn = 0;
	}
	else
	{
		hrgn = CreateRectRgn(0, 0, 0, 0);
		CombineRgn(hrgn, copy->hrgn, copy->hrgn, RGN_COPY);
	}
}

RegionI::RegionI(Canvas *c, RECT *defbounds)
{
	hrgn = CreateRectRgn(0, 0, 0, 0);
	if (!GetClipRgn(c->getHDC(), hrgn))
	{
		if (defbounds != NULL)
		{
			SetRectRgn(hrgn, defbounds->left, defbounds->top, defbounds->right, defbounds->bottom);
			optrect=*defbounds;
			optimized=1;
		}
	}
	init();
	optimize();
}

RegionI::~RegionI()
{
	delete lastdebug;
	if (srv != NULL) srv->delRef(this);
	ASSERT(clonecount == 0);
	if (srv == NULL && hrgn != NULL) DeleteObject(hrgn);
}

void RegionI::init()
{
	srv = NULL;
	clonecount = 0;
	lastdebug = NULL;
	optimized = 0;
}

api_region *RegionI::clone()
{
	api_region *newregion = new RegionI(this);
	clonecount++;
	return newregion;
}

void RegionI::disposeClone(api_region *r)
{
	RegionI *ri = static_cast<RegionI *>(r);
	delete ri; // todo: validate pointer before deleting
	clonecount--;
}

// returns a handle that SetWindowRgn understands (non portable). We should NOT delete this handle, windows will delete
// it by itself upon setting a new region of destroying the window
OSREGIONHANDLE RegionI::makeWindowRegion()
{
	deoptimize();
	OSREGIONHANDLE R = CreateRectRgn(0, 0, 0, 0);
	CombineRgn(R, hrgn, hrgn, RGN_COPY);
	optimize();
	return R;
}

RegionI::RegionI(SkinBitmap *bitmap, RECT *r, int xoffset, int yoffset, bool inverted, int dothreshold, char threshold, int thinverse, int minalpha)
{
	init();
	const wchar_t *id = bitmap->getBitmapName();

	if (xoffset == 0 && yoffset == 0 && r == NULL && !inverted && !dothreshold && minalpha == 1 && id != NULL && *id != 0)
	{
		srv = WASABI_API_IMGLDR->imgldr_requestSkinRegion(id);
		if (srv != NULL)
		{
			srv->addRef(this);
			hrgn = srv->getRegion()->getOSHandle();
		}
	}

	if (srv == NULL)
	{
		if (r)
			hrgn = alphaToRegionRect(bitmap, xoffset, yoffset, TRUE, r->left, r->top, r->right - r->left, r->bottom - r->top, inverted, dothreshold, threshold, thinverse, minalpha);
		else
			hrgn = alphaToRegionRect(bitmap, xoffset, yoffset, FALSE, 0, 0, 0, 0, inverted, dothreshold, threshold, thinverse, minalpha);

		if (id != NULL && *id != 0)
		{
			if (xoffset == 0 && yoffset == 0 && r == NULL && !inverted && !dothreshold && minalpha == 1)
			{
				WASABI_API_IMGLDR->imgldr_cacheSkinRegion(id, this);
				srv = WASABI_API_IMGLDR->imgldr_requestSkinRegion(id);
				if (srv != NULL)
				{
					srv->addRef(this);
					DeleteObject(hrgn);
					hrgn = srv->getRegion()->getOSHandle();
				}
			}
		}
	}
	optimize();

}

OSREGIONHANDLE RegionI::alphaToRegionRect(SkinBitmap *bitmap, int xoffset, int yoffset, bool portion, int _x, int _y, int _w, int _h, bool inverted, int dothreshold, unsigned char threshold, int thinverse, int minalpha)
{
	return alphaToRegionRect(bitmap->getBits(), bitmap->getX(), bitmap->getY(), bitmap->getWidth(), bitmap->getHeight(), bitmap->getFullWidth(), bitmap->getFullHeight(), xoffset, yoffset, portion, _x, _y, _w, _h, inverted, dothreshold, threshold, thinverse, minalpha);
}

OSREGIONHANDLE RegionI::alphaToRegionRect(void *pbits32, int bmX, int bmY, int bmWidth, int bmHeight, int fullw, int fullh, int xoffset, int yoffset, bool portion, int _x, int _y, int _w, int _h, bool inverted, int dothreshold, unsigned char threshold, int thinverse, int minalpha)
{
	OSREGIONHANDLE hRgn = NULL;
	if (!pbits32) return NULL;

	RGNDATA *pData;
	int y, x;

	// For better performances, we will use the ExtCreateRegion() function to create the
	// region. This function take a RGNDATA structure on entry. We will add rectangles by
	// amount of ALLOC_UNIT number in this structure.
	// JF> rects are 8 bytes, so this allocates just under 16kb of memory, no need to REALLOC
#define MAXRECTS 2000
	__int8 regionMemory[sizeof(RGNDATAHEADER) + (sizeof(RECT) * MAXRECTS)] = {0};
	//pData = (RGNDATA *)MALLOC(sizeof(RGNDATAHEADER) + (sizeof(RECT) * MAXRECTS));
	pData = (RGNDATA *)regionMemory;
	//if (!pData) return NULL;

	pData->rdh.dwSize = sizeof(RGNDATAHEADER);
	pData->rdh.iType = RDH_RECTANGLES;
	pData->rdh.nCount = pData->rdh.nRgnSize = 0;

	SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

	int x_end = (portion ? _w + _x : bmWidth);
	int y_end = (portion ? _h + _y : bmHeight);
	int x_start = (portion ? _x : 0);
	int y_start = (portion ? _y : 0);

	x_start += bmX;
	x_end += bmX;
	y_start += bmY;
	y_end += bmY;

	unsigned int iv = minalpha << 24; //inverted?0xff000000:0;

	int shiftx = xoffset - bmX;
	int shifty = yoffset - bmY;

	for (y = y_start; y < y_end; y++)
	{
		// Scan each bitmap pixel from left to right
		unsigned int *lineptr = ((unsigned int *)pbits32) + fullw * y;
		for (x = x_start; x < x_end; x++)
		{
			// Search for a continuous range of "non transparent pixels"
			int x0 = x;
			unsigned int *p = lineptr;
			if (dothreshold)
			{
				if (inverted)
				{
					if (thinverse)
					{
						while (x < x_end)
						{
							unsigned int a = p[x];
							if ((a&0xff000000) >= iv ||
							    (((((a & 0xFF) > threshold || ((a & 0xFF00) >> 8) > threshold || ((a & 0xFF0000) >> 16) > threshold)))))
								break;
							x++;
						}
					}
					else
					{
						while (x < x_end)
						{
							unsigned int a = p[x];
							if ((a&0xff000000) >= iv ||
							    (((((a & 0xFF) < threshold || ((a & 0xFF00) >> 8) < threshold || ((a & 0xFF0000) >> 16) < threshold)))))
								break;
							x++;
						}
					}
				}
				else
				{
					if (thinverse)
					{
						while (x < x_end)
						{
							unsigned int a = p[x];
							if ((a&0xff000000) < iv ||
							    (((((a & 0xFF) > threshold || ((a & 0xFF00) >> 8) > threshold || ((a & 0xFF0000) >> 16) > threshold)))))
								break;
							x++;
						}
					}
					else
					{
						while (x < x_end)
						{
							unsigned int a = p[x];
							if ((a&0xff000000) < iv ||
							    (((((a & 0xFF) < threshold || ((a & 0xFF00) >> 8) < threshold || ((a & 0xFF0000) >> 16) < threshold)))))
								break;
							x++;
						}
					}
				}
			}
			else
			{
				if (inverted)
				{
					while (x < x_end)
					{
						if ((p[x] & 0xFF000000) >= iv) break;
						x++;
					}
				}
				else
				{
					while (x < x_end)
					{
						if ((p[x] & 0xFF000000) < iv) break;
						x++;
					}
				}
			}

			if (x > x0)
			{
				SetRect(((RECT *)&pData->Buffer) + pData->rdh.nCount, x0 + shiftx, y + shifty, x + shiftx, y + 1 + shifty);

				pData->rdh.nCount++;

				if (x0 + shiftx < pData->rdh.rcBound.left) pData->rdh.rcBound.left = x0 + shiftx;
				if (y + shifty < pData->rdh.rcBound.top) pData->rdh.rcBound.top = y + shifty;
				if (x + shiftx > pData->rdh.rcBound.right) pData->rdh.rcBound.right = x + shiftx;
				if (y + 1 + shifty > pData->rdh.rcBound.bottom) pData->rdh.rcBound.bottom = y + 1 + shifty;

				// On Windows98, ExtCreateRegion() may fail if the number of rectangles is too
				// large (ie: > 4000). Therefore, we have to create the region by multiple steps.
				if (pData->rdh.nCount == MAXRECTS)
				{
					OSREGIONHANDLE h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * pData->rdh.nCount), pData);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else hRgn = h;
					pData->rdh.nCount = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
				}
			}
		}
	}

	// Create or extend the region with the remaining rectangles
	OSREGIONHANDLE h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * pData->rdh.nCount), pData);
	if (hRgn)
	{
		CombineRgn(hRgn, hRgn, h, RGN_OR);
		DeleteObject(h);
	}
	else
		hRgn = h;

	// Clean up
	//FREE(pData);

	return hRgn;
}

bool RegionI::ptInRegion(const POINT *pt)
{
	if (optimized) return !!PtInRect(&optrect, *pt);
	CHECK_REGION
	return !!PtInRegion(hrgn, pt->x, pt->y);
}

void RegionI::offset(int x, int y)
{
	if (optimized)
	{
		optrect.left += x;
		optrect.top += y;
		optrect.right += x;
		optrect.bottom += y;
		return ;
	}
	CHECK_REGION
	if (srv)
	{
		hrgn = CreateRectRgn(0, 0, 0, 0);
		RegionServer *s = srv;
		srv = NULL;
		addRegion(s->getRegion());
		s->delRef(this);
	}
	if (x == 0 && y == 0) return ;
	deoptimize(); // because addregion may have optimized it
	OffsetRgn(hrgn, x, y);
	optimize();
}

void RegionI::getBox(RECT *r)
{
	if (optimized)
	{
		*r = optrect;
		return ;
	}
	CHECK_REGION
	GetRgnBox(hrgn, r);
}

OSREGIONHANDLE RegionI::getOSHandle()
{
	deoptimize();
	CHECK_REGION
	return hrgn;
}

void RegionI::subtractRect(const RECT *r)
{
	RegionI s(r);
	subtractRegion(&s);
}

void RegionI::subtractRegion(const api_region *reg)
{
	if (srv)
	{
		hrgn = CreateRectRgn(0, 0, 0, 0);
		RegionServer *s = srv;
		srv = NULL;
		addRegion(s->getRegion());
		s->delRef(this);
	}
	deoptimize();
	CombineRgn(hrgn, hrgn, GETOSHANDLE(reg), RGN_DIFF);
	optimize();
}

void RegionI::andRegion(const api_region *reg)
{
	if (srv)
	{
		hrgn = CreateRectRgn(0, 0, 0, 0);
		RegionServer *s = srv;
		srv = NULL;
		addRegion(s->getRegion());
		s->delRef(this);
	}

	deoptimize();
	CombineRgn(hrgn, hrgn, GETOSHANDLE(reg), RGN_AND);
	optimize();
}

void RegionI::addRect(const RECT *r)
{
	RegionI a(r);
	addRegion(&a);
}

void RegionI::addRegion(const api_region *reg)
{
	if (srv)
	{
		hrgn = CreateRectRgn(0, 0, 0, 0);
		RegionServer *s = srv;
		srv = NULL;
		addRegion(s->getRegion());
		s->delRef(this);
	}
	deoptimize();
	ASSERT(reg != NULL);
	CombineRgn(hrgn, hrgn, GETOSHANDLE(reg), RGN_OR);
	optimize();
}

int RegionI::isEmpty()
{
	RECT r;
	getBox(&r);
	if (r.left == r.right || r.bottom == r.top) return 1;
	return 0;
}

int RegionI::enclosed(const api_region *r, api_region *outside)
{
	deoptimize();
	OSREGIONHANDLE del = NULL;
	if (!outside)
		del = CreateRectRgn(0, 0, 0, 0);
	int rs = CombineRgn(outside ? outside->getOSHandle() : del, hrgn, GETOSHANDLE(r), RGN_DIFF);
	if (del != NULL) DeleteObject(del);
	optimize();
	return rs == NULLREGION;
}

#define IntersectRgn(hrgnResult, hrgnA, hrgnB) CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_AND)

int RegionI::intersectRgn(const api_region *r, api_region *intersection)
{
	ASSERT(intersection != NULL);
	ASSERT(intersection != this);
	int rs;
	if (optimized)
	{
		deoptimize();
		rs = IntersectRgn(intersection->getOSHandle(), hrgn, GETOSHANDLE(r));
		DeleteObject(hrgn);
		hrgn=NULL;
		optimized=1;
	}
	else
	{
		rs = IntersectRgn(intersection->getOSHandle(), hrgn, GETOSHANDLE(r));
	}

	return (rs != NULLREGION && rs != ERROR);
}

int RegionI::doesIntersectRgn(const api_region *r)
{
	if (optimized)
	{
		return RectInRegion(GETOSHANDLE(r), &optrect);
	}
	else
	{
		CHECK_REGION
		HRGN del = CreateRectRgn(0, 0, 0, 0);
		int rs = IntersectRgn(del, hrgn, GETOSHANDLE(r));
		DeleteObject(del);
		return (rs != NULLREGION && rs != ERROR);
	}

}

int RegionI::intersectRect(const RECT *r, api_region *intersection)
{
	int	rs;
	ASSERT(intersection != NULL);
	ASSERT(intersection != this);
	if (optimized)
	{
		RECT temp = optrect;
		rs = IntersectRect(&temp, &optrect, r);
		intersection->setRect(&temp);
		return rs;
	}
	else
	{
		CHECK_REGION

		OSREGIONHANDLE iRgn = intersection->getOSHandle();
		SetRectRgn(iRgn, r->left, r->top, r->right, r->bottom);
		rs = IntersectRgn(iRgn, hrgn, iRgn);
	}
	return (rs != NULLREGION && rs != ERROR);
}

int RegionI::doesIntersectRect(const RECT *r)
{
	return RectInRegion(hrgn, r);
}

void RegionI::empty()
{
	if (srv)
	{
		hrgn = CreateRectRgn(0, 0, 0, 0);
		ASSERT(hrgn != NULL);
		srv->delRef(this);
		srv = NULL;
		optimize();
		return ;
	}
	//deoptimize();
	if (hrgn != NULL)
		DeleteObject(hrgn);
	hrgn=NULL;
	//hrgn = CreateRectRgn(0, 0, 0, 0);
	optrect.left=0;
	optrect.top=0;
	optrect.right=0;
	optrect.bottom=0;
	optimized=1;
	//ASSERT(hrgn != NULL);
	//optimize();
}

void RegionI::setRect(const RECT *r)
{
	if (srv)
	{
		hrgn = CreateRectRgnIndirect(r);
		srv->delRef(this);
		srv = NULL;
		optimize();
		return ;
	}
	//deoptimize();
	//CHECK_REGION
		if (hrgn)
			DeleteObject(hrgn);
	hrgn=NULL;
	//SetRectRgn(hrgn, r->left, r->top, r->right, r->bottom);
	optrect = *r;
	optimized = 1;
	//optimize();
}

int RegionI::equals(const api_region *r)
{
	ASSERT(r);
	api_region *cl = const_cast<api_region*>(r)->clone();
	cl->subtractRegion(this);
	int ret = cl->isEmpty();
	const_cast<api_region*>(r)->disposeClone(cl);
	cl = clone();
	cl->subtractRegion(r);
	ret &= cl->isEmpty();
	disposeClone(cl);
	return ret;
}

int RegionI::isRect()
{
	if (optimized) return 1;
	RECT r;
	getBox(&r);
	RegionI n(&r);
	return equals(&n);
}

void RegionI::scale(double sx, double sy, bool round)
{
	if (srv)
	{
		hrgn = CreateRectRgn(0, 0, 0, 0);
		RegionServer *s = srv;
		srv = NULL;
		addRegion(s->getRegion());
		s->delRef(this);
	}
	deoptimize();
	CHECK_REGION
	DWORD size = 0;
	RECT box;
	getBox(&box);
	size = GetRegionData(hrgn, size, NULL);
	if (!size) return ;
	RGNDATA *data = (RGNDATA *)MALLOC(size);
	RECT *r = (RECT *)data->Buffer;

	GetRegionData(hrgn, size, (RGNDATA *)data);
	double adj = round ? 0.99999 : 0.0;
	int iadj = round ? 1 : 0;

	if (data->rdh.nCount == 1)
	{
		RECT nr = box;
		nr.left = (int)((double)nr.left * sx - iadj);
		nr.top = (int)((double)nr.top * sy - iadj);
		nr.right = (int)((double)nr.right * sx + adj);
		nr.bottom = (int)((double)nr.bottom * sy + adj);
		setRect(&nr);
		FREE(data);
		return ;
	}

	for (int i = 0;i < (int)data->rdh.nCount;i++)
	{
		r[i].left = (int)((double)r[i].left * sx - iadj);
		r[i].top = (int)((double)r[i].top * sy - iadj);
		r[i].right = (int)((double)r[i].right * sx + adj);
		r[i].bottom = (int)((double)r[i].bottom * sy + adj);
	}

	OSREGIONHANDLE nhrgn = ExtCreateRegion(NULL, size, data);
	if (!nhrgn)
	{
		nhrgn = CreateRectRgn(0, 0, 0, 0);
	}
	FREE(data);
	DeleteObject(hrgn);
	hrgn = nhrgn;
	optimize();
}

void RegionI::debug(int async)
{
	if (!async)
	{
		SysCanvas c;
		RECT r;
		getBox(&r);
		//    c.fillRect(&r, 0);
		InvertRgn(c.getHDC(), getOSHandle());
		Sleep(200);
		InvertRgn(c.getHDC(), getOSHandle());
	}
	else
	{
		SysCanvas c;
		RECT r;
		getBox(&r);
		//    c.fillRect(&r, 0);
		if (lastdebug)
			InvertRgn(c.getHDC(), lastdebug->getOSHandle());
		delete lastdebug;
		lastdebug = new RegionI();
		lastdebug->addRegion(this);
		InvertRgn(c.getHDC(), getOSHandle());
	}
}

// later we can cache this data or something if needed
int RegionI::getNumRects()
{
	if (optimized) return 1;
	int bytes_needed = GetRegionData(hrgn, 0, NULL) + sizeof(RGNDATA);
	MemBlock<unsigned char> data(bytes_needed);
	GetRegionData(hrgn, bytes_needed, (LPRGNDATA)data.getMemory());
	RGNDATA *rgndata = reinterpret_cast<RGNDATA *>(data.getMemory());
	return rgndata->rdh.nCount;
}

int RegionI::enumRect(int n, RECT *r)
{
	if (optimized)
	{
		if (n == 0)
		{
			if (r != NULL) *r = optrect;
			return 1;
		}
		return 0;
	}
	if (n < 0) return 0;
	int bytes_needed = GetRegionData(hrgn, 0, NULL) + sizeof(RGNDATA);
	MemBlock<unsigned char> data(bytes_needed);
	GetRegionData(hrgn, bytes_needed, (LPRGNDATA)data.getMemory());
	RGNDATA *rgndata = reinterpret_cast<RGNDATA *>(data.getMemory());
	int nrects = rgndata->rdh.nCount;
	if (n >= nrects) return 0;
	RECT *rectlist = reinterpret_cast<RECT*>(rgndata->Buffer);
	*r = rectlist[n];
	return 1;
}

void RegionI::optimize()
{
	if (optimized) return ;
	if (srv != NULL) return ; // region is cached and shared, do not optimize
	CHECK_REGION
	getBox(&optrect);

	if (IsRectEmpty(&optrect))
		return;

	RECT br;
	OSREGIONHANDLE gr = CreateRectRgnIndirect(&optrect);
	OSREGIONHANDLE res = CreateRectRgn(0, 0, 0, 0);

/*
	// if they don't intersect, we may be offset
	IntersectRgn(res, gr, hrgn);
	GetRgnBox(res, &br);
	if (br.left == br.right || br.bottom == br.top)
	{
		DeleteObject(gr);
		DeleteObject(res);
		return ;
	}
	*/

	// if they intersect, but when subtracting the region from the rect, we get nothing, they're the same, let's optimize
	CombineRgn(res, gr, hrgn, RGN_DIFF);
	DeleteObject(gr);
	GetRgnBox(res, &br);
	DeleteObject(res);
	if (br.left == br.right || br.bottom == br.top)
	{
		optimized = 1;
		DeleteObject(hrgn);
		hrgn = NULL;
	}
}

void RegionI::deoptimize()
{
	if (!optimized) return ;
	CHECK_REGION
	SetRectRgn(hrgn, optrect.left, optrect.top, optrect.right, optrect.bottom);
	//if (hrgn != NULL) { DeleteObject(hrgn); hrgn = NULL; }
	//hrgn = CreateRectRgnIndirect(&optrect);
	//CHECK_REGION
	optimized = 0;
}


#define CBCLASS RegionServerI
START_DISPATCH;
VCB(REGIONSERVER_ADDREF, addRef);
VCB(REGIONSERVER_DELREF, delRef);
CB(REGIONSERVER_GETREGION, getRegion);
END_DISPATCH;

#endif//WIN32
