#include "DialogSkinner.h"

DialogSkinner dialogSkinner;

COLORREF GetHTMLColor(int color)
{
	return ( ( color >> 16 ) & 0xff | ( color & 0xff00 ) | ( ( color << 16 ) & 0xff0000 ) );
}