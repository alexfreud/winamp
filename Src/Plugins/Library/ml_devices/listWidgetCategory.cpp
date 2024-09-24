#include "main.h"
#include "./listWidgetInternal.h"
#include <algorithm>

#define LISTWIDGETCATEGORY_OFFSET_LEFT_DLU			3
#define LISTWIDGETCATEGORY_OFFSET_TOP_DLU			0
#define LISTWIDGETCATEGORY_OFFSET_RIGHT_DLU			4
#define LISTWIDGETCATEGORY_OFFSET_BOTTOM_DLU		0
#define LISTWIDGETCATEGORY_LINE_OFFSET_TOP_DLU		0
#define LISTWIDGETCATEGORY_LINE_HEIGHT_DLU			1
#define LISTWIDGETCATEGORY_TITLE_OFFSET_LEFT_DLU	4
#define LISTWIDGETCATEGORY_MIN_HEIGHT_DLU			9
#define LISTWIDGETCATEGORY_ARROW_WIDTH_PX			8
#define LISTWIDGETCATEGORY_ARROW_HEIGHT_PX			8


ListWidgetCategory *
ListWidget_CreateCategory(const char *name, const wchar_t *title, BOOL collapsed)
{
	ListWidgetCategory *category;

	if (NULL == name)
		return NULL;

	category = new ListWidgetCategory();
	if(NULL == category)
		return NULL;
	
	category->name = AnsiString_Duplicate(name);
	category->title = String_Duplicate(title);
	category->collapsed = collapsed;
	category->countString = NULL;
	category->countWidth = -1;
	category->titleWidth = -1;
	category->emptyText = NULL;
	SetRect(&category->emptyTextRect, -1, -1, -1, -1);

	return category;
}

void 
ListWidget_DestroyCategory(ListWidgetCategory *category)
{
	size_t index;
	if (NULL == category)
		return;

	index = category->groups.size();
	while(index--)
	{
		ListWidget_DestroyGroup(category->groups[index]);
	}

	AnsiString_Free(category->name);
	String_Free(category->title);
	String_Free(category->countString);
	String_Free(category->emptyText);

	delete category;
}

BOOL
ListWidget_GetCategoryMetrics(WidgetStyle *style, ListWidgetCategoryMetric *metrics)
{
	long test;

	if (NULL == metrics || NULL == style)
		return FALSE;

	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->offsetLeft, style, LISTWIDGETCATEGORY_OFFSET_LEFT_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->offsetTop, style, LISTWIDGETCATEGORY_OFFSET_TOP_DLU, 1);
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->offsetRight, style, LISTWIDGETCATEGORY_OFFSET_RIGHT_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->offsetBottom, style, LISTWIDGETCATEGORY_OFFSET_BOTTOM_DLU, 1);
	
	WIDGETSTYLE_DLU_TO_HORZ_PX_MIN(metrics->titleOffsetLeft, style, LISTWIDGETCATEGORY_TITLE_OFFSET_LEFT_DLU, 2);

	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->lineOffsetTop, style, LISTWIDGETCATEGORY_LINE_OFFSET_TOP_DLU, 1);
	WIDGETSTYLE_DLU_TO_VERT_PX_MIN(metrics->minHeight, style, LISTWIDGETCATEGORY_MIN_HEIGHT_DLU, 1);

	#if (0 != LISTWIDGETCATEGORY_LINE_HEIGHT_DLU)
	{
		metrics->lineHeight = WIDGETSTYLE_DLU_TO_VERT_PX(style, LISTWIDGETCATEGORY_LINE_HEIGHT_DLU);
		metrics->lineHeight = metrics->lineHeight /2;
		if (0 == metrics->lineHeight)
			metrics->lineHeight = 1;
	}
	#endif


	metrics->iconWidth = LISTWIDGETCATEGORY_ARROW_WIDTH_PX;
	metrics->iconHeight = LISTWIDGETCATEGORY_ARROW_HEIGHT_PX;
	
	test = metrics->iconHeight + 
		   metrics->offsetTop + metrics->offsetBottom + 
		   metrics->lineHeight + metrics->lineOffsetTop;

	if (metrics->minHeight < test)
		metrics->minHeight = test;

	return TRUE;
}

ListWidgetCategory *
ListWidget_FindCategory(ListWidget *self, const char *name)
{
	if (NULL == self || NULL == name)
		return NULL;

	for (size_t iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		ListWidgetCategory *category = self->categories[iCategory];
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, -1, category->name, -1))
			return category;
	}
	return NULL;
}

ListWidgetCategory *
ListWidget_GetCategoryFromPoint(ListWidget *self, POINT point)
{
	size_t iCategory;
	ListWidgetCategory *category;
	
	if (NULL == self)
		return NULL;

	for (iCategory = 0; iCategory < self->categories.size(); iCategory++)
	{
		category = self->categories[iCategory];
		if (FALSE != PtInRect(&category->rect, point))
			return category;
	}
	return NULL;
}

BOOL 
ListWidget_ToggleCategory(ListWidgetCategory *category, HWND hwnd)
{
	if (NULL == category)
		return FALSE;

	category->collapsed = !category->collapsed;

	if (NULL != hwnd)
		ListWidget_UpdateLayout(hwnd, ListWidgetLayout_UpdateNow);

	return TRUE;
}

void
ListWidget_ResetCategoryCounter(ListWidgetCategory *category)
{
	if (NULL != category)
	{
		String_Free(category->countString);
		category->countString = NULL;
		category->countWidth = -1;
	}
}

static bool
ListWidget_GroupSortCb(const void *element1, const void *element2)
{
	ListWidgetGroup *group1;
	ListWidgetGroup *group2;
	int result;

	group1 = *(ListWidgetGroup**)element1;
	group2 = *(ListWidgetGroup**)element2;

	result = CompareString(LOCALE_USER_DEFAULT, 0, group1->title, -1, group2->title, -1);
	if (CSTR_EQUAL == result || 0 == result)
		result = CompareStringA(CSTR_INVARIANT, 0, group1->name, -1, group2->name, -1);
		
	return (result == CSTR_LESS_THAN);

}


void 
ListWidget_SortCategory(ListWidgetCategory *category)
{
	if (category->groups.size())
	{
		//qsort(category->groups.first(), category->groups.size(), sizeof(ListWidgetGroup**), ListWidget_GroupSortCb);
		std::sort(category->groups.begin(), category->groups.end(), ListWidget_GroupSortCb);
	}
}

BOOL
ListWidget_SetCategoryEmptyText(ListWidgetCategory *category, const wchar_t *text)
{
	if (NULL == category)
		return FALSE;

	String_Free(category->emptyText);
	category->emptyText = String_Duplicate(text);

	SetRect(&category->emptyTextRect, -1, -1, -1, -1);

	return TRUE;
}