#include <precomp.h>
#include <api.h>
#include "textbase.h"
#include <api/skin/skinparse.h>
#include <api/skin/skinelem.h>
#include <api/service/svcs/svc_action.h>
#include <api/script/vcpu.h>

#define COLORMODE_RGB 0
#define COLORMODE_SKINCOLOR 1

XMLParamPair TextBase::params[] =
{
	{TEXTBASE_RCLICKPARAM, L"RCLICKPARAM"},
	{TEXTBASE_DBLCLICKPARAM, L"DBLCLICKPARAM"},
	{TEXTBASE_SETALIGN, L"ALIGN"},
	{TEXTBASE_SETALTANTIALIAS, L"ALTANTIALIAS"},
	{TEXTBASE_SETALTBOLD, L"ALTBOLD"},
	{TEXTBASE_SETALTCOLOR, L"ALTCOLOR"},
	{TEXTBASE_SETALTFONT, L"ALTFONT"},
	{TEXTBASE_SETALTFONTSIZE, L"ALTFONTSIZE"},
	{TEXTBASE_SETALTITALIC, L"ALTITALIC"},
	{TEXTBASE_SETANTIALIAS, L"ANTIALIAS"},
	{TEXTBASE_SETBOLD, L"BOLD"},
	{TEXTBASE_SETCOLOR, L"COLOR"},
	{TEXTBASE_SETDBLCLKACTION, L"DBLCLICKACTION"},
	{TEXTBASE_SETFONT, L"FONT"},
	{TEXTBASE_SETFONTSIZE, L"FONTSIZE"},
	{TEXTBASE_SETITALIC, L"ITALIC"},
	{TEXTBASE_SETLPADDING, L"LEFTPADDING"},
	{TEXTBASE_SETRCLKACTION, L"RIGHTCLICKACTION"},
	{TEXTBASE_SETRPADDING, L"RIGHTPADDING"},
};

TextBase::TextBase()
{
	color[0].setColorGroup(L"Text");
	color[1].setColorGroup(L"Text");
	color[0].setColor(RGB(255, 255, 255));
	color[1].setColor(RGB(255, 255, 255));

	color_mode[0] = COLORMODE_RGB;
	color_mode[1] = COLORMODE_RGB;

	fontsize[0] = fontsize[1] = 14;

	font[0] = wasabi_default_fontnameW;
	font[1] = wasabi_default_fontnameW;

	bold[0] = 0;
	bold[1] = 0;

	italic[0] = 0;
	italic[1] = 0;

	antialias[0] = 1;
	antialias[1] = 1;

	align = ALIGN_LEFT;

	lpadding = rpadding = 0;

	grab = 0;

	/* register XML parameters */
	xuihandle = newXuiHandle();
	CreateXMLParameters(xuihandle);	
}

void TextBase::CreateXMLParameters(int master_handle)
{
	//TEXTBASE_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(xuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(xuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

TextBase::~TextBase()
{
	WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SkinCallbackI*>(this));
}

int TextBase::onInit()
{
	TEXTBASE_PARENT::onInit();
	WASABI_API_SYSCB->syscb_registerCallback(static_cast<SkinCallbackI*>(this));
	return 1;
}

int TextBase::skincb_onColorThemeChanged(const wchar_t *newcolortheme)
{
	invalidateTextBuffer();
	return 0;
}


ARGB32 TextBase::GetColor(int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	if (color_mode[alt] == COLORMODE_SKINCOLOR)
		return scolor[alt];
	return color[alt].getColor();
}

void TextBase::SetTextColor(ARGB32 c, int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	color_mode[alt] = COLORMODE_RGB;
	color[alt].setColor(c);
	invalidateTextBuffer();
	if (alt == 0) SetTextColor(c, 1);
}

void TextBase::SetFontSize(const wchar_t *strvalue, int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	if (!strvalue || !*strvalue) return ;
	if (*strvalue == '+')
		fontsize[alt] += WTOI(strvalue + 1);
	else if (*strvalue == '-')
		fontsize[alt] -= WTOI(strvalue + 1);
	else if (*strvalue == '*')
		fontsize[alt] *= WTOI(strvalue + 1);
	else if (*strvalue == '/')
		fontsize[alt] /= WTOI(strvalue + 1);
	else
		fontsize[alt] = WTOI(strvalue);
	invalidateTextBuffer();
	if (alt == 0) SetFontSize(strvalue, 1);
}

void TextBase::GetFontInfo(Wasabi::FontInfo *_font, int alt)
{
	_font->opaque=false;
	_font->color = GetColor(alt);
	_font->pointSize = fontsize[alt];
	_font->face = font[alt];
	_font->bold = bold[alt];
	_font->italic = !!italic[alt];
	_font->antialias = antialias[alt];
	_font->alignFlags = align;
}

void TextBase::SetFont(const wchar_t *name, int alt)
{
	if (alt < 0 || alt > 1) alt = 0;
	font[alt] = name;
	invalidateTextBuffer();
	if (alt == 0) SetFont(name, 1);
}

void TextBase::SetAntialias(int a, int alt)
{
	antialias[alt] = a;
	if (alt==0)
		antialias[1] = antialias[0];
	invalidateTextBuffer();
}

void TextBase::SetFontAlign(int al)
{
	align = al;
	invalidateTextBuffer();

}

int TextBase::setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval)
{
	if (xuihandle != _xuihandle) return TEXTBASE_PARENT::setXuiParam(_xuihandle, attrid, name, strval);
	switch (attrid)
	{
	case TEXTBASE_SETCOLOR:
		if (WASABI_API_PALETTE->getColorElementRef(strval))
		{
			color_mode[0] = COLORMODE_SKINCOLOR;
			scolor[0] = strval;
			color_mode[1] = COLORMODE_SKINCOLOR;
			scolor[1] = strval;
		}
		else
			SetTextColor(SkinParser::parseColor(strval), 0);
		break;

	case TEXTBASE_SETALTCOLOR:
		if (WASABI_API_PALETTE->getColorElementRef(strval))
		{
			color_mode[1] = COLORMODE_SKINCOLOR;
			scolor[1] = strval;
		}
		else
			SetTextColor(SkinParser::parseColor(strval), 1);
		break;

	case TEXTBASE_SETFONTSIZE:
		SetFontSize(strval);
		break;

	case TEXTBASE_SETFONT:
		SetFont(strval);
		break;

	case TEXTBASE_SETALTFONT:
		SetFont(strval, 1);
		break;

	case TEXTBASE_SETALTFONTSIZE:
		SetFontSize(strval, 1);
		break;

	case TEXTBASE_SETBOLD:
		bold[0] = WTOI(strval);
		bold[1] = bold[0];
		break;

	case TEXTBASE_SETALTBOLD:
		bold[1] = WTOI(strval);
		break;

	case TEXTBASE_SETITALIC:
		italic[0] = WTOI(strval);
		italic[1] = italic[0];
		break;

	case TEXTBASE_SETALTITALIC:
		italic[1] = WTOI(strval);
		break;

	case TEXTBASE_SETANTIALIAS:
		SetAntialias(WTOI(strval));
		invalidateTextBuffer();
		break;

	case TEXTBASE_SETALTANTIALIAS:
		SetAntialias(WTOI(strval), 1);
		invalidateTextBuffer();
		break;

	case TEXTBASE_SETDBLCLKACTION:
		dblClickAction = strval;
		break;

	case TEXTBASE_DBLCLICKPARAM:
		setDblClickParam(strval);
		break;

	case TEXTBASE_RCLICKPARAM:
		setRClickParam(strval);
		break;

	case TEXTBASE_SETRCLKACTION:
		rClickAction = strval;
		break;

	case TEXTBASE_SETALIGN:
		SetFontAlign(SkinParser::getAlign(strval));
		break;

	case TEXTBASE_SETLPADDING:
		lpadding = WTOI(strval);
		break;
	case TEXTBASE_SETRPADDING:
		rpadding = WTOI(strval);
		break;


	default:
		return 0;
	}
	return 1;
}

void TextBase::setDblClickParam(const wchar_t *p) 
{
	dblclickparam=p;
}

const wchar_t *TextBase::getDblClickParam() 
{
	return dblclickparam;
}

void TextBase::setRClickParam(const wchar_t *p) 
{
	rclickparam=p;
}

const wchar_t *TextBase::getRClickParam() 
{
	return rclickparam;
}

int TextBase::onLeftButtonDblClk(int x, int y)
{
	TEXTBASE_PARENT::onLeftButtonDblClk(x, y);
	int r = 1;
	grab = 0;
	if (!dblClickAction.isempty() && !VCPU::getComplete())
	{
#ifdef WASABI_COMPILE_WNDMGR
		const wchar_t *toCheck = L"SWITCH;";
		if (!_wcsnicmp(dblClickAction, toCheck, 7))
		{
			onLeftButtonUp(x, y);
			getGuiObject()->guiobject_getParentGroup()->getParentContainer()->switchToLayout(dblClickAction.getValue() + 7);
		}
		else
		{
#endif
			svc_action *act = ActionEnum(dblClickAction).getNext();
			if (act)
			{
				int _x = x;
				int _y = y;
				clientToScreen(&_x, &_y);
				act->onAction(dblClickAction, getDblClickParam(), _x, _y, NULL, 0, this);
				SvcEnum::release(act);
			}
#ifdef WASABI_COMPILE_WNDMGR

		}
#endif

	}
	return r;
}


int TextBase::onRightButtonDown(int x, int y)
{
	TEXTBASE_PARENT::onRightButtonDown(x, y);
	if (!rClickAction.isempty())
	{
		svc_action *act = ActionEnum(rClickAction).getNext();
		if (act)
		{
			int _x = x;
			int _y = y;
			clientToScreen(&_x, &_y);
			act->onAction(rClickAction, getRClickParam(), _x, _y, NULL, 0, this);
			SvcEnum::release(act);
		}
	}
	return 1;
}