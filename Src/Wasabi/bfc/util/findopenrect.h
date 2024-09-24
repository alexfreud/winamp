#ifndef _FINDOPENRECT_H
#define _FINDOPENRECT_H

#include <bfc/wasabi_std.h>
#include <bfc/ptrlist.h>

class FindOpenRect
{
public:
	FindOpenRect();

	typedef double (*compareRectsFn)(const RECT &rect, const RECT &dest, unsigned long userdata, unsigned long userdata2, int index, double bsf);
	static double compare_overlapArea(const RECT &rect, const RECT &dest, unsigned long userdata = 0, unsigned long userdata2 = 0, int index = 0, double bsf = 0);

	void setCompareRectsFn(compareRectsFn fn);

	// this one does it all in one call
	RECT find(const RECT &viewport, const PtrList<RECT> &list,
	          const RECT &prev, unsigned long userdata = 0, unsigned long userdata2 = 0);

	// these let you do it over time
	void beginFind(const RECT &viewport, const PtrList<RECT> &list,
	               const RECT &prev, unsigned long userdata = 0, unsigned long userdata2 = 0);
	RECT findMore();

	double trySingleRect(const RECT &r, int early_out = FALSE);

	void setTimeLimit(int ms);
	int getNumIters();
	void resetNumIters();

	double getBestValSoFar();
	RECT getBestRectSoFar();

	// this only sets it if it's a better value
	void setBestSoFar(double bsf, const RECT &bestrect);

	RECT getOriginalRect();

	void setCoordDivisor(int xdiv, int ydiv);

private:
	RECT vr, prev;
	PtrList<RECT> list;
	compareRectsFn fn;
	unsigned long userdata, userdata2;

	int timelimit;
	int iters;

	int xdiv, ydiv;

	RECT bsfrect;
	double bsfval;
	double bsfdist;
	//CUT  RECT bsf_nooverlap;
	//CUT  int found_nooverlap;
	//CUT  RECT bsf_overlap;
	//CUT  double bsfval_nooverlap;
	//CUT  double bsfval_overlap;
};

#endif
