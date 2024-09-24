#ifndef _NULLSOFT_WINAMP_ML_DEVICES_WIDGETSTYLE_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_WIDGETSTYLE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef struct WidgetStyle WidgetStyle;
typedef enum WidgetStyleFlags WidgetStyleFlags;
typedef enum WidgetStyleAssignFlags WidgetStyleAssignFlags;

enum WidgetStyleFlags
{
	WIDGETSTYLE_OWN_TEXT_FONT = (1 << 0),
	WIDGETSTYLE_OWN_TITLE_FONT = (1 << 1),
	WIDGETSTYLE_OWN_CATEGORY_FONT = (1 << 2),
	WIDGETSTYLE_OWN_BACK_BRUSH = (1 << 3),
	WIDGETSTYLE_OWN_CATEGORY_BRUSH = (1 << 4),
};
DEFINE_ENUM_FLAG_OPERATORS(WidgetStyleFlags);

enum WidgetStyleAssignFlags
{
	WIDGETSTYLE_LINK_OBJECT = 0,
	WIDGETSTYLE_COPY_OBJECT = (1 << 0),
	WIDGETSTYLE_OWN_OBJECT = (1 << 1),
};
DEFINE_ENUM_FLAG_OPERATORS(WidgetStyleAssignFlags);


struct WidgetStyle
{
	WidgetStyleFlags flags;
	HFONT textFont;
	HFONT titleFont;
	HFONT categoryFont;
	HBRUSH backBrush;
	HBRUSH categoryBrush;
	COLORREF titleColor;
	COLORREF textColor;
	COLORREF backColor;
	COLORREF borderColor;
	COLORREF imageBackColor;
	COLORREF imageFrontColor;
	COLORREF selectBackColor;
	COLORREF selectFrontColor;
	COLORREF inactiveSelectBackColor;
	COLORREF inactiveSelectFrontColor;
	COLORREF categoryTextColor;
	COLORREF categoryLineColor;
	COLORREF categoryBackColor;
	COLORREF categoryEmptyTextColor;
	COLORREF textEditorBorderColor;
	SIZE unitSize;
};

#define DLU_TO_PX_VALIDATE_MIN(_value, _dlu, _min)\
	{if (0 != (_dlu) && ((_value) < (_min))) (_value) = (_min);}

#define WIDGETSTYLE_DLU_TO_HORZ_PX(_style, _dlu) MulDiv((_dlu), ((WidgetStyle*)(_style))->unitSize.cx, 4)
#define WIDGETSTYLE_DLU_TO_VERT_PX(_style, _dlu) MulDiv((_dlu), ((WidgetStyle*)(_style))->unitSize.cy, 8)

#define WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(_value, _style, _dlu, _min)\
	{_value = WIDGETSTYLE_DLU_TO_HORZ_PX(_style, _dlu); DLU_TO_PX_VALIDATE_MIN(_value, _dlu, _min);}

#define WIDGETSTYLE_DLU_TO_VERT_PX_MIN(_value, _style, _dlu, _min)\
	{_value = WIDGETSTYLE_DLU_TO_VERT_PX(_style, _dlu); DLU_TO_PX_VALIDATE_MIN(_value, _dlu, _min);}

#define WIDGETSTYLE_TITLE_FONT(_style) (((WidgetStyle*)(_style))->titleFont)
#define WIDGETSTYLE_TEXT_FONT(_style) (((WidgetStyle*)(_style))->textFont)
#define WIDGETSTYLE_CATEGORY_FONT(_style) (((WidgetStyle*)(_style))->categoryFont)
#define WIDGETSTYLE_BACK_BRUSH(_style) (((WidgetStyle*)(_style))->backBrush)
#define WIDGETSTYLE_CATEGORY_BRUSH(_style) (((WidgetStyle*)(_style))->categoryBrush)
#define WIDGETSTYLE_TITLE_COLOR(_style) (((WidgetStyle*)(_style))->titleColor)
#define WIDGETSTYLE_TEXT_COLOR(_style) (((WidgetStyle*)(_style))->textColor)
#define WIDGETSTYLE_BACK_COLOR(_style) (((WidgetStyle*)(_style))->backColor)
#define WIDGETSTYLE_BORDER_COLOR(_style) (((WidgetStyle*)(_style))->borderColor)
#define WIDGETSTYLE_IMAGE_BACK_COLOR(_style) (((WidgetStyle*)(_style))->imageBackColor)
#define WIDGETSTYLE_IMAGE_FRONT_COLOR(_style) (((WidgetStyle*)(_style))->imageFrontColor)
#define WIDGETSTYLE_SELECT_BACK_COLOR(_style) (((WidgetStyle*)(_style))->selectBackColor)
#define WIDGETSTYLE_SELECT_FRONT_COLOR(_style) (((WidgetStyle*)(_style))->selectFrontColor)
#define WIDGETSTYLE_INACTIVE_SELECT_BACK_COLOR(_style) (((WidgetStyle*)(_style))->inactiveSelectBackColor)
#define WIDGETSTYLE_INACTIVE_SELECT_FRONT_COLOR(_style) (((WidgetStyle*)(_style))->inactiveSelectFrontColor)
#define WIDGETSTYLE_CATEGORY_TEXT_COLOR(_style) (((WidgetStyle*)(_style))->categoryTextColor)
#define WIDGETSTYLE_CATEGORY_BACK_COLOR(_style) (((WidgetStyle*)(_style))->categoryBackColor)
#define WIDGETSTYLE_CATEGORY_LINE_COLOR(_style) (((WidgetStyle*)(_style))->categoryLineColor)
#define WIDGETSTYLE_CATEGORY_EMPTY_TEXT_COLOR(_style) (((WidgetStyle*)(_style))->categoryEmptyTextColor)
#define WIDGETSTYLE_TEXT_EDITOR_BORDER_COLOR(_style) (((WidgetStyle*)(_style))->textEditorBorderColor)
#define WIDGETSTYLE_SET_UNIT_SIZE(_style, _width, _height)\
	{(((WidgetStyle*)(_style))->unitSize).cx = _width;\
	 (((WidgetStyle*)(_style))->unitSize).cy = _height;}

#define WIDGETSTYLE_SET_TITLE_FONT(_style, _font, _flags)\
	WidgetStyle_SetTitleFont(((WidgetStyle*)(_style)), (_font), (_flags))
#define WIDGETSTYLE_SET_TEXT_FONT(_style, _font, _flags)\
	WidgetStyle_SetTextFont(((WidgetStyle*)(_style)), (_font), (_flags))
#define WIDGETSTYLE_SET_CATEGORY_FONT(_style, _font, _flags)\
	WidgetStyle_SetCategoryFont(((WidgetStyle*)(_style)), (_font), (_flags))
#define WIDGETSTYLE_SET_BACK_BRUSH(_style, _brush, _flags)\
	WidgetStyle_SetBackBrush(((WidgetStyle*)(_style)), (_brush), (_flags))
#define WIDGETSTYLE_SET_CATEGORY_BRUSH(_style, _brush, _flags)\
	WidgetStyle_SetCategoryBrush(((WidgetStyle*)(_style)), (_brush), (_flags))

#define WIDGETSTYLE_SET_TITLE_COLOR(_style, _color) (((WidgetStyle*)(_style))->titleColor = (_color))
#define WIDGETSTYLE_SET_TEXT_COLOR(_style, _color) (((WidgetStyle*)(_style))->textColor = (_color))
#define WIDGETSTYLE_SET_BACK_COLOR(_style, _color) (((WidgetStyle*)(_style))->backColor = (_color))
#define WIDGETSTYLE_SET_BORDER_COLOR(_style, _color) (((WidgetStyle*)(_style))->borderColor = (_color))
#define WIDGETSTYLE_SET_IMAGE_BACK_COLOR(_style, _color) (((WidgetStyle*)(_style))->imageBackColor = (_color))
#define WIDGETSTYLE_SET_IMAGE_FRONT_COLOR(_style, _color) (((WidgetStyle*)(_style))->imageFrontColor = (_color))
#define WIDGETSTYLE_SET_SELECT_BACK_COLOR(_style, _color) (((WidgetStyle*)(_style))->selectBackColor = (_color))
#define WIDGETSTYLE_SET_SELECT_FRONT_COLOR(_style, _color) (((WidgetStyle*)(_style))->selectFrontColor = (_color))
#define WIDGETSTYLE_SET_INACTIVE_SELECT_BACK_COLOR(_style, _color) (((WidgetStyle*)(_style))->inactiveSelectBackColor = (_color))
#define WIDGETSTYLE_SET_INACTIVE_SELECT_FRONT_COLOR(_style, _color) (((WidgetStyle*)(_style))->inactiveSelectFrontColor = (_color))
#define WIDGETSTYLE_SET_CATEGORY_TEXT_COLOR(_style, _color) (((WidgetStyle*)(_style))->categoryTextColor = (_color))
#define WIDGETSTYLE_SET_CATEGORY_BACK_COLOR(_style, _color) (((WidgetStyle*)(_style))->categoryBackColor = (_color))
#define WIDGETSTYLE_SET_CATEGORY_LINE_COLOR(_style, _color) (((WidgetStyle*)(_style))->categoryLineColor = (_color))
#define WIDGETSTYLE_SET_CATEGORY_EMPTY_TEXT_COLOR(_style, _color) (((WidgetStyle*)(_style))->categoryEmptyTextColor = (_color))
#define WIDGETSTYLE_SET_TEXT_EDITOR_BORDER_COLOR(_style, _color) (((WidgetStyle*)(_style))->textEditorBorderColor = (_color))
void
WidgetStyle_Free(WidgetStyle *self);

BOOL
WidgetStyle_SetBackBrush(WidgetStyle *self, 
						 HBRUSH brush, 
						 WidgetStyleAssignFlags flags);

BOOL
WidgetStyle_SetCategoryBrush(WidgetStyle *self, 
						 HBRUSH brush, 
						 WidgetStyleAssignFlags flags);

BOOL
WidgetStyle_SetTextFont(WidgetStyle *self, 
						HFONT font, 
						WidgetStyleAssignFlags flags);

BOOL
WidgetStyle_SetTitleFont(WidgetStyle *self, 
						 HFONT font, 
						 WidgetStyleAssignFlags flags);

BOOL
WidgetStyle_SetCategoryFont(WidgetStyle *self, 
						 HFONT font, 
						 WidgetStyleAssignFlags flags);

BOOL
WidgetStyle_UpdateDefaultColors(WidgetStyle *style);

BOOL
WidgetStyle_UpdateDefaultFonts(WidgetStyle *style, 
							  HFONT baseFont, 
							  long unitWidth, 
							  long unitHeight);


#endif //_NULLSOFT_WINAMP_ML_DEVICES_WIDGETSTYLE_HEADER
