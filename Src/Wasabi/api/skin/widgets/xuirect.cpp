#include <precomp.h>
#include "xuirect.h"

#include <tataki/canvas/ifc_canvas.h>
#include <bfc/parse/paramparser.h>
#include <api/skin/skinfilter.h>
#include <api/wnd/PaintCanvas.h>

#define BLTSIZE 1


namespace RectEdges
{
enum { LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8 };
};
using namespace RectEdges;

XMLParamPair ScriptRect::params[] = {
                                      {SCRIPTRECT_SETCOLOR, L"COLOR"},
                                      {SCRIPTRECT_EDGES, L"EDGES"},
                                      {SCRIPTRECT_SETFILLED, L"FILLED"},
                                      {SCRIPTRECT_GAMMAGROUP, L"GAMMAGROUP"},
                                      {SCRIPTRECT_THICKNESS, L"THICKNESS"},
                                    };

ScriptRect::ScriptRect()
		: pixel(BLTSIZE, BLTSIZE, NULL)
{
	filled = 0;
	myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);
	color.setColorGroup(L"");

	resetPixel();
	edges = LEFT | RIGHT | TOP | BOTTOM;
	thickness = 1;
}

void ScriptRect::CreateXMLParameters(int master_handle)
{
	//SCRIPTRECT_PARENT::CreateXMLParameters(master_handle);
		int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		if (params[i].id == SCRIPTRECT_SETCOLOR)
			addParam(myxuihandle, params[i], XUI_ATTRIBUTE_REQUIRED);
		else
			addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

ScriptRect::~ScriptRect()
{}

int ScriptRect::onInit()
{
	SCRIPTRECT_PARENT::onInit();
	return 1;
}

int ScriptRect::onPaint(Canvas *c)
{
	if (c == NULL)
	{
		PaintCanvas pc;
		if (!pc.beginPaint(this)) return 0;
		return ScriptRect::onPaint(&pc);
	}

	// check for colors changing on us
	if (!color.iteratorValid()) resetPixel();

	//RECT src = {0, 0, BLTSIZE, BLTSIZE};
	if (filled)
	{
		RECT dst;
		getClientRect(&dst);
		c->fillRectAlpha(&dst, color.v(), getPaintingAlpha());
//		pixel./*getSkinBitmap()->*/stretchToRectAlpha(c, &src, &dst, getPaintingAlpha());
	}
	else
	{
		RECT dst, odst;
		getClientRect(&odst);
		if (edges & TOP)
		{
			dst = odst;
			dst.bottom = dst.top + thickness;
				c->fillRectAlpha(&dst, color.v(), getPaintingAlpha());
			//pixel./*getSkinBitmap()->*/stretchToRectAlpha(c, &src, &dst, getPaintingAlpha());
		}
		if (edges & BOTTOM)
		{
			dst = odst;
			dst.top = dst.bottom - thickness;
				c->fillRectAlpha(&dst, color.v(), getPaintingAlpha());
			//pixel./*getSkinBitmap()->*/stretchToRectAlpha(c, &src, &dst, getPaintingAlpha());
		}
		if (edges & RIGHT)
		{
			dst = odst;
			dst.top++; dst.bottom--;
			dst.left = dst.right - thickness;
				c->fillRectAlpha(&dst, color.v(), getPaintingAlpha());
			//pixel./*getSkinBitmap()->*/stretchToRectAlpha(c, &src, &dst, getPaintingAlpha());
		}
		if (edges & LEFT)
		{
			dst = odst;
			dst.right = dst.left + thickness;
				c->fillRectAlpha(&dst, color.v(), getPaintingAlpha());
			//pixel./*getSkinBitmap()->*/stretchToRectAlpha(c, &src, &dst, getPaintingAlpha());

		}
	}

	return 1;
}

int ScriptRect::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value)
{
	if (xuihandle != myxuihandle)
		return SCRIPTRECT_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

	switch (xmlattributeid)
	{
		case SCRIPTRECT_SETCOLOR:
		{
			ARGB32 prev_color = color;
			color.setElementName(value);
			//CUT      color = WASABI_API_SKIN->skin_getColorElement((char*)value);
			if (color.v() != prev_color)
			{
				//CUT?        ApplySkinFilters::apply(NULL, getXmlParamByName("gammagroup"), &color, BLTSIZE, BLTSIZE);
				resetPixel();
				invalidate();
			}
		}
		break;
		case SCRIPTRECT_GAMMAGROUP:
		{
			ARGB32 prev_color = color;
			color.setColorGroup(value);
			if (color.v() != prev_color)
			{
				resetPixel();
				invalidate();
			}
		}
		break;
		case SCRIPTRECT_SETFILLED:
		{
			int was_filled = filled;
			filled = WTOI(value);
			if (was_filled != filled) invalidate();
		}
		break;
		case SCRIPTRECT_EDGES:
		{
			int prev_edges = edges;
			ParamParser pp((const wchar_t *)value);
			edges = 0;
			edges |= !!pp.hasString(L"left") * LEFT;
			edges |= !!pp.hasString(L"right") * RIGHT;
			edges |= !!pp.hasString(L"top") * TOP;
			edges |= !!pp.hasString(L"bottom") * BOTTOM;
			if (edges != prev_edges) invalidate();
		}
		break;
		case SCRIPTRECT_THICKNESS:
		{
			int prev_thickness = thickness;
			thickness = WTOI(value);
			if (thickness < 1) thickness = 1;
			if (thickness != prev_thickness) invalidate();
		}
		break;
		default:
			return 0;
	}
	return 1;
}

void ScriptRect::resetPixel()
{
	pixel.fillBits(0xFF000000 | RGBTOBGR(color.v()));
}
