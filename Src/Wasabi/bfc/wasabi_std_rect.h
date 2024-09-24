#ifndef NULLSOFT_BFC_STD_RECT_H
#define NULLSOFT_BFC_STD_RECT_H

#include <bfc/platform/platform.h>
namespace Wasabi
{
	namespace Std
	{
		bool rectIntersect(const RECT& a, const RECT& b, RECT* intersection = NULL);
		bool pointInRect(const RECT& r, const POINT& p);
		void setRect(RECT* r, int left, int top, int right, int bottom);
		RECT makeRect(int left, int top, int right, int bottom);
		POINT makePoint(int x, int y);
		void setPoint(POINT* p, int x, int y);
		void offsetRect(RECT* r, int x, int y);
		bool rectEqual(const RECT& a, const RECT& b);
		bool rectEqual(const RECT* a, const RECT* b);
		void scaleRect(RECT* r, double scale);

	}
}
#endif