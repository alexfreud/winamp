/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"

void FixMainWindowRect(RECT *r)
{
	if (r->right-r->left > 280)
	{
		if (r->bottom-r->top < 200)
			r->bottom=r->top+14*2;
	}
	else
	{
		if (r->bottom-r->top < 100)
			r->bottom=r->top+14;
	}
}

void EstMainWindowRect( RECT *r )
{
	r->left   = config_wx;
	r->top    = config_wy;
	r->right  = config_wx + ( WINDOW_WIDTH << ( config_dsize ? 1 : 0 ) );
	r->bottom = config_wy + ( ( config_windowshade ? 14 : WINDOW_HEIGHT ) << ( config_dsize ? 1 : 0 ) );
}

void EstEQWindowRect( RECT *r )
{
	r->left   = config_eq_wx;
	r->top    = config_eq_wy;
	r->right  = config_eq_wx + ( WINDOW_WIDTH << ( config_dsize && config_eqdsize ? 1 : 0 ) );
	r->bottom = config_eq_wy + ( ( config_eq_ws ? 14 : WINDOW_HEIGHT ) << ( config_dsize && config_eqdsize ? 1 : 0 ) );
}

void EstPLWindowRect( RECT *r )
{
	r->left   = config_pe_wx;
	r->top    = config_pe_wy;
	r->right  = config_pe_wx + config_pe_width;
	r->bottom = config_pe_wy + config_pe_height;
}

void EstVidWindowRect( RECT *r )
{
	r->left   = config_video_wx;
	r->top    = config_video_wy;
	r->right  = config_video_wx + config_video_width;
	r->bottom = config_video_wy + config_video_height;
}

void SetMainWindowRect(RECT *r)
{
	config_wx=r->left;
	config_wy=r->top;
}

void SetEQWindowRect(RECT *r)
{
	config_eq_wx=r->left;
	config_eq_wy=r->top;
}

void SetPLWindowRect(RECT *r)
{
	config_pe_wx=r->left;
	config_pe_wy=r->top;
}

void SetVidWindowRect(RECT *r)
{
	config_video_wx=r->left;
	config_video_wy=r->top;
}

void MoveRect(RECT *r, int x, int y)
{
	r->left+=x;
	r->right+=x;
	r->top+=y;
	r->bottom+=y;
}

int IsWindowAttached(RECT rc, RECT rc2)
{
#define INREG(x,l,h) ((x) >= (l) && (x) <= (h))
  int r=0;
	if (rc2.right == rc.left || rc2.left == rc.right)
	{
	    if (INREG(rc.top,rc2.top,rc2.bottom) || INREG(rc.bottom,rc2.top,rc2.bottom) ||
	        INREG(rc2.top,rc.top,rc.bottom) || INREG(rc2.bottom,rc.top,rc.bottom))
			r|=1;
	}
	if (rc2.bottom == rc.top || rc2.top == rc.bottom) 
	{
		if (INREG(rc2.left,rc.left,rc.right) || INREG(rc2.right,rc.left,rc.right) ||
			INREG(rc.left,rc2.left,rc2.right) || INREG(rc.right,rc2.left,rc2.right))
			r|=2;
	}
#undef INREG
	return r;
}

void SnapWindowToWindow(RECT *rcSrc, RECT rcDest)
{

#define INREG(x,l,h) ((x) >= (l) && (x) <= (h))
#define IRR(l1,r1,l2,r2) (INREG(l1,l2,r2)||INREG(r1,l2,r2)||INREG(l2,l1,r1)||INREG(r2,l1,r1))
#define CLOSETO(x,t) INREG(x,t-config_snaplen,t+config_snaplen)
	if (IRR(rcDest.left,rcDest.right,rcSrc->left,rcSrc->right))
	{
		if (CLOSETO(rcSrc->top,rcDest.bottom))
		{
			rcSrc->bottom+=rcDest.bottom-rcSrc->top;
			rcSrc->top=rcDest.bottom;
		}
		else if (CLOSETO(rcSrc->bottom,rcDest.top))
		{
			rcSrc->top=rcDest.top-(rcSrc->bottom-rcSrc->top);
			rcSrc->bottom=rcDest.top;
		}
	}

	if (IRR(rcDest.top,rcDest.bottom,rcSrc->top,rcSrc->bottom))
	{
		if (CLOSETO(rcSrc->right,rcDest.left))
		{
			rcSrc->left = rcDest.left-(rcSrc->right-rcSrc->left);
			rcSrc->right= rcDest.left;
		}
		else if (CLOSETO(rcSrc->left,rcDest.right))
		{
			rcSrc->right += (rcDest.right-rcSrc->left);
			rcSrc->left=rcDest.right;
		}
	}

	if (rcSrc->right == rcDest.left || rcSrc->left== rcDest.right)
	{
		if (CLOSETO(rcSrc->top,rcDest.top))
		{
			rcSrc->bottom += rcDest.top-rcSrc->top;
			rcSrc->top = rcDest.top;
		}
		else if (CLOSETO(rcSrc->bottom,rcDest.bottom))
		{
			rcSrc->top += rcDest.bottom-rcSrc->bottom;
			rcSrc->bottom=rcDest.bottom;
		}
	}

	if (rcSrc->bottom == rcDest.top || rcSrc->top == rcDest.bottom)
	{
		if (CLOSETO(rcSrc->left,rcDest.left))
		{
			rcSrc->right += rcDest.left-rcSrc->left;
			rcSrc->left = rcDest.left;
		}
		else if (CLOSETO(rcSrc->right,rcDest.right))
		{
			rcSrc->left += rcDest.right-rcSrc->right;
			rcSrc->right = rcDest.right;
		}
	}
#undef INREG
#undef IRR
#undef CLOSETO
}

void AdjustSnap(RECT old1, RECT old2, RECT *new1, RECT *new2)
{
#define INREG(x,l,h) ((x) >= (l) && (x) < (h))
	if (INREG(old1.top,old2.top,old2.bottom) || INREG(old2.top,old1.top,old1.bottom)) {
#undef INREG
		// xpos
		if (old1.right >= old2.left && old1.left < old2.right) // old1/old2
		{
			MoveRect(new1,(new2->left-(new1->right-new1->left)) - new1->left,0);
		}
		else if (old2.right >= old1.left && old2.left < old1.right) // old2/old1 
		{
			MoveRect(new2,(new1->left-(new2->right-new2->left)) - new2->left,0);
		}
	}
#define INREG(x,l,h) ((x) >= (l) && (x) < (h))
	if (INREG(old1.left,old2.left,old2.right) || INREG(old2.left,old1.left,old1.right)) {
#undef INREG
		// ypos
		if (old1.bottom >= old2.top && old1.top < old2.bottom) // old1/old2
		{
			MoveRect(new1,0,(new2->top-(new1->bottom-new1->top)) - new1->top);
		}
		else if (old2.bottom >= old1.top && old2.top < old1.bottom) // old2/old1 
		{
			MoveRect(new2,0,(new1->top-(new2->bottom-new2->top)) - new2->top);
		}
	}
}

int IsPointInRect(int x, int y, RECT *r)
{
	if (x >= r->left && x < r->right && y >= r->top && y < r->bottom)
		return 1;
	return 0;
}

void FixOverlaps(RECT *r1, RECT *r2)
{
	if (r1->left >= r2->left) // r1 - r2
	{
		RECT *t=r1;
		r1=r2;
		r2=t;
	}
	{
		if (IsPointInRect(r2->left,r2->top,r1))
		{
			if (r1->right-r2->left < r1->bottom-r2->top)
			{
				MoveRect(r2,r1->right-r2->left,0);
			}
			else
			{
				MoveRect(r2,0,r1->bottom-r2->top);
			}
		}
	}
}