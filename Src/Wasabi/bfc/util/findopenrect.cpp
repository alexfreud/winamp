#include <precomp.h>
#include "findopenrect.h"

#include <bfc/pair.h>

//CUT #define MAXTRIALS 10000	// unless time runs out
#define MOUSEPENALTY 65536

FindOpenRect::FindOpenRect()
{
	timelimit = 100;
	iters = 0;
	xdiv = ydiv = 1;
	fn = compare_overlapArea;
	userdata = userdata2 = 0;
	bsfval = bsfdist = 0;
}

void FindOpenRect::setCompareRectsFn(compareRectsFn _fn)
{
	fn = _fn;
}

RECT FindOpenRect::find(const RECT &vr, const PtrList<RECT> &list, const RECT &prev, unsigned long userdata, unsigned long userdata2)
{
	beginFind(vr, list, prev, userdata, userdata2);
	return findMore();
}

void FindOpenRect::beginFind(const RECT &viewport, const PtrList<RECT> &_list,
                             const RECT &_prev, unsigned long _userdata, unsigned long _userdata2)
{
	vr = viewport;
	list.copyFrom(&_list);
	prev = _prev;
	userdata = _userdata;
	userdata2 = _userdata2;

	// reset best rect found so far
	//CUT  bsf_nooverlap = prev;
	//CUT  found_nooverlap=0;
	//CUT  bsf_overlap = prev;
	//CUT  bsfval_nooverlap = 1e100;
	//CUT  bsfval_overlap = 1e100;
	bsfrect = prev;
	bsfval = 10e10;
	bsfdist = 10e10;

	//CUT  POINT mousepos;
	//CUT  Std::getMousePos(&mousepos);
}

RECT FindOpenRect::findMore()
{
	int w = prev.right - prev.left, h = prev.bottom - prev.top;
	int vw = vr.right - vr.left;
	int vh = vr.bottom - vr.top;

	if (vw <= w || vh <= h)
	{
		DebugStringW(L"findopenrect: window too big for viewport");
	}

	
	double started = Wasabi::Std::getTimeStampMS();

	for (int c = 0; /*CUTc < MAXTRIALS*/; c++)
	{

		// too bad = crashy crash :P love, BU
		if (c != 0 && (vw <= w || vh <= h))
			break;

		// set the trial rect
		RECT tr;
		if (iters == 0)
		{	// try prev
			tr = prev;
		}
		else
		{
			int x = Wasabi::Std::random32(vw - w) + vr.top;
			int y = Wasabi::Std::random32(vh - h) + vr.left;
			if (xdiv != 1) x -= x % xdiv;
			if (ydiv != 1) y -= y % ydiv;
			tr = Wasabi::Std::makeRect(x, y, x + w, y + h);
		}

		// add up the coverage of trial position
		trySingleRect(tr, TRUE);

		if (timelimit > 0 && c > 0 && (int)((Wasabi::Std::getTimeStampMS() - started)*1000.f) > timelimit)
		{
			//      DebugString("FindOpenRect::find() timeout, %d iters", c);
			break;
		}
	}

	return getBestRectSoFar();
}

typedef Pair<RECT, double> RectV;

namespace
{
	class Sorter
	{
	public:
		static int compareItem(RectV *p1, RectV* p2)
		{
			return -CMP3(p1->b, p2->b);
		}
	};
};

double FindOpenRect::trySingleRect(const RECT &tr, int early_out)
{
#if 0
	PtrListQuickSorted<RectV, Sorter> candidates(list.getNumItems());
	foreach(list)
	double d = compare_overlapArea(tr, *list.getfor());
	candidates.addItem(new RectV(*list.getfor(), d));
	endfor
	candidates.sort();
#else
	PtrList<RECT> candidates;
	candidates.copyFrom(&list);
#endif

	double trarea = 0;

	foreach(candidates)
#if 0
	RECT *wr = &candidates.getfor()->a;
#else
	RECT *wr = candidates.getfor();
#endif 
	//CUT    double bsf = MIN(found_nooverlap ? 0 : bsfval_nooverlap, bsfval_overlap);
	trarea += (*fn)(tr, *wr, userdata, userdata2, foreach_index, bsfval);

	// early quit if not breaking any records
	if (early_out && trarea >= bsfval) break;

	endfor

#if 0
	candidates.deleteAll();
#endif

	if (trarea < 0.005) trarea = 0;

#if 0
	// found one that does not overlap!
	if (trarea == 0)
	{
		if (val < bsfval_nooverlap)
		{ // if it's closer than prev closest, save it
			bsf_nooverlap = tr;
			bsfval_nooverlap = val;
			found_nooverlap = 1;
		}
	}
	else
	{
		if (trarea < bsfval_overlap)
		{	// overlaps least stuff
			bsf_overlap = tr;
			bsfval_overlap = trarea;
		}
	}
#endif
	if (trarea == 0)
	{
		// find the distance from the previous
		double val = SQRT(SQR(prev.left - tr.left) + SQR(prev.top - tr.top));
		if (bsfdist > val)
		{
			bsfrect = tr;
			bsfdist = val;
		}
	}
	else
	{
		if (trarea < bsfval)
		{
			bsfrect = tr;
			bsfval = trarea;
		}
	}
	iters++;
	return trarea;
}

double FindOpenRect::compare_overlapArea(const RECT &rect, const RECT &dest, unsigned long userdata, unsigned long userdata2, int index, double bsf)
{
	RECT intersection;
	if (Wasabi::Std::rectIntersect(rect, dest, &intersection))
	{
		intersection.right -= intersection.left;
		intersection.bottom -= intersection.top;
		return intersection.right * intersection.bottom;
	}
	return 0;
}

void FindOpenRect::setTimeLimit(int ms)
{
	timelimit = ms;
}

int FindOpenRect::getNumIters()
{
	return iters;
}

void FindOpenRect::resetNumIters()
{
	iters = 0;
}

double FindOpenRect::getBestValSoFar()
{
	return (bsfval == 10e10) ? 0 : bsfval;
}

RECT FindOpenRect::getBestRectSoFar()
{
	return bsfrect;
}

void FindOpenRect::setBestSoFar(double bsf, const RECT &bestrect)
{
	if (bsf > 0 && bsfval > bsf)
	{
		bsfval = bsf;
		bsfrect = bestrect;
		bsfdist = SQRT(SQR(prev.left - bsfrect.left) + SQR(prev.top - bsfrect.top));
		//CUTDebugString("*******************8 better BSF ****************");
	}
}

RECT FindOpenRect::getOriginalRect()
{
	return prev;
}

void FindOpenRect::setCoordDivisor(int _xdiv, int _ydiv)
{
	xdiv = MAX(_xdiv, 1);
	ydiv = MAX(_ydiv, 1);
}
