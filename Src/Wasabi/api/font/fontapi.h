#ifndef __FONTAPI_H
#define __FONTAPI_H

#include <api/font/api_font.h>

class FontApi : public api_font
{
public:
	FontApi();
	~FontApi();
	void font_textOut(ifc_canvas *c, int style, int x, int y, int w, int h, const wchar_t *txt);
	int font_getInfo(ifc_canvas *c, const wchar_t *font, int infoid, const wchar_t *txt, int *w, int *h);

protected:
	RECVS_DISPATCH;
};

extern api_font *fontApi;

#endif
