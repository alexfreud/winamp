#ifndef NULLSOFT_WASABI_TEXTBASE_H
#define NULLSOFT_WASABI_TEXTBASE_H

#include <api/wnd/wndclass/bufferpaintwnd.h>
#include <tataki/color/filteredcolor.h>
#include <tataki/color/skinclr.h>
#include <bfc/string/StringW.h>

#define TEXTBASE_PARENT BufferPaintWnd

class TextBase : public TEXTBASE_PARENT, public SkinCallbackI
{
public:
	TextBase();
	virtual ~TextBase();

	ARGB32 GetColor(int alt=0);
	void GetFontInfo(Wasabi::FontInfo *_font, int alt=0);

	void SetTextColor(ARGB32 c, int alt=0);
	void SetFontSize(const wchar_t *strvalue, int alt=0);
	void SetFont(const wchar_t *name, int alt=0);
	void SetAntialias(int a, int alt=0);
	void SetFontAlign(int al);
	virtual void setDblClickParam(const wchar_t *p);
	virtual const wchar_t *getDblClickParam();
	virtual void setRClickParam(const wchar_t *p);
	virtual const wchar_t *getRClickParam();
protected:
	/* Virtual methods to override */
	virtual void invalidateTextBuffer()=0;
	
	
/*static */void CreateXMLParameters(int master_handle);
private:
	StringW dblClickAction;
	StringW rClickAction;

	/* Font Info */
	FilteredColor color[2];
	SkinColor scolor[2];
	int color_mode[2];
	StringW font[2];
	int bold[2];
	int italic[2];
	int antialias[2];
	int align;

protected:
	int fontsize[2];
	int lpadding, rpadding;
	int grab;

private:
	/* XML Parameters */
	enum
	{
		TEXTBASE_SETCOLOR,
		TEXTBASE_SETALTCOLOR,
		TEXTBASE_SETFONTSIZE,
    TEXTBASE_SETFONT,
		TEXTBASE_SETALTFONT,
		    TEXTBASE_SETALTFONTSIZE,
				    TEXTBASE_SETBOLD,
    TEXTBASE_SETITALIC,
    TEXTBASE_SETALTBOLD,
    TEXTBASE_SETALTITALIC,
		TEXTBASE_SETANTIALIAS,
    TEXTBASE_SETALTANTIALIAS,
		TEXTBASE_SETDBLCLKACTION,
		TEXTBASE_SETRCLKACTION,
    TEXTBASE_SETALIGN,
		    TEXTBASE_SETLPADDING,
    TEXTBASE_SETRPADDING,
	TEXTBASE_RCLICKPARAM,
	TEXTBASE_DBLCLICKPARAM,


	};
	static XMLParamPair params[];
	int xuihandle;
	StringW dblclickparam;
	StringW rclickparam;
protected:
	/* Methods that TextBase overrides */
	int setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval);
	int onInit();
	int skincb_onColorThemeChanged(const wchar_t *newcolortheme);
	int onLeftButtonDblClk(int x, int y);
	int onRightButtonDown(int x, int y);
	virtual int wantAutoContextMenu() { return rClickAction.isempty(); }

};

#endif