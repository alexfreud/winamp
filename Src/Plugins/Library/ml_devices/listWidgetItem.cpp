#include "main.h"
#include "./listWidgetInternal.h"
#include "../nu/AutoWide.h"
#include <strsafe.h>

#define LISTWIDGETITEM_OFFSET_LEFT_DLU			0
#define LISTWIDGETITEM_OFFSET_TOP_DLU			0
#define LISTWIDGETITEM_OFFSET_RIGHT_DLU			0
#define LISTWIDGETITEM_OFFSET_BOTTOM_DLU		2
#define LISTWIDGETITEM_IMAGE_OFFSET_LEFT_DLU	2
#define LISTWIDGETITEM_IMAGE_OFFSET_TOP_DLU		2
#define LISTWIDGETITEM_IMAGE_OFFSET_RIGHT_DLU	2
#define LISTWIDGETITEM_IMAGE_OFFSET_BOTTOM_DLU	2
#define LISTWIDGETITEM_SPACEBAR_OFFSET_DLU		1
#define LISTWIDGETITEM_SPACEBAR_HEIGHT_DLU		8
#define LISTWIDGETITEM_TITLE_OFFSET_DLU			1
#define LISTWIDGETITEM_TITLE_MIN_WIDTH_DLU		(8 * 4)
#define LISTWIDGETITEM_TITLE_EDITOR_MARGIN_HORZ_DLU 2

ListWidgetItem*
ListWidget_CreateItemFromDevice(ListWidget *self, ifc_device* device)
{
	ListWidgetItem *item;
	ifc_deviceactivity *activity;
	wchar_t buffer[1024] = {0};

	if (NULL == device || NULL == device->GetName())
		return NULL;

	item = new ListWidgetItem();
	if (NULL == item)
		return NULL;
		
	item->name = AnsiString_Duplicate(device->GetName());
	if (NULL == item->name)
	{
		delete item;
		return NULL;
	}

	if (SUCCEEDED(device->GetDisplayName(buffer, ARRAYSIZE(buffer))))
		item->title = String_Duplicate(buffer);
	else
		item->title = NULL;

	ListWidgetItem_UnsetTextTruncated(item);
	SetSize(&item->titleSize, -1, -1);
	
	item->image = NULL;
		
	if (FAILED(device->GetTotalSpace(&item->spaceTotal)))
		item->spaceTotal = 0;

	if (FAILED(device->GetUsedSpace(&item->spaceUsed)))
		item->spaceUsed = 0;
				
	item->connection = NULL;
	if (NULL != self)
	{
		item->connection = ListWidget_FindConnection(self, device->GetConnection());
		if (NULL == item->connection)
		{
			item->connection = ListWidget_CreateConnection(device->GetConnection());
			if (NULL != item->connection)
				ListWidget_AddConnection(self, item->connection);
		}
	}



	item->activity = NULL;
	if (S_OK == device->GetActivity(&activity) && NULL != activity)
	{
		if (FALSE != activity->GetActive())
		{
			ListWidget_CreateItemActivity(item);
			ListWidget_UpdateItemActivity(item, activity);
		}

		activity->Release();
	}
	return item;
}

void 
ListWidget_DestroyItem(ListWidgetItem *item)
{
	if (NULL == item)
		return;

	if (NULL != item->image)
		DeviceImage_Release(item->image);

	ListWidget_DeleteItemActivity(item);

	AnsiString_Free(item->name);
	String_Free(item->title);



	delete item;
}

BOOL
ListWidget_SetItemTitle(ListWidgetItem *item, const wchar_t *title)
{
	if (NULL == item)
		return FALSE;

	String_Free(item->title);
	SetSize(&item->titleSize, -1, -1);
	ListWidgetItem_UnsetTextTruncated(item);
	
	item->title = String_Duplicate(title);

	if (NULL != title && NULL == item->title)
		return FALSE;

	return TRUE;

}

size_t
ListWidget_RemoveItem(ListWidget *self, HWND hwnd, const char *name)
{
	size_t iCategory, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	size_t removed;
	RECT rect;
	POINT origin;
	ListWidgetItem *selectItem;

	removed = 0;
	selectItem = NULL;

	if (NULL == self || NULL == name)
		return 0;

	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
		ZeroMemory(&origin, sizeof(POINT));

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
		BOOL categoryModified = FALSE;

		size_t iGroup = category->groups.size();
		while(iGroup--)
		{
			group = category->groups[iGroup];
			iItem = group->items.size();
			while(iItem--)
			{
				item = group->items[iItem];
				if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, item->name, -1, name, -1))
				{
					removed++;
					categoryModified = TRUE;

					if (NULL != item->activity)
						ListWidget_UnregisterActiveItem(self, hwnd, item);

					if (self->selectedItem == item)
					{
						self->selectedItem = NULL;

						if (NULL != self->activeMenu)
							EndMenu();

						ListWidget_UpdateSelectionStatus(self, hwnd, FALSE);

						selectItem = ListWidget_GetNextGroupItem(self, group, item);
						if (NULL == selectItem)
						{
							selectItem = ListWidget_GetPreviousGroupItem(self, group, item);
							if (NULL == selectItem)
							{
								selectItem = ListWidget_GetNextCategoryItem(self, category, item);
								if (NULL == selectItem)
									selectItem = ListWidget_GetPreviousCategoryItem(self, category, item);
							}
						}
					}

					if (self->hoveredItem == item)
						self->hoveredItem = NULL;

					group->items.erase(group->items.begin() + iItem);
					ListWidget_DestroyItem(item);

					if (0 == group->items.size())
					{
						category->groups.erase(category->groups.begin() + iGroup);
						ListWidget_DestroyGroup(group);
						break;
					}
				}
			}
		}

		if (FALSE != categoryModified)
		{
			ListWidget_ResetCategoryCounter(category);

			if (FALSE == category->collapsed)
			{
				ListWidget_UpdateLayout(hwnd, ListWidgetLayout_UpdateNow | ListWidgetLayout_KeepStable);
			}
			else
			{
				CopyRect(&rect, &category->rect);
				OffsetRect(&rect, origin.x, origin.y);
				InvalidateRect(hwnd, &rect, FALSE);
			}
		}
	}

	if (0 != removed && NULL != selectItem)
		ListWidget_SelectItem(self, hwnd, selectItem, FALSE);

	return removed;
}

BOOL
ListWidget_GetItemMetrics(WidgetStyle *style, ListWidgetItemMetric *metrics)
{
	if (NULL == metrics || NULL == style)
		return FALSE;
	
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->offsetLeft, style, LISTWIDGETITEM_OFFSET_LEFT_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->offsetTop, style, LISTWIDGETITEM_OFFSET_TOP_DLU, 1);
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->offsetRight, style, LISTWIDGETITEM_OFFSET_RIGHT_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->offsetBottom, style, LISTWIDGETITEM_OFFSET_BOTTOM_DLU, 1);

	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->imageOffsetLeft, style, LISTWIDGETITEM_IMAGE_OFFSET_LEFT_DLU, 2);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->imageOffsetTop, style, LISTWIDGETITEM_IMAGE_OFFSET_TOP_DLU, 2);
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->imageOffsetRight, style, LISTWIDGETITEM_IMAGE_OFFSET_RIGHT_DLU, 2);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->imageOffsetBottom, style, LISTWIDGETITEM_IMAGE_OFFSET_BOTTOM_DLU, 2);

	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->spacebarOffsetTop, style, LISTWIDGETITEM_SPACEBAR_OFFSET_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->spacebarHeight, style, LISTWIDGETITEM_SPACEBAR_HEIGHT_DLU, 2);
	metrics->spacebarHeight = 14;
	
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->titleOffsetTop, style, LISTWIDGETITEM_TITLE_OFFSET_DLU, 1);
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->titleMinWidth, style, LISTWIDGETITEM_TITLE_MIN_WIDTH_DLU, 32);

	return TRUE;
}

static HBITMAP
ListWidget_GetDeviceBitmap(ifc_device *device, int width, int height, 
						   DeviceImageFlags flags, DeviceImage **imageOut)
{
	HBITMAP bitmap;
	wchar_t path[MAX_PATH*2] = {0};
	const wchar_t *defaultImage;
	
	DeviceImage *image;
	DeviceImageCache *imageCache;
	ifc_devicetype *type;

	if (NULL == device)
		return NULL;
				
	imageCache = Plugin_GetImageCache();
	if (NULL == imageCache)
		return NULL;

	if (SUCCEEDED(device->GetIcon(path, ARRAYSIZE(path), width, height)))
	{
		image = DeviceImageCache_GetImage(imageCache, path, width, height, NULL, NULL);
		if (NULL != image)
		{
			bitmap = DeviceImage_GetBitmap(image, flags);
			if (NULL != bitmap)
			{
				if (NULL != imageOut)
					*imageOut = image;
				else
					DeviceImage_Release(image);

				return bitmap;
			}
		}
	}
		
	if (NULL != WASABI_API_DEVICES && 
		S_OK == WASABI_API_DEVICES->TypeFind(device->GetType(), &type))
	{
		if (SUCCEEDED(type->GetIcon(path, ARRAYSIZE(path), width, height)))
		{
			image = DeviceImageCache_GetImage(imageCache, path, width, height, NULL, NULL);
			if (NULL != image)
			{
				bitmap = DeviceImage_GetBitmap(image, flags);
				if (NULL != bitmap)
				{
					if (NULL != imageOut)
						*imageOut = image;
					else
						DeviceImage_Release(image);

					type->Release();
					return bitmap;
				}
			}
		}
		type->Release();
	}
	
	defaultImage = Plugin_GetDefaultDeviceImage(width, height);
	if (NULL != defaultImage)
	{	
		image = DeviceImageCache_GetImage(imageCache, defaultImage, width, height, NULL, NULL);
		if (NULL != image)
		{
			bitmap = DeviceImage_GetBitmap(image, flags);
			if (NULL != bitmap)
			{
				if (NULL != imageOut)
					*imageOut = image;
				else
					DeviceImage_Release(image);
				return bitmap;
			}
		}
	}

	return NULL;
}


HBITMAP
ListWidget_GetItemImage(ListWidget *self, WidgetStyle *style, ListWidgetItem *item)
{
	HBITMAP bitmap;

	if (NULL == item)
		return NULL;
	
	if (NULL != item->image)
		return DeviceImage_GetBitmap(item->image, DeviceImage_Normal);
	
	if (NULL == self || NULL == style)
		return NULL;
						
	ifc_device *device;
	if (NULL == WASABI_API_DEVICES ||
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return NULL;
	}
	
				
	bitmap = ListWidget_GetDeviceBitmap(device, self->imageSize.cx, self->imageSize.cy, 
							DeviceImage_Normal, &item->image);
	
	device->Release();

	return bitmap;
	
}

BOOL
ListWidget_CalculateItemBaseSize(ListWidget *self, WidgetStyle *style, SIZE *baseSize, long *itemTextWidth)
{
	ListWidgetItemMetric metrics;

	if (NULL == baseSize)
		return FALSE;

	if (FALSE == ListWidget_GetItemMetrics(style, &metrics))
		ZeroMemory(&metrics, sizeof(metrics));
	
	baseSize->cx = self->imageSize.cx;
	baseSize->cy = self->imageSize.cy;

	baseSize->cx += metrics.imageOffsetLeft + metrics.imageOffsetRight;
	if (baseSize->cx < metrics.titleMinWidth)
		baseSize->cx = metrics.titleMinWidth;

	if (FALSE != itemTextWidth)
		*itemTextWidth = baseSize->cx;

	baseSize->cx +=	metrics.offsetLeft + metrics.offsetRight;
	
	baseSize->cy += metrics.offsetTop + metrics.offsetBottom +
					metrics.imageOffsetTop + metrics.imageOffsetBottom +
					metrics.spacebarHeight + metrics.spacebarOffsetTop +
					metrics.titleOffsetTop; 

	if (FALSE != itemTextWidth)
		*itemTextWidth = baseSize->cx - (metrics.offsetLeft + metrics.offsetRight);
	
	return TRUE;
}



ListWidgetItem *
ListWidget_GetItemFromPointEx(ListWidget *self, POINT point, 
							ListWidgetCategory **categoryOut, ListWidgetGroup **groupOut)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;

	if (NULL == self)
		return NULL;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					
					if (FALSE != PtInRect(&item->rect, point))
					{
						if (NULL != categoryOut)
							*categoryOut = category;

						if (NULL != groupOut)
							*groupOut = group;

						return item;
					}
				}
			}
		}
	}
	
	if (NULL != categoryOut)
		*categoryOut = NULL;

	if (NULL != groupOut)
		*groupOut = NULL;

	return NULL;
}

ListWidgetItem *
ListWidget_GetItemFromPoint(ListWidget *self, POINT point)
{
	return ListWidget_GetItemFromPointEx(self, point, NULL, NULL);
}

ListWidgetItem *
ListWidget_GetFirstItem(ListWidget *self)
{
	size_t iCategory, iGroup;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;

	if (NULL == self)
		return NULL;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				if (group->items.size() > 0)
					return group->items[0];
			}
		}
	}
	return NULL;
}

ListWidgetItem *
ListWidget_GetLastItem(ListWidget *self)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;

	if (NULL == self)
		return NULL;

	iCategory = self->categories.size();
	while(iCategory--)
	{
		category = self->categories[iCategory];
		if (FALSE == category->collapsed)
		{
			iGroup = category->groups.size();
			while(iGroup--)
			{
				group = category->groups[iGroup];
				iItem = group->items.size();
				if (iItem > 0)
					return group->items[iItem - 1];
			}
		}
	}
	return NULL;
}

ListWidgetItem *
ListWidget_GetNextItem(ListWidget *self, ListWidgetItem *baseItem)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	BOOL returnNext;

	if (NULL == self || NULL == baseItem)
		return NULL;
	
	returnNext = FALSE;
	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					if (item == baseItem)
						returnNext = TRUE;
					else if (FALSE != returnNext)
						return item;
				}
			}
		}
	}
	return NULL;
}

ListWidgetItem *
ListWidget_GetPreviousItem(ListWidget *self, ListWidgetItem *baseItem)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	BOOL returnPrevious;

	if (NULL == self || NULL == baseItem)
		return NULL;
		
	returnPrevious = FALSE;
	iCategory = self->categories.size();
	while(iCategory--)
	{
		category = self->categories[iCategory];
		if (FALSE == category->collapsed)
		{
			iGroup = category->groups.size();
			while(iGroup--)
			{
				group = category->groups[iGroup];
				iItem = group->items.size();
				while(iItem--)
				{
					item = group->items[iItem];
					if (item == baseItem)
						returnPrevious = TRUE;
					else if (FALSE != returnPrevious)
						return item;
				}
			}
		}
	}
	return NULL;	
}

ListWidgetItem *
ListWidget_GetNextCategoryItem(ListWidget *self, ListWidgetCategory *category, ListWidgetItem *baseItem)
{
	size_t iGroup, iItem;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	BOOL returnNext;

	if (NULL == self || NULL == baseItem || 
		NULL == category || FALSE != category->collapsed)
	{
		return NULL;
	}
	
	returnNext = FALSE;
	
	for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
	{
		group = category->groups[iGroup];
		for(iItem = 0; iItem < group->items.size(); iItem++)
		{
			item = group->items[iItem];
			if (item == baseItem)
				returnNext = TRUE;
			else if (FALSE != returnNext)
				return item;
		}
	}
	
	return NULL;
}

ListWidgetItem *
ListWidget_GetPreviousCategoryItem(ListWidget *self, ListWidgetCategory *category, ListWidgetItem *baseItem)
{
	if (NULL == self || NULL == baseItem || 
		NULL == category || FALSE != category->collapsed)
	{
		return NULL;
	}

	BOOL returnPrevious = FALSE;
	size_t iGroup = category->groups.size();
	while(iGroup--)
	{
		ListWidgetGroup	*group = category->groups[iGroup];
		size_t iItem = group->items.size();
		while(iItem--)
		{
			ListWidgetItem *item = group->items[iItem];
			if (item == baseItem)
				returnPrevious = TRUE;
			else if (FALSE != returnPrevious)
				return item;
		}
	}

	return NULL;
}

ListWidgetItem *
ListWidget_GetNextGroupItem(ListWidget *self, ListWidgetGroup *group, ListWidgetItem *baseItem)
{
	if (NULL == self || NULL == baseItem || NULL == group)
		return NULL;

	BOOL returnNext = FALSE;
	for(size_t iItem = 0; iItem < group->items.size(); iItem++)
	{
		ListWidgetItem *item = group->items[iItem];
		if (item == baseItem)
			returnNext = TRUE;
		else if (FALSE != returnNext)
			return item;
	}

	return NULL;
}

ListWidgetItem *
ListWidget_GetPreviousGroupItem(ListWidget *self, ListWidgetGroup *group, ListWidgetItem *baseItem)
{
	if (NULL == self || NULL == baseItem || NULL == group)
		return NULL;

	BOOL returnPrevious = FALSE;
	size_t iItem = group->items.size();
	while(iItem--)
	{
		ListWidgetItem *item = group->items[iItem];
		if (item == baseItem)
			returnPrevious = TRUE;
		else if (FALSE != returnPrevious)
			return item;
	}

	return NULL;
}

ListWidgetItem *
ListWidget_GetNextLineItem(ListWidget *self, ListWidgetItem *baseItem)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	size_t itemLinePos, itemsPerLine, targetLinePos;

	if (NULL == self || NULL == baseItem)
		return NULL;
	
	itemsPerLine = MAX(self->itemsPerLine, 1);
	targetLinePos = -1;
		
	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				if (-1 == targetLinePos)
				{
					itemLinePos = 0;
					for(iItem = 0; iItem < group->items.size(); iItem++, itemLinePos++)
					{
						if (itemLinePos == itemsPerLine)
							itemLinePos = 0;

						item = group->items[iItem];
						if (item == baseItem)
						{
							size_t test;
							targetLinePos = itemLinePos;
							test = iItem + (itemsPerLine - itemLinePos);
							if (test < group->items.size())
							{
								test += targetLinePos;
								if (test >= group->items.size())
									test = group->items.size() - 1;
								return group->items[test];
							}
							break;
						}
					}
				}
				else if (group->items.size() > 0)
				{				
					size_t test;

					if (targetLinePos < group->items.size())
						test = targetLinePos;
					else
						test = group->items.size() - 1;
					return group->items[test];
				}

			}
		}
	}

	return NULL;	
}

ListWidgetItem *
ListWidget_GetPreviousLineItem(ListWidget *self, ListWidgetItem *baseItem)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	size_t itemLinePos, itemsPerLine, targetLinePos;

	if (NULL == self || NULL == baseItem)
		return NULL;
	
	itemsPerLine = MAX(self->itemsPerLine, 1);
	targetLinePos = -1;
		
	iCategory = self->categories.size();
	while(iCategory--)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			iGroup = category->groups.size();
			while(iGroup--)
			{
				group = category->groups[iGroup];
				if (-1 == targetLinePos)
				{
					itemLinePos = 0;
					for(iItem = 0; iItem < group->items.size(); iItem++, itemLinePos++)
					{
						if (itemLinePos == itemsPerLine)
							itemLinePos = 0;

						item = group->items[iItem];
						if (item == baseItem)
						{
							targetLinePos = itemLinePos;
							if (iItem >= (itemLinePos + 1))
							{
								size_t test = iItem - (itemLinePos + 1);
								if (test >= (itemsPerLine - (itemLinePos + 1)))
									test -= itemsPerLine - (itemLinePos + 1);
								return group->items[test];
							}
							break;
						}
					}
				}
				else if (group->items.size() > 0)
				{
					size_t test = group->items.size();
					test = test/itemsPerLine + ((0 != test%itemsPerLine) ? 0 : - 1);
					test = test * itemsPerLine;
					test += targetLinePos;
					if (test >= group->items.size())
						test = group->items.size() - 1;
					return group->items[test];
				}

			}
		}
	}

	return NULL;
}

static ListWidgetItem *
ListWidget_FindLastVisibleLine(ListWidget *self, long viewBottom)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	ListWidgetItem *lineItem;

	lineItem = NULL;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					if (item->rect.top < viewBottom)
					{
						if (NULL == lineItem || 
							item->rect.top != lineItem->rect.top)
						{
							if (item->rect.bottom <= viewBottom)
								lineItem = item;
							else 
								return (NULL != lineItem) ? lineItem : item;
						}
					}
					else
						return lineItem;
				}
			}
		}
	}

	return lineItem;
}

static ListWidgetItem *
ListWidget_FindFirstVisibleLine(ListWidget *self, long viewTop)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	ListWidgetItem *lineItem;

	lineItem = NULL;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					if (NULL == lineItem || 
						item->rect.top != lineItem->rect.top)
					{
						lineItem = item;
						if (item->rect.top >= viewTop)
							return lineItem;
					}
				}
			}
		}
	}

	return lineItem;
}

static ListWidgetItem *
ListWidget_FindNextLine(ListWidget *self, ListWidgetItem *baseItem)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	BOOL foundItem;

	foundItem = FALSE;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					if (item == baseItem)
						foundItem = TRUE;
					if (FALSE != foundItem && 
						item->rect.top != baseItem->rect.top)
					{
						return item;
					}
				}
			}
		}
	}
	return NULL;
}

static ListWidgetItem *
ListWidget_FindPreviousLine(ListWidget *self, ListWidgetItem *baseItem)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	BOOL foundItem;

	foundItem = FALSE;

	iCategory = self->categories.size();
	while(iCategory--)
	{
		category = self->categories[iCategory];
	
		if (FALSE == category->collapsed)
		{
			iGroup = category->groups.size();
			while(iGroup--)
			{
				group = category->groups[iGroup];
				iItem = group->items.size();
				while(iItem--)
				{
					item = group->items[iItem];
					if (item == baseItem)
						foundItem = TRUE;
					if (FALSE != foundItem && 
						item->rect.top != baseItem->rect.top)
					{
						return item;
					}
				}
			}
		}
	}

	return NULL;
}

static ListWidgetItem *
ListWidget_FindLineItemAtPos(ListWidget *self, ListWidgetItem *beginLine, ListWidgetItem *linePosition)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;
	BOOL foundLine;

	foundLine = FALSE;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];

		if (FALSE == category->collapsed)
		{
			for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
			{				
				group = category->groups[iGroup];
				for(iItem = 0; iItem < group->items.size(); iItem++)
				{
					item = group->items[iItem];
					if (item == beginLine)
						foundLine = TRUE;
					
					if (FALSE != foundLine)
					{
						if (beginLine->rect.top == linePosition->rect.top)
						{
							if (item->rect.top != beginLine->rect.top)
							{
								return (iItem > 0) ? group->items[iItem - 1] : beginLine;
							}
						}
						else
						{
							if (item->rect.left == linePosition->rect.left)
								return item;
						}
					}
				}
				if (FALSE != foundLine)
				{
					if (group->items.size() > 0)
						return group->items[group->items.size() - 1];
					return NULL;
				}
			}
		}
	}

	return NULL;
}

ListWidgetItem *
ListWidget_GetNextPageItem(ListWidget *self, HWND hwnd, ListWidgetItem *baseItem)
{
	ListWidgetItem *lineItem;
	RECT rect;
	POINT origin;
	long viewBottom;

	if (NULL == self || NULL == baseItem)
		return NULL;

	if (FALSE == GetClientRect(hwnd, &rect))
		return NULL;

	if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
		OffsetRect(&rect, -origin.x, -origin.y);

	if (baseItem->rect.bottom < rect.top)
		viewBottom = baseItem->rect.top + RECTHEIGHT(rect);
	else
		viewBottom = rect.bottom;

	lineItem = ListWidget_FindLastVisibleLine(self, viewBottom);
	if (NULL == lineItem)
		return NULL;

	if (lineItem->rect.top <= baseItem->rect.top)
	{
		viewBottom = baseItem->rect.top + RECTHEIGHT(rect);
		lineItem = ListWidget_FindLastVisibleLine(self, viewBottom);
		if (NULL == lineItem)
			return NULL;
		if (lineItem->rect.top <= baseItem->rect.top)
		{
			lineItem = ListWidget_FindNextLine(self, baseItem);
			if (NULL == lineItem)
				return NULL;
		}
	}

	return ListWidget_FindLineItemAtPos(self, lineItem, baseItem);
}

ListWidgetItem *
ListWidget_GetPreviousPageItem(ListWidget *self, HWND hwnd, ListWidgetItem *baseItem)
{
	ListWidgetItem *lineItem;
	RECT rect;
	POINT origin;
	long viewTop;
		
	if (NULL == self || NULL == baseItem)
		return NULL;
	
	if (FALSE == GetClientRect(hwnd, &rect))
		return NULL;

	if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
		OffsetRect(&rect, -origin.x, -origin.y);

	if (baseItem->rect.top > rect.bottom)
		viewTop = baseItem->rect.bottom - RECTHEIGHT(rect);
	else
		viewTop = rect.top;

	lineItem = ListWidget_FindFirstVisibleLine(self, viewTop);
	if (NULL == lineItem)
		return NULL;

	if (lineItem->rect.top >= baseItem->rect.top)
	{
		viewTop = baseItem->rect.bottom - RECTHEIGHT(rect);
		lineItem = ListWidget_FindFirstVisibleLine(self, viewTop);
		if (NULL == lineItem)
			return NULL;
		if (lineItem->rect.top >= baseItem->rect.top)
		{
			lineItem = ListWidget_FindPreviousLine(self, baseItem);
			if (NULL == lineItem)
				return NULL;
		}
	}

	return ListWidget_FindLineItemAtPos(self, lineItem, baseItem);
}

BOOL
ListWidget_EnsureItemVisisble(ListWidget *self, HWND hwnd, ListWidgetItem *item, ListWidgetVisibleFlags flags)
{
	RECT rect;
	POINT pt;
	int dx, dy;

	if (NULL == self || NULL == item || NULL == hwnd)
		return FALSE;

	if (FALSE == GetClientRect(hwnd, &rect))
		return FALSE;

	if (FALSE != ListWidget_GetViewOrigin(hwnd, &pt))
		OffsetRect(&rect, -pt.x, -pt.y);

	if (0 == (VISIBLE_ALIGN_ALWAYS & flags))
	{
		if (item->rect.left >= rect.left && 
			item->rect.right <=  rect.right &&
			item->rect.top >= rect.top &&
			item->rect.bottom <= rect.bottom)
		{
			return FALSE;
		}
	}

	if (0 != (VISIBLE_PARTIAL_OK  & flags))
	{
		if (item->rect.left < rect.right && 
			item->rect.right > rect.left &&
			item->rect.top < rect.bottom &&
			item->rect.bottom > rect.top)
		{
			return FALSE;
		}
	}
	
	if (item->rect.right > rect.right)
		dx = item->rect.right - rect.right;
	else
		dx = 0;

	if ((item->rect.left - dx) < rect.left)
		dx = item->rect.left - rect.left;

	dy = 0;
	if (0 != (VISIBLE_ALIGN_TOP & flags))
	{
		dy = item->rect.bottom - rect.bottom;
	}
	else if (0 != (VISIBLE_ALIGN_BOTTOM & flags))
	{
		SCROLLINFO scrollInfo;
		scrollInfo.cbSize = sizeof(scrollInfo);
		scrollInfo.fMask = SIF_RANGE | SIF_PAGE;
		if (FALSE != GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		{
			dy = scrollInfo.nMax - rect.bottom;
		}
	}

	if ((item->rect.bottom - dy) > rect.bottom)
		dy = item->rect.bottom - rect.bottom;
	
	if ((item->rect.top - dy) < rect.top)
		dy = item->rect.top - rect.top;

	if (0 == dx && 0 == dy)
		return FALSE;
	
	if (FALSE == WIDGET_SCROLL(hwnd, dx, dy, TRUE))
		return FALSE;
	
	ListWidget_UpdateHover(self, hwnd);
	
	return TRUE;
}

BOOL
ListWidget_AddItem(ListWidgetGroup *group, ListWidgetItem *item)
{
	if (NULL == group || NULL == item)
		return FALSE;

	group->items.push_back(item);
	return TRUE;

}


ListWidgetItem *
ListWidget_FindGroupItemEx(ListWidgetGroup *group, const char *name, size_t max)
{
	size_t index, count;

	if (NULL == group || NULL == name)
		return NULL;

	count = group->items.size();
	if (max < count) 
		count = max;

	for(index = 0; index < count; index++)
	{
		ListWidgetItem *item = group->items[index];
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, -1, item->name, -1))
			return item;
	}

	return NULL;
}

ListWidgetItem *
ListWidget_FindGroupItem(ListWidgetGroup *group, const char *name)
{
	return ListWidget_FindGroupItemEx(group, name, -1);
}


ListWidgetGroup *
ListWidget_GetItemOwner(ListWidget *self, ListWidgetItem *baseItem, ListWidgetCategory **categoryOut)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;

	if (NULL == self || NULL == baseItem)
		return NULL;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
		{
			group = category->groups[iGroup];
			for(iItem = 0; iItem < group->items.size(); iItem++)
			{
				item = group->items[iItem];
					
				if (item == baseItem)
				{
					if (NULL != categoryOut)
						*categoryOut = category;
			
					return group;
				}
			}
		}
	}

	if (NULL != categoryOut)
		*categoryOut = NULL;

	return NULL;
}

ListWidgetItem *
ListWidget_FindItem(ListWidget *self, const char *name, 
					ListWidgetCategory **categoryOut, 
					ListWidgetGroup **groupOut)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;
	ListWidgetItem *item;

	if (NULL == self || NULL == name)
		return NULL;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
		{
			group = category->groups[iGroup];
			for(iItem = 0; iItem < group->items.size(); iItem++)
			{
				item = group->items[iItem];
					
				if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, -1, item->name, -1))
				{
					if (NULL != categoryOut)
						*categoryOut = category;

					if (NULL != groupOut)
						*groupOut = group;
			
					return item;
				}
			}
		}
	}

	return NULL;
}

BOOL
ListWidget_FindItemPos(ListWidget *self, ListWidgetItem *item,
					   size_t *categoryOut, size_t *groupOut, size_t *itemOut)
{
	size_t iCategory, iGroup, iItem;
	ListWidgetCategory *category;
	ListWidgetGroup	*group;

	if (NULL == self || NULL == item)
		return FALSE;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
	
		for(iGroup = 0; iGroup < category->groups.size(); iGroup++)
		{
			group = category->groups[iGroup];
			for(iItem = 0; iItem < group->items.size(); iItem++)
			{
				if (item == group->items[iItem])
				{
					if (NULL != categoryOut)
						*categoryOut = iCategory;

					if (NULL != groupOut)
						*groupOut = iGroup;

					if (NULL != itemOut)
						*itemOut = iItem;
					
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

BOOL
ListWidget_DisplayItemContextMenu(ListWidget *self, HWND hwnd, ListWidgetItem *item, POINT pt)
{
	HMENU menu;
	ifc_device *device;
	unsigned int commandId;
	BOOL succeeded;
	char *itemName;
	
	
	if (NULL == self || NULL == item)
		return FALSE;

	if (NULL != self->activeMenu)
		return FALSE;

	if (NULL == WASABI_API_DEVICES || 
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return FALSE;
	}
	
	menu = CreatePopupMenu();
	if (NULL != menu)
	{
		if (0 == Menu_InsertDeviceItems(menu, 0, 100, device, DeviceCommandContext_ViewMenu))
		{
			DestroyMenu(menu);
			menu = NULL;
		}
	}

	device->Release();

	if (NULL == menu)
		return FALSE;

	succeeded = FALSE;

	self->activeMenu = menu;
	itemName = AnsiString_Duplicate(item->name);
	
	if (FALSE != ListWidget_RemoveHover(self, hwnd, TRUE))
		UpdateWindow(hwnd);
	
	commandId = Menu_TrackPopup(Plugin_GetLibraryWindow(), menu, 
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION | TPM_VERTICAL | TPM_RETURNCMD,
					pt.x, pt.y, hwnd, NULL);


	self->activeMenu = NULL;

	if (0 != commandId)
	{
		const char *command;

		command = (const char*)Menu_GetItemData(menu, commandId, FALSE);
		succeeded = ListWidget_SendItemCommand(itemName, command, hwnd, 0, TRUE);
	}
	else
	{
		if (ERROR_SUCCESS == GetLastError())
			succeeded = TRUE;
	}

	Menu_FreeItemData(menu, 0, -1);

	AnsiString_Free(itemName);
	
	if (FALSE != ListWidget_UpdateHover(self, hwnd))
		UpdateWindow(hwnd);
	
	return succeeded;
}

size_t
ListWidget_GetItemCommands(ListWidgetItem *item, ListWidgetCommand **buffer, size_t bufferMax)
{
	size_t count;
	ifc_device *device;

	if (NULL == item)
		return 0;

	count = 0;

	if (NULL != WASABI_API_DEVICES &&
		S_OK == WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		count = ListWigdet_GetDeviceCommands(buffer, bufferMax, device);
		device->Release();
	}

	return count;
}

BOOL
ListWidget_SendItemCommand(const char *name, const char *command, HWND hostWindow, ULONG_PTR param, BOOL enableIntercept)
{
	BOOL succeeded;
	ifc_device *device;
	BOOL commandProcessed;

	if (NULL == name ||
		NULL == command ||
		NULL == WASABI_API_DEVICES ||
		S_OK != WASABI_API_DEVICES->DeviceFind(name, &device))
	{
		return FALSE;
	}

	commandProcessed = FALSE;
	succeeded = FALSE;

	if (FALSE != enableIntercept)
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, command, -1, "view_open", -1))
		{				
			succeeded = Navigation_SelectDevice(device->GetName());
			commandProcessed = succeeded;
		}
		else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, command, -1, "rename", -1))
		{
			succeeded = Navigation_EditDeviceTitle(device->GetName());
			commandProcessed = succeeded;
		}
	}						
	
	if (FALSE == commandProcessed &&
		SUCCEEDED(device->SendCommand(command, hostWindow, param)))
	{
			succeeded = TRUE;
	}

	device->Release();

	return succeeded;
}

BOOL
ListWidget_CreateItemActivity(ListWidgetItem *item)
{
	if (NULL == item)
		return FALSE;

	if (NULL == item->activity)
	{
		item->activity = (ListWidgetActivity*)malloc(sizeof(ListWidgetActivity));
		if (NULL == item->activity)
			return FALSE;
	}

	item->activity->step = 0;
	item->activity->cancelable = FALSE;
	item->activity->percent = (unsigned int)-1;
	item->activity->title = NULL;
	SetSizeEmpty(&item->activity->titleSize);
	
	return TRUE;
}

BOOL
ListWidget_DeleteItemActivity(ListWidgetItem *item)
{
	if (NULL == item || 
		NULL == item->activity)
	{
		return FALSE;
	}

	String_Free(item->activity->title);

	free(item->activity);
	item->activity = NULL;

	return TRUE;
}


ListWidtetActivityChange
ListWidget_UpdateItemActivity(ListWidgetItem *item, ifc_deviceactivity *activity)
{
	ListWidgetActivityChange changed;
	BOOL cancelable;
	unsigned int percent;
	wchar_t buffer[512] = {0};

	if (NULL == item || NULL == item->activity || NULL == activity)
		return ListWidgetActivityChanged_Nothing;

	changed = ListWidgetActivityChanged_Nothing;

	cancelable = activity->GetCancelable();
	if (item->activity->cancelable != cancelable)
	{
		changed |= ListWidgetActivityChanged_Cancelable;
		item->activity->cancelable = cancelable;
	}

	if(FAILED(activity->GetProgress(&percent)))
		percent = (unsigned int)-1;

	if (item->activity->percent != percent)
	{
		changed |= ListWidgetActivityChanged_Percent;
		item->activity->percent = percent;

	}

	if (FAILED(activity->GetDisplayName(buffer, ARRAYSIZE(buffer))))
		buffer[0] = L'\0';

	if (NULL == item->activity->title ||
		CSTR_EQUAL != CompareString(LOCALE_SYSTEM_DEFAULT, 0, item->activity->title, -1, buffer, -1))
	{
		changed |= ListWidgetActivityChanged_Title;

		String_Free(item->activity->title);
		item->activity->title = String_Duplicate(buffer);
		SetSizeEmpty(&item->activity->titleSize);
	}

	return changed;
}

BOOL
ListWidget_GetItemImageRect(ListWidget *self, ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{	
	if (NULL == item || NULL == rect)
		return FALSE;

	if (FALSE == CopyRect(rect, &item->rect))
		return FALSE;
	
	if (NULL != metrics)
	{
		rect->left += metrics->offsetLeft + metrics->imageOffsetLeft;
		rect->top += metrics->offsetTop + metrics->imageOffsetTop;
		rect->right -= metrics->offsetRight - metrics->imageOffsetRight;
		rect->bottom = rect->top + self->imageSize.cy;
	}

	return TRUE;
}

BOOL
ListWidget_GetItemFrameRect(ListWidget *self, ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{	
	if (NULL == item || NULL == rect)
		return FALSE;

	if (FALSE == CopyRect(rect, &item->rect))
		return FALSE;
	
	if (NULL != metrics)
	{
		rect->bottom = rect->top + metrics->offsetTop + 
						metrics->imageOffsetTop + metrics->imageOffsetBottom + 
						self->imageSize.cy;
	}

	return TRUE;
}

static BOOL
ListWidget_CalcItemActivityTitleSize(ListWidget *self, HDC hdc, ListWidgetActivity *activity)
{	
	BOOL result;
	HDC windowDC;
	HFONT prevFont;
	RECT rect;
	
	
	if (NULL == hdc)
	{
		windowDC = GetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE | DCX_NORESETATTRS);
		if (NULL == windowDC)
		{
			SetSizeEmpty(&activity->titleSize);
			return FALSE;
		}
		hdc = windowDC;
	}
	else 
		windowDC = NULL;

	prevFont = SelectFont(hdc, self->activityFont);
	
	SetRect(&rect, 0, 0, self->activityMetrics.titleWidth, self->activityMetrics.titleHeight);
	result = DrawText(hdc, activity->title, -1, &rect, 
				DT_CALCRECT | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL | DT_WORD_ELLIPSIS);
	
	if (FALSE == result)
		SetSizeEmpty(&activity->titleSize);
	else
	{
		TEXTMETRIC textMetrics;
		if (FALSE == GetTextMetrics(hdc, &textMetrics))
			ZeroMemory(&textMetrics, sizeof(textMetrics));

		if (rect.right > self->activityMetrics.titleWidth)
			rect.right = self->activityMetrics.titleWidth;
		if (rect.bottom > self->activityMetrics.titleHeight)
		{					
			textMetrics.tmHeight = self->activityMetrics.fontHeight;
			rect.bottom = (self->activityMetrics.titleHeight/textMetrics.tmHeight)*textMetrics.tmHeight;
		}

		activity->titleSize.cx = rect.right + textMetrics.tmAveCharWidth/2;
		activity->titleSize.cy = rect.bottom;
	}
	
	SelectFont(hdc, prevFont);
	if (NULL != windowDC)
		ReleaseDC(NULL, windowDC);

	return result;
}

static BOOL
ListWidget_GetItemActivityWorkRect(ListWidget *self, HDC hdc, 
								   ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{
	ListWidgetActivityMetric *activityMetrics;
	long length;

	if (FALSE == ListWidget_GetItemActivityRect(self, item, metrics, rect))
		return FALSE;

	if (0 == item->activity->titleSize.cy && 
		FALSE == ListWidget_CalcItemActivityTitleSize(self, hdc, item->activity))
	{
		return FALSE;
	}

	activityMetrics = &self->activityMetrics;

	length = 0;
	if (0 != activityMetrics->progressWidth)
	{
		if (0 != length)
			length += activityMetrics->spacing;
		length += activityMetrics->progressWidth;
	}
	
	if (0 != item->activity->titleSize.cx)
	{
		if (0 != length)
			length += activityMetrics->spacing;
		length += item->activity->titleSize.cx;
	}

	if (0 != activityMetrics->percentWidth)
	{
		if (0 != length)
			length += activityMetrics->spacing;
		length += activityMetrics->percentWidth;
	}

	rect->top += activityMetrics->offsetTop;
	rect->bottom -= activityMetrics->offsetBottom;

	rect->left += activityMetrics->offsetLeft;
	rect->right -= activityMetrics->offsetRight;

	rect->left += ((rect->right - rect->left) - length)/2;
	rect->right = rect->left + length;

	return TRUE;
}


BOOL
ListWidget_GetItemActivityRect(ListWidget *self, ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{
	if (NULL == self || 
		NULL == item ||
		NULL == metrics ||
		NULL == rect)
	{
		return FALSE;
	}
	
	rect->bottom = item->rect.top + self->imageSize.cy /*+ metrics.imageOffsetBottom*/;
	rect->bottom += metrics->offsetTop + metrics->imageOffsetTop;
	rect->top = rect->bottom - self->activityMetrics.height;
		
	rect->left = item->rect.left + (self->itemWidth - self->activityMetrics.width)/2;
	rect->right = rect->left + self->activityMetrics.width;

	return TRUE;
}

BOOL
ListWidget_GetItemActivityProgressRect(ListWidget *self, HDC hdc,
									   ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{
	if (0 == self->activityMetrics.progressWidth || 
		FALSE == ListWidget_GetItemActivityWorkRect(self, hdc, item, metrics, rect))
	{
		return FALSE;
	}
		
	rect->right = rect->left + self->activityMetrics.progressWidth;
	rect->top +=((rect->bottom - rect->top) - self->activityMetrics.progressHeight)/2;
	rect->bottom = rect->top + self->activityMetrics.progressHeight;
	return TRUE;
}

BOOL
ListWidget_GetItemActivityPercentRect(ListWidget *self, HDC hdc,
									  ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{
	if (0 == self->activityMetrics.percentWidth ||
		FALSE == ListWidget_GetItemActivityWorkRect(self, hdc, item, metrics, rect))
	{
		return FALSE;
	}
			
	rect->left = rect->right - self->activityMetrics.percentWidth;
	rect->top += ((rect->bottom - rect->top) - self->activityMetrics.percentHeight)/2;
	rect->bottom = rect->top + self->activityMetrics.percentHeight;

	return TRUE;
}

BOOL
ListWidget_GetItemActivityTitleRect(ListWidget *self, HDC hdc, 
									ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{
	if (0 == self->activityMetrics.titleWidth ||
		FALSE == ListWidget_GetItemActivityWorkRect(self, hdc, item, metrics, rect))
	{
		return FALSE;
	}
	
	if (0 != self->activityMetrics.progressWidth)
		rect->left += self->activityMetrics.progressWidth + self->activityMetrics.spacing;
	
	rect->right = rect->left + item->activity->titleSize.cx;
		
	rect->top += ((rect->bottom - rect->top) - item->activity->titleSize.cy)/2;
	rect->bottom = rect->top + item->activity->titleSize.cy;

	return TRUE;
}

BOOL
ListWidget_InvalidateItemActivity(ListWidget *self, HWND hwnd, ListWidgetItem *item, ListWidgetActivityChange changes)
{	
	ListWidgetItemMetric metrics;
	WidgetStyle *style;
	POINT origin;
	RECT rect;
	BOOL invalidated;
	
	
	if (ListWidgetActivityChanged_Nothing == changes)
		return FALSE;

	style = WIDGET_GET_STYLE(hwnd);
		

	if (NULL == style || 
		FALSE == ListWidget_GetItemMetrics(style, &metrics))
	{
		return FALSE;
	}
	
	if (FALSE == ListWidget_GetViewOrigin(hwnd, &origin))
	{
		origin.x = 0;
		origin.y = 0;
	}

	invalidated = FALSE;

	if (0 != (ListWidgetActivityChanged_Percent & changes))
	{
		if (FALSE != ListWidget_GetItemActivityPercentRect(self, NULL, item, &metrics, &rect))
		{
			OffsetRect(&rect, origin.x, origin.y);
			if (FALSE != InvalidateRect(hwnd, &rect, FALSE))
				invalidated = TRUE;
		}
	}

	if (0 != (ListWidgetActivityChanged_Title & changes))
	{
		if (FALSE != ListWidget_GetItemActivityTitleRect(self, NULL, item, &metrics, &rect))
		{
			OffsetRect(&rect, origin.x, origin.y);
			if (FALSE != InvalidateRect(hwnd, &rect, FALSE))
				invalidated = TRUE;
		}
	}

	return invalidated;
}

BOOL
ListWidget_InvalidateItemImage(ListWidget *self, HWND hwnd, ListWidgetItem *item)
{	
	ListWidgetItemMetric metrics;
	WidgetStyle *style;
	POINT origin;
	RECT rect;
	
	style = WIDGET_GET_STYLE(hwnd);
		
	if (NULL == style || 
		FALSE == ListWidget_GetItemMetrics(style, &metrics))
	{
		return FALSE;
	}
			
	if (FALSE == ListWidget_GetItemImageRect(self, item, &metrics, &rect))
		return FALSE;

	if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
		OffsetRect(&rect, origin.x, origin.y);

	return InvalidateRect(hwnd, &rect, FALSE);
}


BOOL
ListWidget_GetItemSpacebarRect(ListWidget *self, ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{
	if (NULL == item || NULL == rect)
		return FALSE;

	if (0 == item->spaceTotal || 
		FALSE == CopyRect(rect, &item->rect))
	{
		return FALSE;
	}
		
	if (NULL != metrics)
	{
		rect->left += metrics->offsetLeft;
		rect->top += metrics->offsetTop + metrics->imageOffsetTop + 
					self->imageSize.cy + metrics->imageOffsetBottom + 
					metrics->spacebarOffsetTop;
		rect->right -= metrics->offsetRight;
		rect->bottom = rect->top + metrics->spacebarHeight;
	}

	return TRUE;
}

BOOL
ListWidget_GetItemTitleRect(ListWidget *self, ListWidgetItem *item, ListWidgetItemMetric *metrics, BOOL exactSize, RECT *rect)
{
	if (NULL == item || NULL == rect)
		return FALSE;

	if (FALSE == CopyRect(rect, &item->rect))
	{
		return FALSE;
	}
		
	if (NULL != metrics)
	{
		rect->left += metrics->offsetLeft;
		rect->top += metrics->offsetTop + metrics->imageOffsetTop + 
					self->imageSize.cy + metrics->imageOffsetBottom;

		if (0 != item->spaceTotal)
			rect->top += metrics->spacebarOffsetTop + metrics->spacebarHeight;

		rect->top += metrics->titleOffsetTop;
		rect->right -= metrics->offsetRight;
		rect->bottom -= metrics->offsetBottom;
	}

	if (-1 != item->titleSize.cy)
	{
		long max;

		if (FALSE != exactSize)
		{
			max = rect->right - rect->left;
			if (max > item->titleSize.cx)
			{
				rect->left += (max - item->titleSize.cx)/2;
				rect->right = rect->left + item->titleSize.cx;
			}
		}
		
		max = rect->top + item->titleSize.cy;
		if (rect->bottom > max)
			rect->bottom = max;
	}

	return TRUE;
}

BOOL
ListWidget_GetItemConnectionRect(ListWidget *self, ListWidgetItem *item, ListWidgetItemMetric *metrics, RECT *rect)
{
	if (NULL == item || NULL == rect)
		return FALSE;

	if (NULL == metrics)
		return FALSE;
	
	SetRect(rect, 0, 0, self->connectionSize.cx, self->connectionSize.cy);
	OffsetRect(rect, 
			item->rect.right - metrics->offsetRight - metrics->imageOffsetRight - rect->right - 2, 
			item->rect.top + metrics->offsetTop + metrics->imageOffsetTop + self->imageSize.cy - rect->bottom - 2);

	if (rect->left < (item->rect.left + metrics->offsetLeft) ||
		rect->top < (item->rect.top + metrics->offsetTop))
	{
		return FALSE;
	}

	return TRUE;
}

ListWidgetItemPart
ListWidget_GetItemPartFromPoint(ListWidget *self,  ListWidgetItem *item, ListWidgetItemMetric *metrics, 
								POINT pt, ListWidgetItemPart mask, RECT *partRect)
{
	RECT rect;

	if (NULL == self ||
		NULL == item ||
		NULL == metrics)
	{
		if (NULL != partRect)
			SetRectEmpty(partRect);

		return ListWidgetItemPart_None;
	}

	
	
	if (NULL != item->activity && 
		FALSE != ListWidget_GetItemActivityRect(self, item, metrics, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		if (0 != (ListWidgetItemPart_Activity & mask))
		{
			if (NULL != partRect)
				CopyRect(partRect, &rect);

			return ListWidgetItemPart_Activity;
		}
		
		mask &= ~(ListWidgetItemPart_Command | ListWidgetItemPart_Connection);
	}

	if (0 != (ListWidgetItemPart_Command & mask) &&
		FALSE != ListWidgetItem_IsInteractive(item))
	{

		size_t index = self->commandsCount;
		while(index--)
		{
			if (FALSE != ListWidget_GetCommandRect(self->commands[index], &rect))
			{
				OffsetRect(&rect, item->rect.left, item->rect.top);
				if (FALSE != PtInRect(&rect, pt))
				{					
					if (FALSE == ListWidget_GetCommandDisabled(self->commands[index]))
					{
						if (NULL != partRect)
							CopyRect(partRect, &rect);

						return ListWidgetItemPart_Command;
					}
					break;
				}
			}
		}
	}

	if (0 != (ListWidgetItemPart_Connection & mask) &&
		FALSE != ListWidget_GetItemConnectionRect(self, item, metrics, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		if (NULL != partRect)
			CopyRect(partRect, &rect);
		return ListWidgetItemPart_Connection;
	}

	if (0 != (ListWidgetItemPart_Image & mask) &&
		FALSE != ListWidget_GetItemImageRect(self, item, metrics, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		if (NULL != partRect)
			CopyRect(partRect, &rect);
		return ListWidgetItemPart_Image;
	}

	if (0 != (ListWidgetItemPart_Frame & mask) &&
		FALSE != ListWidget_GetItemFrameRect(self, item, metrics, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		if (NULL != partRect)
			CopyRect(partRect, &rect);
		return ListWidgetItemPart_Frame;
	}
	

	if (0 != (ListWidgetItemPart_Spacebar & mask) &&
		FALSE != ListWidget_GetItemSpacebarRect(self, item, metrics, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		if (NULL != partRect)
			CopyRect(partRect, &rect);
		return ListWidgetItemPart_Spacebar;
	}

	if (0 != (ListWidgetItemPart_Title & mask) &&
		FALSE != ListWidget_GetItemTitleRect(self, item, metrics, FALSE, &rect) &&
		FALSE != PtInRect(&rect, pt))
	{
		if (NULL != partRect)
			CopyRect(partRect, &rect);
		return ListWidgetItemPart_Title;
	}

	return ListWidgetItemPart_None;
}



static BOOL 
ListWidget_FilterItemTitle(wchar_t *buffer, size_t bufferMax)
{
	size_t read, write;
			
	for(read = 0, write = 0;; read++)
	{
		if (read == bufferMax)
			return FALSE;

		if (L'\r' == buffer[read])
			continue;

		if (L'\n' == buffer[read] || 
			L'\t' == buffer[read] ||
			L'\b' == buffer[read])
		{
			buffer[write] = L' ';
		}
		
		buffer[write] = buffer[read];
		if (L'\0' == buffer[read])
			break;

		write++;
	}

	return TRUE;
}

static HRESULT
ListWidget_GetDeviceStatus(ifc_device *device, wchar_t *buffer, size_t bufferMax)
{
	HRESULT hr;
	ifc_deviceactivity *activity;

	if(NULL == buffer)
		return E_POINTER;

	buffer[0] = L'\0';
	if (NULL == device)
		return S_OK;

	hr = device->GetActivity(&activity);
	if (S_OK == hr && NULL != activity)
	{
		hr = activity->GetStatus(buffer, bufferMax);
		if (FAILED(hr) || L'\0' == buffer[0])
			hr = activity->GetDisplayName(buffer, bufferMax);

		activity->Release();
	}

	if (FAILED(hr) || L'\0' == buffer[0])
		hr = device->GetStatus(buffer, bufferMax);

	if (E_NOTIMPL == hr)
	{
		hr = S_OK;
		buffer[0] = L'\0';
	}
	
	return hr;

}
BOOL
ListWidget_FormatItemCommandTip(ListWidget *self, ListWidgetItem *item, const RECT *commandRect, wchar_t *buffer, size_t bufferMax)
{
	size_t index;
	RECT rect;

	if (NULL == self)
		return FALSE;

	for(index = 0; index < self->commandsCount; index++)
	{
		ListWidgetCommand *command = self->commands[index];
		if (FALSE != ListWidget_GetCommandRect(command, &rect) &&
			FALSE != EqualRect(&rect, commandRect))
		{
			const wchar_t *value;
			wchar_t *cursor; 
			size_t remaining;

			cursor = buffer;
			remaining = bufferMax;

			value = ListWidget_GetCommandTitle(command);
			if (FALSE == IS_STRING_EMPTY(value))
				StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

			value = ListWidget_GetCommandDescription(command);
			if (FALSE == IS_STRING_EMPTY(value))
			{
				if (cursor != buffer)
					StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
				StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
			}

			return (cursor != buffer);
		}
	}

	return FALSE;
}

BOOL
ListWidget_FormatItemTip(ListWidget *self, ListWidgetItem *item, wchar_t *buffer, size_t bufferMax)
{
	ifc_device *device;
	ifc_devicetype *type;
	ifc_deviceconnection *connection;
	wchar_t value[1024], valueName[512], *cursor; 
	size_t remaining;
	uint64_t totalSpace, usedSpace;

	if (NULL == item ||
		NULL == WASABI_API_DEVICES || 
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return FALSE;
	}

	cursor = buffer;
	remaining = bufferMax;

	if (FALSE != ListWidgetItem_IsTextTruncated(item))
	{
		if (SUCCEEDED(device->GetDisplayName(value, ARRAYSIZE(value))) && 
			L'\0' != value[0])
		{
			ListWidget_FilterItemTitle(value, ARRAYSIZE(value));
			StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}
	}

	if (SUCCEEDED(device->GetModel(value, ARRAYSIZE(value))) && 
		L'\0' != value[0])
	{
		if (cursor != buffer)
			StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

		WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_MODEL_SHORT, valueName, ARRAYSIZE(valueName));
		HRESULT hr;
		if (L'\0' != valueName[0])
			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);
		else
			hr = S_OK;
		
		if (SUCCEEDED(hr))
			StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
	}

	if (NULL != WASABI_API_DEVICES &&
		S_OK == WASABI_API_DEVICES->TypeFind(device->GetType(), &type))
	{
		const char* typeStr = device->GetDisplayType();
		if (typeStr && *typeStr)
		{
			if (cursor != buffer)
				StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

			WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_TYPE_SHORT, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);

			StringCchCopyEx(cursor, remaining, AutoWide(typeStr), &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}
		else
		{
			if (SUCCEEDED(type->GetDisplayName(value, ARRAYSIZE(value))) &&
				L'\0' != value[0])
			{
				if (cursor != buffer)
					StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

				WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_TYPE_SHORT, valueName, ARRAYSIZE(valueName));
				if (L'\0' != valueName[0])
					StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);

				StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
			}
		}
		type->Release();
	}

	if (NULL != WASABI_API_DEVICES &&
		S_OK == WASABI_API_DEVICES->ConnectionFind(device->GetConnection(), &connection))
	{
		if (SUCCEEDED(connection->GetDisplayName(value, ARRAYSIZE(value))) &&
			L'\0' != value[0])
		{
			if (cursor != buffer)
				StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

			WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_CONNECTION_SHORT, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);
			
			StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}
		connection->Release();
	}

	if (FAILED(device->GetTotalSpace(&totalSpace)) || 
		0 == totalSpace)
	{
		totalSpace = ((uint64_t)-1);
	}

	if (FAILED(device->GetUsedSpace(&usedSpace)))
		usedSpace = ((uint64_t)-1);
	else if (((uint64_t)-1) != totalSpace && usedSpace > totalSpace)
		usedSpace = totalSpace;

	if (((uint64_t)-1) != totalSpace && ((uint64_t)-1) != usedSpace)
	{
		if (NULL != WASABI_API_LNG->FormattedSizeString(value, ARRAYSIZE(value), totalSpace - usedSpace))
		{
					
			WASABI_API_LNGSTRINGW_BUF(IDS_FREE_SPACE, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
			{
				if (cursor != buffer)
					StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: %s", 
						valueName, value);
			}
		}
	}

	if (((uint64_t)-1) != totalSpace)
	{
		if (NULL != WASABI_API_LNG->FormattedSizeString(value, ARRAYSIZE(value), totalSpace))
		{
					
			WASABI_API_LNGSTRINGW_BUF(IDS_TOTAL_SPACE, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
			{
				if (cursor != buffer)
					StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: %s", 
						valueName, value);
			}
		}
	}

	// status
	/*if (SUCCEEDED(ListWidget_GetDeviceStatus(device, value, ARRAYSIZE(value))) && 
		L'\0' != value[0])
	{
		if (cursor != buffer)
			StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

		WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_STATUS_SHORT, valueName, ARRAYSIZE(valueName));
		if (L'\0' != valueName[0])
			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);
		else
			hr = S_OK;
		
		if (SUCCEEDED(hr))
			hr = StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
	}
	*/

	device->Release();
	return (cursor != buffer);
}

BOOL
ListWidget_FormatItemTitleTip(ListWidget *self, ListWidgetItem *item, wchar_t *buffer, size_t bufferMax)
{
	BOOL result;
	ifc_device *device;

	if (NULL == item ||
		FALSE == ListWidgetItem_IsTextTruncated(item) ||
		NULL == WASABI_API_DEVICES || 
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return FALSE;
	}

	if (SUCCEEDED(device->GetDisplayName(buffer, bufferMax)))
	{
		ListWidget_FilterItemTitle(buffer, bufferMax);
		result = TRUE;
	}
	else
		result = FALSE;
	
	device->Release();

	return result;
}

BOOL
ListWidget_FormatItemSpaceTip(ListWidget *self, ListWidgetItem *item, wchar_t *buffer, size_t bufferMax)
{
	ifc_device *device;

	wchar_t value[1024], valueName[512], *cursor; 
	size_t remaining;
	uint64_t totalSpace, usedSpace;

	if (NULL == item ||
		NULL == WASABI_API_DEVICES || 
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return FALSE;
	}

	cursor = buffer;
	remaining = bufferMax;

	if (FAILED(device->GetTotalSpace(&totalSpace)) || 
		0 == totalSpace)
	{
		totalSpace = ((uint64_t)-1);
	}

	if (FAILED(device->GetUsedSpace(&usedSpace)))
		usedSpace = ((uint64_t)-1);
	else if (((uint64_t)-1) != totalSpace && usedSpace > totalSpace)
		usedSpace = totalSpace;

	if (((uint64_t)-1) != usedSpace)
	{
		if (NULL != WASABI_API_LNG->FormattedSizeString(value, ARRAYSIZE(value), usedSpace))
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_USED_SPACE, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
			{
				if (cursor != buffer)
					StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: %s", 
									valueName, value);
			}
		}
	}

	if (((uint64_t)-1) != totalSpace && ((uint64_t)-1) != usedSpace)
	{
		if (NULL != WASABI_API_LNG->FormattedSizeString(value, ARRAYSIZE(value), totalSpace - usedSpace))
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_FREE_SPACE, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
			{
				if (cursor != buffer)
					StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: %s", 
						valueName, value);
			}
		}
	}

	if (((uint64_t)-1) != totalSpace)
	{
		if (NULL != WASABI_API_LNG->FormattedSizeString(value, ARRAYSIZE(value), totalSpace))
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_TOTAL_SPACE, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
			{
				if (cursor != buffer)
					StringCchCopyEx(cursor, remaining, L"\r\n", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: %s", 
						valueName, value);
			}
		}
	}

	device->Release();
	return (cursor != buffer);
}

BOOL
ListWidget_FormatItemStatus(ListWidget *self, ListWidgetItem *item, wchar_t *buffer, size_t bufferMax)
{
	ifc_device *device;
	ifc_devicetype *type;
	ifc_deviceconnection *connection;

	HRESULT hr;
	wchar_t value[512], valueName[128], *cursor; 
	size_t remaining;

	if (NULL == item ||
		NULL == WASABI_API_DEVICES || 
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return FALSE;
	}

	hr = S_OK;
	cursor = buffer;
	remaining = bufferMax;

	if (FALSE != ListWidgetItem_IsTextTruncated(item))
	{
		if (SUCCEEDED(device->GetDisplayName(value, ARRAYSIZE(value))) && 
			L'\0' != value[0])
		{
			ListWidget_FilterItemTitle(value, ARRAYSIZE(value));
			hr = StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}
	}

	if (cursor == buffer &&
		SUCCEEDED(device->GetModel(value, ARRAYSIZE(value))) && 
		L'\0' != value[0])
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_MODEL_SHORT, valueName, ARRAYSIZE(valueName));
		if (L'\0' != valueName[0])
			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);

		if (SUCCEEDED(hr))
			hr = StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
	}

	if (cursor == buffer &&
		NULL != WASABI_API_DEVICES &&
		S_OK == WASABI_API_DEVICES->TypeFind(device->GetType(), &type))
		{
		if (SUCCEEDED(type->GetDisplayName(value, ARRAYSIZE(value))) &&
			L'\0' != value[0])
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_TYPE_SHORT, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
				hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);

			if (SUCCEEDED(hr))
				hr = StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}

		type->Release();
	}
	else
	{
		const char* typeStr = device->GetDisplayType();
		if (typeStr && *typeStr)
		{
			if (cursor != buffer)
				hr = StringCchCopyEx(cursor, remaining, L", ", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

			WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_TYPE_SHORT, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
				hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);

			if (SUCCEEDED(hr))
				StringCchCopyEx(cursor, remaining, AutoWide(typeStr), &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}
	}

	if (NULL != WASABI_API_DEVICES &&
		S_OK == WASABI_API_DEVICES->ConnectionFind(device->GetConnection(), &connection))
	{
		if (SUCCEEDED(connection->GetDisplayName(value, ARRAYSIZE(value))) &&
			L'\0' != value[0])
		{
			if (cursor != buffer)
				StringCchCopyEx(cursor, remaining, L", ", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

			WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_CONNECTION_SHORT, valueName, ARRAYSIZE(valueName));
			if (L'\0' != valueName[0])
				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, L"%s: ", valueName);
			
			StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
		}
		connection->Release();
	}

	if (cursor == buffer &&
		SUCCEEDED(device->GetDisplayName(value, ARRAYSIZE(value))))
	{
		ListWidget_FilterItemTitle(value, ARRAYSIZE(value));
		StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
	}

	if (SUCCEEDED(ListWidget_GetDeviceStatus(device, value, ARRAYSIZE(value))) && 
			L'\0' != value[0])
	{		
		if (cursor != buffer)
			StringCchCopyEx(cursor, remaining, L", ", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);

		StringCchCopyEx(cursor, remaining, value, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
	}

	device->Release();

	return (cursor != buffer);
}

BOOL
ListWidget_FormatItemSpaceStatus(ListWidget *self, ListWidgetItem *item, wchar_t *buffer, size_t bufferMax)
{
	ifc_device *device;

	wchar_t *cursor; 
	size_t remaining;
	uint64_t totalSpace, usedSpace;

	if (NULL == item ||
		NULL == WASABI_API_DEVICES || 
		S_OK != WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		return FALSE;
	}

	cursor = buffer;
	remaining = bufferMax;

	if (FAILED(device->GetTotalSpace(&totalSpace)) || 
		0 == totalSpace)
	{
		totalSpace = ((uint64_t)-1);
	}

	if (FAILED(device->GetUsedSpace(&usedSpace)))
		usedSpace = ((uint64_t)-1);
	else if (((uint64_t)-1) != totalSpace && usedSpace > totalSpace)
		usedSpace = totalSpace;

	if (((uint64_t)-1) != totalSpace)
	{
		if (((uint64_t)-1) != usedSpace)
		{
			wchar_t value1[64] = {0}, value2[64] = {0};
			if (NULL != WASABI_API_LNG->FormattedSizeString(value1, ARRAYSIZE(value1), totalSpace - usedSpace) && 
				NULL != WASABI_API_LNG->FormattedSizeString(value2, ARRAYSIZE(value2), totalSpace))
			{
				wchar_t format[128] = {0};
				WASABI_API_LNGSTRINGW_BUF(IDS_STATUS_SPACE_TEMPLATE, format, ARRAYSIZE(format));
				StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
									format, value1, value2);
			}
		}
		else
		{
			wchar_t value[64] = {0};
			if (NULL != WASABI_API_LNG->FormattedSizeString(value, ARRAYSIZE(value), totalSpace))
			{
				wchar_t valueName[128] = {0};
				WASABI_API_LNGSTRINGW_BUF(IDS_TOTAL_SPACE, valueName, ARRAYSIZE(valueName));
				if (L'\0' != valueName[0])
				{
					StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
									L"%s %s", value, valueName);

				}
			}
		}
	}

	device->Release();

	return (cursor != buffer);
}

static void CALLBACK
ListWidget_EndTitleEditCb(HWND editorWindow, BOOL canceled, const wchar_t *text, void *user)
{
	HWND hwnd;
	ListWidget *self;
	char *itemName;

	hwnd = GetAncestor(editorWindow, GA_PARENT);
	self = WIDGET_GET_SELF(hwnd, ListWidget);
	
	itemName = (char*)user;

	if (NULL != self)
	{
		ListWidgetItem *item;

		if (self->titleEditor == editorWindow)
			self->titleEditor = NULL;

		item = ListWidget_FindItem(self, itemName, NULL, NULL);
		if (NULL != item)
		{
			ListWidgetItemMetric metrics;
			WidgetStyle *style;
			POINT origin;
			RECT rect;

			ListWidgetItem_UnsetTextEdited(item);

			style = WIDGET_GET_STYLE(hwnd);
		
			if (NULL != style &&
				FALSE != ListWidget_GetItemMetrics(style, &metrics) &&
				FALSE != ListWidget_GetItemTitleRect(self, item, &metrics, FALSE, &rect))
			{
				if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
						OffsetRect(&rect, origin.x, origin.y);

				InvalidateRect(hwnd, &rect, FALSE);
			}
		}			
		
		if (FALSE == canceled)
		{
			ifc_device *device;
			if (NULL != WASABI_API_DEVICES &&
				S_OK == WASABI_API_DEVICES->DeviceFind(itemName, &device))
			{				
				wchar_t buffer[1024] = {0};

				if (FAILED(device->GetDisplayName(buffer, ARRAYSIZE(buffer))) ||
					CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT, 0, buffer, -1, text, -1))
				{
					HRESULT hr;

					hr = device->SetDisplayName(text);
					
					if (FAILED(hr))
					{
						wchar_t title[256] = {0}, message[1024] = {0};

						WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGEBOX_TITLE, title, ARRAYSIZE(title));
						WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGE_UNABLE_TO_RENAME, message, ARRAYSIZE(message));
			
						MessageBox(hwnd, message, title, MB_OK | MB_ICONERROR);
					}
				}

				device->Release();
			}
		}
	}
	
	EMBEDDEDEDITOR_SET_USER_DATA(editorWindow, NULL);
	AnsiString_Free(itemName);
}

HWND
ListWidget_BeginItemTitleEdit(ListWidget *self, HWND hwnd, ListWidgetItem *item)
{
	RECT rect;
	WidgetStyle *style;
	ListWidgetItemMetric metrics;
	HWND editor;
	POINT origin;
	unsigned long editorStyleEx, editorStyle;
	char *itemName;
	ifc_device * device;
	BOOL blockEditor;


	if (NULL == self || NULL == item)
		return NULL;

	style = WIDGET_GET_STYLE(hwnd);
	if (NULL == style)
		return NULL;

	if (NULL != WASABI_API_DEVICES &&
		S_OK == WASABI_API_DEVICES->DeviceFind(item->name, &device))
	{
		blockEditor = (FALSE == DeviceCommand_GetEnabled(device, "rename", 
								DeviceCommandContext_ViewMenu));
		device->Release();
	}
	else
		blockEditor = TRUE;
		
	if (FALSE != blockEditor)
		return NULL;

	if (FALSE == ListWidget_GetItemMetrics(style, &metrics))
		return NULL;

	if (FALSE == ListWidget_GetItemTitleRect(self, item, &metrics, FALSE, &rect))
		return NULL;

	if (FALSE != ListWidget_GetViewOrigin(hwnd, &origin))
		OffsetRect(&rect, origin.x, origin.y);
	
	editorStyleEx = WS_EX_CLIENTEDGE;
	editorStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
					ES_CENTER | ES_NOHIDESEL | ES_MULTILINE | ES_AUTOVSCROLL;
	
	EmbeddedEditor_AdjustWindowRectEx(&rect, editorStyleEx, editorStyle);

	editor = CreateWindowEx(editorStyleEx, WC_EDIT, item->title, editorStyle,
							rect.left, rect.top, 0, 0, 
							hwnd, NULL, NULL, 0L);
	if (NULL == editor)
		return NULL;


	itemName = AnsiString_Duplicate(item->name);
	if (FALSE == EmbeddedEditor_Attach(editor, ListWidget_EndTitleEditCb, itemName))
	{
		AnsiString_Free(itemName);
		DestroyWindow(editor);
		return NULL;
	}

	ListWidgetItem_SetTextEdited(item);

	EMBEDDEDEDITOR_SET_ANCHOR_POINT(editor, rect.left, rect.top);
	EMBEDDEDEDITOR_SET_MAX_SIZE(editor, RECTWIDTH(rect), 0);

	
	SendMessage(editor, WM_SETFONT, (WPARAM)WIDGETSTYLE_TEXT_FONT(style), 0L);

	ListWidget_UpdateTitleEditorColors(editor, style);

	SendMessage(editor, EM_SETSEL, 0, -1);
	ShowWindow(editor, SW_SHOW);
	SetFocus(editor);
	
	return editor;
}


int
ListWidget_CompareItemPos(ListWidget *self, ListWidgetItem *item1, ListWidgetItem *item2)
{
	size_t iCategory1, iGroup1, iItem1;
	size_t iCategory2, iGroup2, iItem2;
	
	if (FALSE == ListWidget_FindItemPos(self, item1, &iCategory1, &iGroup1, &iItem1) ||
		FALSE == ListWidget_FindItemPos(self, item2, &iCategory2, &iGroup2, &iItem2))
	{
		return _NLSCMPERROR;
	}

	if (iCategory1 != iCategory2)
		return (int)(iCategory1 - iCategory2);

	if (iGroup1 != iGroup2)
		return (int)(iGroup1 - iGroup2);

	return (int)(iItem1 - iItem2);
}

BOOL
ListWidget_GetViewItemPos(HWND hwnd, ListWidgetItem *item, POINT *pt)
{
	if (NULL == hwnd ||
		NULL == item ||
		NULL == pt)
	{
		return FALSE;
	}

	if (FALSE == ListWidget_GetViewOrigin(hwnd, pt))
	{
		pt->x = 0;
		pt->y = 0;
	}
	
	pt->x = item->rect.left - pt->x;
	pt->y = item->rect.top - pt->y;

	return TRUE;
}