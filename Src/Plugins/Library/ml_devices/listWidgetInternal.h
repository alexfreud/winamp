#ifndef _NULLSOFT_WINAMP_ML_DEVICES_LIST_WIDGET_INTERNAL_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_LIST_WIDGET_INTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <vector>
#include "./imageCache.h"

typedef enum ListWidgetItemState
{
	ListWidgetItemState_Default = (0),
	ListWidgetItemState_Hovered = (1 << 0),
	ListWidgetItemState_Selected = (1 << 1),
	ListWidgetItemState_Interactive = (1 << 2),
	ListWidgetItemState_TextTruncated = (1 << 3),
	ListWidgetItemState_TextEdited = (1 << 4),

} ListWidgetItemSate;
DEFINE_ENUM_FLAG_OPERATORS(ListWidgetItemSate);

#define ListWidgetItem_State(_item)					(((ListWidgetItem*)(_item))->state)
#define ListWidgetItem_SetState(_item, _state)		(ListWidgetItem_State(_item) |= (_state)) 
#define ListWidgetItem_UnsetState(_item, _state)	(ListWidgetItem_State(_item) &= ~(_state))

#define ListWidgetItem_IsHovered(_item)			(0 != (ListWidgetItemState_Hovered & ListWidgetItem_State(_item)))
#define ListWidgetItem_SetHovered(_item)		ListWidgetItem_SetState(_item, ListWidgetItemState_Hovered)
#define ListWidgetItem_UnsetHovered(_item)		ListWidgetItem_UnsetState(_item, ListWidgetItemState_Hovered)

#define ListWidgetItem_IsSelected(_item)		(0 != (ListWidgetItemState_Selected & ListWidgetItem_State(_item)))
#define ListWidgetItem_SetSelected(_item)		ListWidgetItem_SetState(_item, ListWidgetItemState_Selected)
#define ListWidgetItem_UnsetSelected(_item)		ListWidgetItem_UnsetState(_item, ListWidgetItemState_Selected)

#define ListWidgetItem_IsInteractive(_item)		(0 != (ListWidgetItemState_Interactive & ListWidgetItem_State(_item)))
#define ListWidgetItem_SetInteractive(_item)	ListWidgetItem_SetState(_item, ListWidgetItemState_Interactive)
#define ListWidgetItem_UnsetInteractive(_item)	ListWidgetItem_UnsetState(_item, ListWidgetItemState_Interactive)


#define ListWidgetItem_IsTextTruncated(_item)		(0 != (ListWidgetItemState_TextTruncated & ListWidgetItem_State(_item)))
#define ListWidgetItem_SetTextTruncated(_item)		ListWidgetItem_SetState(_item, ListWidgetItemState_TextTruncated)
#define ListWidgetItem_UnsetTextTruncated(_item)	ListWidgetItem_UnsetState(_item, ListWidgetItemState_TextTruncated)

#define ListWidgetItem_IsTextEdited(_item)		(0 != (ListWidgetItemState_TextEdited & ListWidgetItem_State(_item)))
#define ListWidgetItem_SetTextEdited(_item)		ListWidgetItem_SetState(_item, ListWidgetItemState_TextEdited)
#define ListWidgetItem_UnsetTextEdited(_item)	ListWidgetItem_UnsetState(_item, ListWidgetItemState_TextEdited)


typedef struct ListWidgetConnection ListWidgetConnection;
typedef std::vector<ListWidgetConnection*> ListWidgetConnectionList;

typedef struct ListWidgetTooltip ListWidgetTooltip;

typedef enum ListWidgetCommandState
{	
	ListWidgetCommandState_Normal = (0),
	ListWidgetCommandState_Disabled = (1 << 0),
	ListWidgetCommandState_Primary = (1 << 1),
	ListWidgetCommandState_Pressed = (1 << 2),
} ListWidgetCommandState;
DEFINE_ENUM_FLAG_OPERATORS(ListWidgetCommandState);

typedef struct ListWidgetCommand ListWidgetCommand;

typedef struct ListWidgetActivity 
{
	unsigned int step;
	unsigned int percent;
	wchar_t *title;
	SIZE titleSize;
	BOOL cancelable;
}
ListWidgetActivity;

typedef struct ListWidgetActivityMetric
{
	long height;
	long width;
	long progressWidth;
	long progressHeight;
	long percentWidth;
	long percentHeight;
	long titleWidth;
	long titleHeight;
	long fontHeight;
	long offsetLeft;
	long offsetRight;
	long offsetTop;
	long offsetBottom;
	long spacing;
} ListWidgetActivityMetric;

typedef enum ListWidgetActivityChange
{
	ListWidgetActivityChanged_Nothing = 0,
	ListWidgetActivityChanged_Percent = (1 << 0),
	ListWidgetActivityChanged_Title = (1 << 1),
	ListWidgetActivityChanged_Cancelable = (1 << 2),
	ListWidgetActivityChanged_All = (ListWidgetActivityChanged_Percent | ListWidgetActivityChanged_Title | ListWidgetActivityChanged_Cancelable ),
}
ListWidtetActivityChange;
DEFINE_ENUM_FLAG_OPERATORS(ListWidtetActivityChange);


typedef enum ListWidgetItemPart
{
	ListWidgetItemPart_None = 0,
	ListWidgetItemPart_Frame = (1 << 0),
	ListWidgetItemPart_Image = (1 << 1),
	ListWidgetItemPart_Title = (1 << 2),
	ListWidgetItemPart_Activity = (1 << 3),
	ListWidgetItemPart_Command = (1 << 4),
	ListWidgetItemPart_Spacebar = (1 << 5),
	ListWidgetItemPart_Connection = (1 << 6),
}ListWidgetItemPart;
DEFINE_ENUM_FLAG_OPERATORS(ListWidgetItemPart);

typedef struct ListWidgetItem
{
	char *name;
	wchar_t *title;
	RECT rect;
	SIZE titleSize;
	DeviceImage *image;
	uint64_t spaceTotal;
	uint64_t spaceUsed;
	ListWidgetItemState state;
	ListWidgetConnection *connection;
	ListWidgetActivity *activity;
}ListWidgetItem;
typedef std::vector<ListWidgetItem*> ListWidgetItemList;

typedef struct ListWidgetGroup
{
	char *name;
	wchar_t *title;
	ListWidgetItemList items;
} ListWidgetGroup;
typedef std::vector<ListWidgetGroup*> ListWidgetGroupList;

typedef struct ListWidgetCategory
{
	char *name;
	wchar_t *title;
	BOOL collapsed;
	ListWidgetGroupList groups;
	RECT rect;
	long titleWidth;
	long countWidth;
	wchar_t *countString;
	wchar_t *emptyText;
	RECT	emptyTextRect;
}ListWidgetCategory;

typedef std::vector<ListWidgetCategory*> ListWidgetCategoryList;

typedef enum ListWidgetFlags
{
	ListWidgetFlag_NoFocusSelect = (1 << 0),
	ListWidgetFlag_LButtonDownOnCommand = (1 << 1),
} ListWidgetFlags;
DEFINE_ENUM_FLAG_OPERATORS(ListWidgetFlags);

typedef struct ListWidget
{
	ListWidgetFlags flags;
	ListWidgetCategoryList categories;
	ListWidgetConnectionList connections;
	BackBuffer backBuffer;
	ListWidgetItem *hoveredItem;
	ListWidgetItem *selectedItem;
	ListWidgetItem *titleEditItem;
	ListWidgetCategory *pressedCategory;
	SIZE imageSize;
	long itemWidth;
	size_t itemsPerLine;
	size_t deviceHandler;
	ListWidgetCommand **commands;
	size_t commandsCount;
	size_t commandsMax;
	ListWidgetItemList activeItems;
	POINT previousMouse;
	
	HBITMAP spacebarBitmap;
	HBITMAP arrowsBitmap;
	
	HBITMAP hoverBitmap;
	HBITMAP selectBitmap;
	HBITMAP inactiveSelectBitmap;
	
	HBITMAP largeBadgeBitmap;
	HBITMAP smallBadgeBitmap;
	
	SIZE connectionSize;
	SIZE primaryCommandSize;
	SIZE secondaryCommandSize;
	DeviceImage *unknownCommandLargeImage;
	DeviceImage *unknownCommandSmallImage;
	
	ListWidgetActivityMetric activityMetrics;
	HFONT activityFont;
	HBITMAP activityBadgeBitmap;
	DeviceImage *activityProgressImage;
	BOOL activityTimerEnabled;
	
	HMENU  activeMenu;
	ListWidgetTooltip *tooltip;

	unsigned int selectionStatus;
	HWND titleEditor;

} ListWidget;

typedef struct 
ListWidgetItemMetric
{
	long titleMinWidth;
	long offsetLeft;
	long offsetTop;
	long offsetRight;
	long offsetBottom;
	long imageOffsetLeft;
	long imageOffsetTop;
	long imageOffsetRight;
	long imageOffsetBottom;
	long titleOffsetTop;
	long spacebarOffsetTop;
	long spacebarHeight;
} ListWidgetItemMetric;

typedef struct 
ListWidgetCategoryMetric
{
	long offsetLeft;
	long offsetTop;
	long offsetRight;
	long offsetBottom;
	long lineHeight;
	long lineOffsetTop;
	long titleOffsetLeft;
	long minHeight;
	long iconWidth;
	long iconHeight;
} ListWidgetCategoryMetric;

HBITMAP
ListWidget_GetSpacebarBitmap(ListWidget *self, 
							 WidgetStyle *style,
							 HWND hwnd,
							 long width, 
							 long height);

HBITMAP
ListWidget_GetHoverBitmap(ListWidget *self, 
							 WidgetStyle *style, 
							 HWND hwnd,
							 long width, 
							 long height);

HBITMAP
ListWidget_GetSelectBitmap(ListWidget *self, 
							 WidgetStyle *style, 
							 HWND hwnd,
							 long width, 
							 long height);

HBITMAP
ListWidget_GetInactiveSelectBitmap(ListWidget *self, 
								   WidgetStyle *style, 
								   HWND hwnd,
								   long width, 
								   long height);

HBITMAP
ListWidget_GetLargeBadgeBitmap(ListWidget *self, 
							   WidgetStyle *style, 
							   HWND hwnd,
							   long width, 
							   long height);


HBITMAP
ListWidget_GetSmallBadgeBitmap(ListWidget *self, 
					  		   WidgetStyle *style, 
							   HWND hwnd,
							   long width, 
							   long height);

HBITMAP
ListWidget_GetUnknownCommandSmallBitmap(ListWidget *self, 
					  					WidgetStyle *style, 
										long width, 
										long height);

HBITMAP
ListWidget_GetUnknownCommandLargeBitmap(ListWidget *self, 
					  					WidgetStyle *style, 
										long width, 
										long height);

HBITMAP
ListWidget_GetArrowsBitmap(ListWidget *self, 
						   WidgetStyle *style, 
						   HWND hwnd);

HBITMAP
ListWidget_GetActivityProgressBitmap(ListWidget *self, 
							 WidgetStyle *style);


HBITMAP
ListWidget_GetActivityBadgeBitmap(ListWidget *self,
								  WidgetStyle *style, 
								  HWND hwnd,
								  long width, 
								  long height);

BOOL
ListWidget_GetViewOrigin(HWND hwnd, 
						 POINT *pt);

BOOL
ListWidget_UpdateHoverEx(ListWidget *self, 
						 HWND hwnd, 
						 const POINT *cursor);

BOOL
ListWidget_UpdateHover(ListWidget *self, 
					   HWND hwnd);

BOOL
ListWidget_RemoveHover(ListWidget *self,
					   HWND hwnd,
					   BOOL invalidate);

BOOL
ListWidget_SelectItem(ListWidget *self, 
					  HWND hwnd, 
					  ListWidgetItem *item, 
					  BOOL ensureVisible);

BOOL
ListWidget_SetImageSize(ListWidget *self, 
						HWND hwnd, 
						int imageWidth, 
						int imageHeight, 
						BOOL redraw);

typedef enum ListWidgetLayoutFlags
{
	ListWidgetLayout_Normal = 0,
	ListWidgetLayout_NoRedraw = (1 << 0),
	ListWidgetLayout_UpdateNow = (1 << 1),
	ListWidgetLayout_KeepStable = (1 << 2),
}ListWidgetLayoutFlags;
DEFINE_ENUM_FLAG_OPERATORS(ListWidgetLayoutFlags);

BOOL
ListWidget_UpdateLayout(HWND hwnd, 
						ListWidgetLayoutFlags flags);

BOOL
ListWidget_DisplayContextMenu(ListWidget *self,
							  HWND hostWindow,
							  POINT pt);

BOOL
ListWidget_RegisterActiveItem(ListWidget *self,
							  HWND hwnd, 
							  ListWidgetItem *item);

BOOL
ListWidget_UnregisterActiveItem(ListWidget *self,
								HWND hwnd,
								ListWidgetItem *item);

double 
ListWidget_GetZoomRatio(ListWidget *self);

void
ListWidget_UpdateSelectionStatus(ListWidget *self,
								 HWND hwnd, 
								 BOOL ensureVisible);

void
ListWidget_UpdateSelectionSpaceStatus(ListWidget *self, 
									  HWND hwnd, 
									  BOOL ensureVisible);

void 
ListWidget_UpdateTitleEditorColors(HWND editor,
								   WidgetStyle *style);
/*
<<<<<<<<<<<<<<<<<<<<<<<<< Category >>>>>>>>>>>>>>>>>>>>>>>>>
*/

ListWidgetCategory *
ListWidget_CreateCategory(const char *name, 
						  const wchar_t *title, 
						  BOOL collapsed);

void 
ListWidget_DestroyCategory(ListWidgetCategory *category);

ListWidgetCategory *
ListWidget_GetCategoryFromPoint(ListWidget *self, 
								POINT point);

ListWidgetCategory *
ListWidget_FindCategory(ListWidget *self, 
						const char *name);

BOOL
ListWidget_GetCategoryMetrics(WidgetStyle *style,
							  ListWidgetCategoryMetric *metrics);

BOOL 
ListWidget_ToggleCategory(ListWidgetCategory *category, 
						  HWND hwnd);

void
ListWidget_ResetCategoryCounter(ListWidgetCategory *category);

void 
ListWidget_SortCategory(ListWidgetCategory *category);

BOOL
ListWidget_SetCategoryEmptyText(ListWidgetCategory *category, const wchar_t *text);

/*
<<<<<<<<<<<<<<<<<<<<<<<<< Group >>>>>>>>>>>>>>>>>>>>>>>>>
*/

ListWidgetGroup *
ListWidget_CreateGroup(const char *name);

ListWidgetGroup *
ListWidget_CreateGroupEx(const char *name, 
						 const wchar_t *title);

void 
ListWidget_DestroyGroup(ListWidgetGroup *group);

BOOL
ListWidget_AddGroup(ListWidgetCategory *category,
					ListWidgetGroup *group);
					

ListWidgetGroup *
ListWidget_FindGroup(ListWidgetCategory *category, 
					 const char *name);

ListWidgetGroup *
ListWidget_FindGroupEx(ListWidgetCategory *category, 
					 const char *name, 
					 size_t max);

void 
ListWidget_SortGroup(ListWidgetGroup *group);

/*
<<<<<<<<<<<<<<<<<<<<<<<<< Item >>>>>>>>>>>>>>>>>>>>>>>>>
*/

typedef enum ListWidgetVisibleFlags
{
	VISIBLE_NORMAL	= 0,
	VISIBLE_PARTIAL_OK	= (1 << 0),
	VISIBLE_ALIGN_BOTTOM =(1 << 1),
	VISIBLE_ALIGN_TOP = (1 << 2),
	VISIBLE_ALIGN_ALWAYS = (1 << 3),
} ListWidgetVisibleFlags;
DEFINE_ENUM_FLAG_OPERATORS(ListWidgetVisibleFlags);

ListWidgetItem*
ListWidget_CreateItemFromDevice(ListWidget *self, 
								ifc_device* device);

void 
ListWidget_DestroyItem(ListWidgetItem *item);

BOOL
ListWidget_CalculateItemBaseSize(ListWidget *self,
								 WidgetStyle *style, 
								 SIZE *baseSize, 
								 long *itemTextWidth);

size_t		// number of removed items 
ListWidget_RemoveItem(ListWidget *self, 
					  HWND hwnd, 
					  const char *name);

ListWidgetItem *
ListWidget_GetFirstItem(ListWidget *self);

ListWidgetItem *
ListWidget_GetLastItem(ListWidget *self);

ListWidgetItem *
ListWidget_GetNextItem(ListWidget *self, 
					   ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetPreviousItem(ListWidget *self, 
						   ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetNextCategoryItem(ListWidget *self,
							   ListWidgetCategory *category,
							   ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetPreviousCategoryItem(ListWidget *self,
								   ListWidgetCategory *category,
								   ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetNextGroupItem(ListWidget *self,
							ListWidgetGroup *group,
							ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetPreviousGroupItem(ListWidget *self,
								ListWidgetGroup *group,
								ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetNextLineItem(ListWidget *self, 
						   ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetPreviousLineItem(ListWidget *self, 
							   ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetNextPageItem(ListWidget *self, 
						   HWND hwnd, 
						   ListWidgetItem *baseItem);

ListWidgetItem *
ListWidget_GetPreviousPageItem(ListWidget *self, 
						   HWND hwnd, 
						   ListWidgetItem *baseItem);

BOOL
ListWidget_EnsureItemVisisble(ListWidget *self, 
							  HWND hwnd, 
							  ListWidgetItem *item, 
							  ListWidgetVisibleFlags flags);

HBITMAP
ListWidget_GetItemImage(ListWidget *self,
						WidgetStyle *style,
						ListWidgetItem *item);

BOOL
ListWidget_GetItemMetrics(WidgetStyle *style,
						  ListWidgetItemMetric *metrics);

ListWidgetItem *
ListWidget_GetItemFromPointEx(ListWidget *self, 
							POINT point, 
							ListWidgetCategory **categoryOut,  // optional
							ListWidgetGroup **groupOut); // optional

ListWidgetItem *
ListWidget_GetItemFromPoint(ListWidget *self, 
							POINT point);



BOOL
ListWidget_AddItem(ListWidgetGroup *group,
				   ListWidgetItem *item);

ListWidgetItem *
ListWidget_FindGroupItem(ListWidgetGroup *group,
					const char *name);

ListWidgetItem *
ListWidget_FindGroupItemEx(ListWidgetGroup *group,
					const char *name, 
					size_t max);

ListWidgetGroup *
ListWidget_GetItemOwner(ListWidget *self, 
						ListWidgetItem *baseItem, 
						ListWidgetCategory **categoryOut);


ListWidgetItem *
ListWidget_FindItem(ListWidget *self,
					const char *name,
					ListWidgetCategory **categoryOut,
					ListWidgetGroup **groupOut);

BOOL
ListWidget_FindItemPos(ListWidget *self,
					   ListWidgetItem *item,
					   size_t *categoryOut,
					   size_t *groupOut, 
					   size_t *itemOut);


BOOL
ListWidget_SetItemTitle(ListWidgetItem *item, 
						const wchar_t *title);

BOOL
ListWidget_DisplayItemContextMenu(ListWidget *self,
								  HWND hostWindow,
								  ListWidgetItem *item, 
								  POINT pt);

size_t
ListWidget_GetItemCommands(ListWidgetItem *item, 
						   ListWidgetCommand **buffer, 
						   size_t bufferMax);

BOOL
ListWidget_SendItemCommand(const char *name, 
						   const char *command, 
						   HWND hostWindow, 
						   ULONG_PTR param,
						   BOOL enableIntercept);

BOOL
ListWidget_CreateItemActivity(ListWidgetItem *item);

BOOL
ListWidget_DeleteItemActivity(ListWidgetItem *item);



ListWidtetActivityChange
ListWidget_UpdateItemActivity(ListWidgetItem *item, 
							  ifc_deviceactivity *activity);

BOOL
ListWidget_InvalidateItemImage(ListWidget *self, 
							   HWND hwnd, 
							   ListWidgetItem *item);

BOOL
ListWidget_InvalidateItemActivity(ListWidget *self,
								  HWND hwnd, 
								  ListWidgetItem *item,
								  ListWidgetActivityChange changes);


BOOL
ListWidget_GetItemFrameRect(ListWidget *self, 
							ListWidgetItem *item, 
							ListWidgetItemMetric *metrics, 
							RECT *rect);

BOOL
ListWidget_GetItemImageRect(ListWidget *self,
							ListWidgetItem *item,
							ListWidgetItemMetric *metrics,
							RECT *rect);

BOOL
ListWidget_GetItemActivityRect(ListWidget *self,
							   ListWidgetItem *item, 
							   ListWidgetItemMetric *metrics, 
							   RECT *rect);

BOOL
ListWidget_GetItemActivityProgressRect(ListWidget *self,
									   HDC hdc,
									   ListWidgetItem *item, 
									   ListWidgetItemMetric *metrics, 
									   RECT *rect);

BOOL
ListWidget_GetItemActivityPercentRect(ListWidget *self,
									  HDC hdc,
									  ListWidgetItem *item,
									  ListWidgetItemMetric *metrics,
									  RECT *rect);

BOOL
ListWidget_GetItemActivityTitleRect(ListWidget *self, 
									HDC hdc,
									ListWidgetItem *item, 
									ListWidgetItemMetric *metrics,
									RECT *rect);

BOOL
ListWidget_GetItemSpacebarRect(ListWidget *self,
							   ListWidgetItem *item,
							   ListWidgetItemMetric *metrics,
							   RECT *rect);

BOOL
ListWidget_GetItemTitleRect(ListWidget *self,
							   ListWidgetItem *item,
							   ListWidgetItemMetric *metrics,
							   BOOL exactSize,
							   RECT *rect);

BOOL
ListWidget_GetItemConnectionRect(ListWidget *self,
							   ListWidgetItem *item,
							   ListWidgetItemMetric *metrics,
							   RECT *rect);

ListWidgetItemPart
ListWidget_GetItemPartFromPoint(ListWidget *self,
								ListWidgetItem *item,
								ListWidgetItemMetric *metrics, 
								POINT pt, 
								ListWidgetItemPart mask, 
								RECT *partRect);

BOOL
ListWidget_FormatItemTip(ListWidget *self,
						 ListWidgetItem *item,
						 wchar_t *buffer, 
						 size_t bufferMax);

BOOL
ListWidget_FormatItemTitleTip(ListWidget *self,
							  ListWidgetItem *item,
							  wchar_t *buffer,
							  size_t bufferMax);

BOOL
ListWidget_FormatItemCommandTip(ListWidget *self, 
								ListWidgetItem *item, 
								const RECT *commandRect, 
								wchar_t *buffer, 
								size_t bufferMax);

BOOL
ListWidget_FormatItemSpaceTip(ListWidget *self, 
							  ListWidgetItem *item, 
							  wchar_t *buffer, 
							  size_t bufferMax);

BOOL
ListWidget_FormatItemStatus(ListWidget *self,
							ListWidgetItem *item,
							wchar_t *buffer,
							size_t bufferMax);

BOOL
ListWidget_FormatItemSpaceStatus(ListWidget *self,
								 ListWidgetItem *item, 
								 wchar_t *buffer, 
								 size_t bufferMax);
HWND
ListWidget_BeginItemTitleEdit(ListWidget *self, 
							  HWND hwnd, 
							  ListWidgetItem *item);

int
ListWidget_CompareItemPos(ListWidget *self, 
						  ListWidgetItem *item1,
						  ListWidgetItem *item2);

BOOL
ListWidget_GetViewItemPos(HWND hwnd, 
						  ListWidgetItem *item, 
						  POINT *pt);

/*
<<<<<<<<<<<<<<<<<<<<<<<<< Connection >>>>>>>>>>>>>>>>>>>>>>>>>
*/

ListWidgetConnection *
ListWidget_CreateConnection(const char *name);

void 
ListWidget_DestroyConnection(ListWidgetConnection *connection);


HBITMAP
ListWidget_GetConnectionImage(WidgetStyle *style, 
							  ListWidgetConnection *connection, 
							  int width, 
							  int height);

BOOL
ListWidget_ConnectionResetColors(WidgetStyle *style, 
								 ListWidgetConnection *connection);

void
ListWidget_ResetConnnectionsColors(ListWidget *self, 
								   WidgetStyle *style);

ListWidgetConnection *
ListWidget_FindConnection(ListWidget *self, 
						  const char *name);

BOOL
ListWidget_AddConnection(ListWidget *self, 
						 ListWidgetConnection *connection);

void
ListWidget_RemoveConnection(ListWidget *self, 
							const char *name);

void
ListWidget_RemoveAllConnections(ListWidget *self);


BOOL
ListWidget_UpdateConnectionImageSize(ListWidgetConnection *connection,
									 int width,
									 int height);

/*
<<<<<<<<<<<<<<<<<<<<<<<<< Command >>>>>>>>>>>>>>>>>>>>>>>>>
*/

ListWidgetCommand *
ListWidget_CreateCommand(const char *name, 
						 BOOL primary,
						 BOOL disabled);

void
ListWidget_DestroyCommand(ListWidgetCommand *command);

size_t 
ListWigdet_GetDeviceCommands(ListWidgetCommand **buffer, 
							 size_t bufferMax, 
							 ifc_device *device);

void
ListWidget_DestroyAllCommands(ListWidgetCommand** buffer,
							  size_t bufferMax);

const wchar_t *
ListWidget_GetCommandTitle(ListWidgetCommand *command);

const wchar_t *
ListWidget_GetCommandDescription(ListWidgetCommand *command);

HBITMAP
ListWidget_GetCommandLargeBitmap(WidgetStyle *style, 
								 ListWidgetCommand *command, 
								 int width,
								 int height);

HBITMAP
ListWidget_GetCommandSmallBitmap(WidgetStyle *style, 
								 ListWidgetCommand *command, 
								 int width,
								 int height);

BOOL
ListWidget_ResetCommandImages(ListWidgetCommand *command);

BOOL
ListWidget_GetCommandRect(ListWidgetCommand *command, 
						  RECT *rect);

BOOL
ListWidget_SetCommandRect(ListWidgetCommand *command, 
						  const RECT *rect);

BOOL
ListWidget_GetCommandRectEqual(ListWidgetCommand *command, 
							   const RECT *rect);

BOOL
ListWidget_GetCommandPrimary(ListWidgetCommand *command);

BOOL
ListWidget_GetCommandDisabled(ListWidgetCommand *command);


BOOL
ListWidget_EnableCommand(ListWidgetCommand *command,
						 BOOL enable);

BOOL
ListWidget_GetCommandPressed(ListWidgetCommand *command);


BOOL
ListWidget_SetCommandPressed(ListWidgetCommand *command,
							 BOOL pressed);

const char *
ListWidget_GetCommandName(ListWidgetCommand *command);

/*
<<<<<<<<<<<<<<<<<<<<<<<<< Paint >>>>>>>>>>>>>>>>>>>>>>>>>
*/

typedef struct ListWidgetPaintSpacebar
{
	HBITMAP bitmap;
	long width;
	long height;
	long emptyBarOffset;
	long filledBarOffset;
} ListWidgetPaintSpacebar;

typedef struct ListWidgetPaintArrow
{
	HBITMAP bitmap;
	long width;
	long height;
	long collapsedOffset;
	long expandedOffset;
} ListWidgetPaintArrow;


typedef struct ListWidgetPaint
{
	ListWidget *widget;
	WidgetStyle *style;
	HWND hwnd;
	HDC hdc;
	BOOL erase;
	const RECT *paintRect;
	HDC sourceDC;
	ListWidgetPaintSpacebar spacebar;
	ListWidgetItemMetric itemMetrics;
	ListWidgetCategoryMetric categoryMetrics;
	ListWidgetPaintArrow arrow;
	RECT partRect;
	BOOL focused;
} ListWidgetPaint;

BOOL
ListWidgetPaint_Initialize(ListWidgetPaint *self, 
						   ListWidget *widget, 
						   WidgetStyle *style,
						   HWND hwnd,
						   HDC hdc,
						   const RECT *paintRect, 
						   BOOL erase);

void
ListWidgetPaint_Uninitialize(ListWidgetPaint *self);

BOOL
ListWidgetPaint_DrawItem(ListWidgetPaint *self, 
						 ListWidgetItem *item);

BOOL
ListWidgetPaint_DrawCategory(ListWidgetPaint *self, 
							 ListWidgetCategory *category);

BOOL
ListWidgetPaint_DrawEmptyCategoryText(ListWidgetPaint *self, 
									  ListWidgetCategory *category);


/*
<<<<<<<<<<<<<<<<<<<<<<<<< Tooltip >>>>>>>>>>>>>>>>>>>>>>>>>
*/

ListWidgetTooltip*
ListWidget_TooltipCreate(HWND hwnd);

void
ListWidget_TooltipDestroy(ListWidgetTooltip *tooltip);

void
ListWidget_TooltipFontChanged(ListWidgetTooltip *tooltip);

BOOL
ListWidget_TooltipActivate(ListWidgetTooltip *tooltip, 
						   const RECT *rect);

BOOL
ListWidget_TooltipUpdate(ListWidgetTooltip *tooltip, 
						 ListWidgetItem *item, 
						 ListWidgetItemPart part, 
						 const RECT *partRect);

void
ListWidget_TooltipHide(ListWidgetTooltip *tooltip);

void
ListWidget_TooltipRelayMouseMessage(ListWidgetTooltip *tooltip, 
									unsigned int message, 
									unsigned int vKeys,
									const POINT *cursor);
BOOL
ListWidget_TooltipProcessNotification(ListWidget *self, 
									  ListWidgetTooltip *tooltip, 
									  NMHDR *pnmh,
									  LRESULT *result);

ListWidgetItem *
ListWidget_TooltipGetCurrent(ListWidgetTooltip *tooltip, 
							 ListWidgetItemPart *part, 
							 RECT *partRect);

BOOL
ListWidget_TooltipGetChanged(ListWidgetTooltip *tooltip,
							 ListWidgetItem *item,
							 ListWidgetItemPart part,
							 const RECT *partRect);

typedef enum TooltipUpdateReason
{
	Tooltip_DeviceTitleChanged = 1,
	Tooltip_DeviceSpaceChanged = 2,
	Tooltip_DeviceActivityChanged = 3,
	Tooltip_DeviceModelChanged = 4,
	Tooltip_DeviceStatusChanged = 5,
} TooltipUpdateReason;

BOOL
ListWidget_TooltipUpdateText(ListWidget *self, 
							 ListWidgetTooltip *tooltip,
							 ListWidgetItem *item,
							 TooltipUpdateReason reason);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_LIST_WIDGET_INTERNAL_HEADER

