#include "main.h"
#include "./widgetStyle.h"

static void
WidgetStyle_DeleteGdiObject(UINT flags, UINT flagsMask, HGDIOBJ object)
{
	if (NULL != object && 0 != (flagsMask & flags))
		DeleteObject(object);
}


static BOOL
WidgetStyle_SetBrushInt(HBRUSH *brush, WidgetStyleFlags *flags, WidgetStyleFlags flagsMask,
						HBRUSH sourceBrush, WidgetStyleAssignFlags assignFlags)
{
	LOGBRUSH lb;

	WidgetStyle_DeleteGdiObject(*flags, flagsMask, *brush);
		
	if (0 == (WIDGETSTYLE_COPY_OBJECT & assignFlags))
	{
		if (0 == (WIDGETSTYLE_OWN_OBJECT & assignFlags))
			*flags &= ~flagsMask;
		else
			*flags |= flagsMask;

		*brush = sourceBrush;
		return TRUE;
	}
	
	
	*brush = (sizeof(lb) == GetObjectW(sourceBrush, sizeof(lb), &lb)) ?
				CreateBrushIndirect(&lb) : NULL;

	if (NULL == *brush)
	{	
		*flags &= ~flagsMask;
		return FALSE;
	}
			
	*flags |= flagsMask;
	return TRUE;
}

static BOOL
WidgetStyle_SetFontInt(HFONT *font, WidgetStyleFlags *flags, WidgetStyleFlags flagsMask, 
					   HFONT sourceFont, WidgetStyleAssignFlags assignFlags)
{
	LOGFONTW lf = {0};
	WidgetStyle_DeleteGdiObject(*flags, flagsMask, *font);

	if (0 == (WIDGETSTYLE_COPY_OBJECT & assignFlags))
	{
		if (0 == (WIDGETSTYLE_OWN_OBJECT & assignFlags))
			*flags &= ~flagsMask;
		else
			*flags |= flagsMask;

		*font = sourceFont;
		return TRUE;
	}

	*font = (sizeof(lf) == GetObjectW(sourceFont, sizeof(lf), &lf)) ? CreateFontIndirectW(&lf) : NULL;

	if (NULL == *font)
	{	
		*flags &= ~flagsMask;
		return FALSE;
	}

	*flags |= flagsMask;
	return TRUE;
}

void
WidgetStyle_Free(WidgetStyle *self)
{
	if (NULL == self)
		return;

	WidgetStyle_DeleteGdiObject(self->flags, WIDGETSTYLE_OWN_TEXT_FONT, self->textFont);
	WidgetStyle_DeleteGdiObject(self->flags, WIDGETSTYLE_OWN_TITLE_FONT, self->titleFont);
	WidgetStyle_DeleteGdiObject(self->flags, WIDGETSTYLE_OWN_CATEGORY_FONT, self->categoryFont);
	WidgetStyle_DeleteGdiObject(self->flags, WIDGETSTYLE_OWN_BACK_BRUSH, self->backBrush);
	WidgetStyle_DeleteGdiObject(self->flags, WIDGETSTYLE_OWN_CATEGORY_BRUSH, self->categoryBrush);
}

BOOL
WidgetStyle_SetBackBrush(WidgetStyle *self, HBRUSH brush, WidgetStyleAssignFlags flags)
{
	return WidgetStyle_SetBrushInt(&self->backBrush, &self->flags, 
									WIDGETSTYLE_OWN_BACK_BRUSH, brush, flags);
}

BOOL
WidgetStyle_SetCategoryBrush(WidgetStyle *self, HBRUSH brush, WidgetStyleAssignFlags flags)
{
	return WidgetStyle_SetBrushInt(&self->categoryBrush, &self->flags, 
									WIDGETSTYLE_OWN_CATEGORY_BRUSH, brush, flags);
}

BOOL
WidgetStyle_SetTextFont(WidgetStyle *self, HFONT font, WidgetStyleAssignFlags flags)
{
	return WidgetStyle_SetFontInt(&self->textFont, &self->flags,
									WIDGETSTYLE_OWN_TEXT_FONT, font, flags);

}

BOOL
WidgetStyle_SetTitleFont(WidgetStyle *self, HFONT font, WidgetStyleAssignFlags flags)
{
	return WidgetStyle_SetFontInt(&self->titleFont, &self->flags,
									WIDGETSTYLE_OWN_TITLE_FONT, font, flags);

}

BOOL
WidgetStyle_SetCategoryFont(WidgetStyle *self, HFONT font, WidgetStyleAssignFlags flags)
{
	return WidgetStyle_SetFontInt(&self->categoryFont, &self->flags,
									WIDGETSTYLE_OWN_CATEGORY_FONT, font, flags);

}

static COLORREF
WidgetStyle_GetCategoryLineColor(COLORREF categoryBackColor)
{
	COLORREF categoryLineColor;
	size_t index;

	const int categoryLineColors[] = 
	{			
		WADLG_LISTHEADER_FRAME_MIDDLECOLOR,
		WADLG_LISTHEADER_FRAME_BOTTOMCOLOR,
		WADLG_LISTHEADER_FRAME_TOPCOLOR,
		WADLG_HILITE,
	};
	
	for (index = 0; index < ARRAYSIZE(categoryLineColors); index++)
	{		
		categoryLineColor = Graphics_GetSkinColor(categoryLineColors[index]);
		int distance = Graphics_GetColorDistance(categoryLineColor, categoryBackColor);
		if (distance < 0) distance = -distance; 
		if (distance >= 40)	break;
	}

	return categoryLineColor;
}

static COLORREF
WidgetStyle_GetBorderColor(COLORREF backColor, COLORREF textColor)
{
	COLORREF borderColor;

	for(int step = 0;; step++)
	{
		switch(step)
		{
			case 0:
				borderColor = Graphics_GetSkinColor(WADLG_HILITE);
				break;
			case 1:
				borderColor = Graphics_BlendColors(Graphics_GetSkinColor(WADLG_SELBAR_FGCOLOR), Graphics_GetSkinColor(WADLG_SELBAR_BGCOLOR), 17);
				borderColor = Graphics_BlendColors(borderColor, backColor, 229);
				break;
			default:
				return textColor;
		}

		int distance = Graphics_GetColorDistance(borderColor, backColor);
		if (distance < 0) distance = -distance;
		if (distance >= 40)
			break;
	}

	return borderColor;
}

BOOL
WidgetStyle_UpdateDefaultColors(WidgetStyle *style)
{
	#define WIDGETSTYLE_SET_COLOR(_colorName, _color)\
		{COLORREF _colorCopy = (_color);\
		if (WIDGETSTYLE_##_colorName##_COLOR(style) != _colorCopy)\
		{WIDGETSTYLE_SET_##_colorName##_COLOR(style, _colorCopy);\
		styleChanged = TRUE;}}

	#define WIDGETSTYLE_SET_COLOR_BLEND(_colorName, _colorTop, _colorBottom, _alpha)\
		WIDGETSTYLE_SET_COLOR(_colorName, Graphics_BlendColors((_colorTop), (_colorBottom), (_alpha)))
	

	COLORREF widgetBackColor, widgetTextColor, categoryBackColor, categoryTextColor;
	BOOL styleChanged;
	HBRUSH brush;

	if (NULL == style)
		return FALSE;

	styleChanged = FALSE;
	
	widgetBackColor = Graphics_GetSkinColor(WADLG_ITEMBG);
	widgetTextColor = Graphics_GetSkinColor(WADLG_ITEMFG);
	categoryBackColor = Graphics_GetSkinColor(WADLG_LISTHEADER_BGCOLOR);
	categoryTextColor = Graphics_GetSkinColor(WADLG_LISTHEADER_FONTCOLOR);

	if (WIDGETSTYLE_BACK_COLOR(style) != widgetBackColor ||
		NULL == WIDGETSTYLE_BACK_BRUSH(style))
	{
		brush = CreateSolidBrush(widgetBackColor);	
		WIDGETSTYLE_SET_BACK_BRUSH(style, brush, WIDGETSTYLE_OWN_OBJECT);
		styleChanged = TRUE;
	}

	if (WIDGETSTYLE_CATEGORY_BACK_COLOR(style) != categoryBackColor ||
		NULL == WIDGETSTYLE_CATEGORY_BRUSH(style))
	{
		brush = CreateSolidBrush(categoryBackColor);	
		WIDGETSTYLE_SET_CATEGORY_BRUSH(style, brush, WIDGETSTYLE_OWN_OBJECT);
		styleChanged = TRUE;
	}

	WIDGETSTYLE_SET_COLOR(BACK, widgetBackColor);
	WIDGETSTYLE_SET_COLOR(TEXT, widgetTextColor);
	WIDGETSTYLE_SET_COLOR_BLEND(TITLE, widgetTextColor, widgetBackColor, 210);
	WIDGETSTYLE_SET_COLOR(BORDER, WidgetStyle_GetBorderColor(widgetBackColor, widgetTextColor));
	WIDGETSTYLE_SET_COLOR(IMAGE_BACK, widgetBackColor);
	WIDGETSTYLE_SET_COLOR(IMAGE_FRONT, widgetTextColor); 
	WIDGETSTYLE_SET_COLOR(SELECT_BACK, Graphics_GetSkinColor(WADLG_SELBAR_BGCOLOR));
	WIDGETSTYLE_SET_COLOR(SELECT_FRONT, Graphics_GetSkinColor(WADLG_SELBAR_FGCOLOR));
	WIDGETSTYLE_SET_COLOR_BLEND(INACTIVE_SELECT_BACK, WIDGETSTYLE_SELECT_BACK_COLOR(style), widgetBackColor, 192);
	WIDGETSTYLE_SET_COLOR_BLEND(INACTIVE_SELECT_FRONT, WIDGETSTYLE_SELECT_FRONT_COLOR(style), widgetBackColor, 192);
	WIDGETSTYLE_SET_COLOR(CATEGORY_BACK, categoryBackColor);
	WIDGETSTYLE_SET_COLOR(CATEGORY_TEXT, categoryTextColor);
	WIDGETSTYLE_SET_COLOR(CATEGORY_LINE, WidgetStyle_GetCategoryLineColor(categoryBackColor));
	WIDGETSTYLE_SET_COLOR_BLEND(CATEGORY_EMPTY_TEXT, widgetTextColor, widgetBackColor, 210);
	WIDGETSTYLE_SET_COLOR_BLEND(TEXT_EDITOR_BORDER, widgetTextColor, widgetBackColor, 140);

	return styleChanged;
}

BOOL
WidgetStyle_UpdateDefaultFonts(WidgetStyle *style, HFONT baseFont, long unitWidth, long unitHeight)
{
	HFONT tempFont;

	if (NULL == style)
		return FALSE;

	tempFont = Graphics_DuplicateFont(baseFont, 3, FALSE, TRUE);
	WIDGETSTYLE_SET_TITLE_FONT(style, tempFont, WIDGETSTYLE_OWN_OBJECT);

	tempFont = Graphics_DuplicateFont(baseFont, 0, FALSE, TRUE);
	WIDGETSTYLE_SET_CATEGORY_FONT(style, tempFont, WIDGETSTYLE_OWN_OBJECT);

	WIDGETSTYLE_SET_TEXT_FONT(style, baseFont, WIDGETSTYLE_LINK_OBJECT);

	WIDGETSTYLE_SET_UNIT_SIZE(style, unitWidth, unitHeight);

	return TRUE;
}