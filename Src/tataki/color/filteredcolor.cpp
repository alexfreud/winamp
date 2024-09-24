#include "filteredcolor.h"
#include <tataki/api__tataki.h>
#include <bfc/bfc_assert.h>

static const int *skin_iterator=0;

FilteredColor::FilteredColor(ARGB32 _color, const wchar_t *colorgroupname)
{
	group=0;
	color = _color;
	filteredcolor = _color;
	if (colorgroupname)
		group = _wcsdup(colorgroupname);
	skin_iterator = NULL;
	latest_iteration = -1;
	need_filter = 1;
}

FilteredColor::~FilteredColor()
{
	free(group);
}

void FilteredColor::setColor(ARGB32 _color)
{
	color = _color;
	filteredcolor = color;
}

void FilteredColor::setColorGroup(const wchar_t *_group)
{
	free(group);
	if (_group)
		group = _wcsdup(_group);
	else
		group = 0;
	need_filter = 1;
}

ARGB32 FilteredColor::getColor()
{
	ensureFiltered();
	return filteredcolor;
}

ARGB32 *FilteredColor::getColorRef()
{
	if (!WASABI_API_SKIN)
		return 0;

	ensureFiltered();
	return &filteredcolor;
}

void FilteredColor::ensureFiltered()
{
	// fetch iterator pointer if necessary
	if (skin_iterator == NULL)
	{
		skin_iterator = WASABI_API_PALETTE->getSkinPartIteratorPtr();
		ASSERT(skin_iterator != NULL);
	}

	// see if we're current
	if (*skin_iterator != latest_iteration)
	{
		need_filter = 1;	// pointer now invalid, must re-get
		latest_iteration = *skin_iterator;	// and then we'll be current
	}

	if (need_filter && WASABI_API_SKIN)
	{
		filteredcolor = WASABI_API_SKIN->filterSkinColor(color, getColorName(), group);
		need_filter = 0;
	}

}

