#include "skinclr.h"
#include <bfc/assert.h>
#include <tataki/api__tataki.h>

static const int *skin_iterator = 0;

SkinColor::SkinColor(const wchar_t *_name, const wchar_t *colorgroup)
: FilteredColor(0, (colorgroup == NULL || !*colorgroup) ? L"Text" : colorgroup)
{
	name = 0;
	latest_iteration = -1;
	//CUT  skin_iterator = NULL;
	setElementName(_name);
	ovr_grp = colorgroup;
	dooverride = 0;
	color_override = 0;

}

SkinColor::~SkinColor()
{
	if (name) free(name);
}

ARGB32 SkinColor::v(ARGB32 defaultColor)
{
	if (!name || !*name) return defaultColor;

	if (!iteratorValid())
	{
		val = NULL;	// pointer now invalid, must re-get
		latest_iteration = *skin_iterator;	// and then we'll be current
		// new pointer please

		const wchar_t *grp = NULL;
		ARGB32 r;
		if (dooverride)
			r = color_override;
		else
			r = WASABI_API_PALETTE->getColorElement(name, &grp);
		if (ovr_grp == NULL && grp != NULL) 
			setColorGroup(grp);
		FilteredColor::setColor(r);
		val = getColorRef();
	}
	if (val == NULL) return defaultColor;

	return *val;
}

void SkinColor::setElementName(const wchar_t *_name)
{
	if (name) free(name);
	if (_name)
		name = _wcsdup(_name);
	else
		name = 0;
	val = NULL;
	latest_iteration = 0;
}

void SkinColor::setColor(ARGB32 c)
{
	dooverride = 1;
	color_override = c;
	FilteredColor::setColor(color_override);
}

int SkinColor::iteratorValid()
{
	// fetch iterator pointer if necessary
	if (skin_iterator == NULL)
	{
		skin_iterator = WASABI_API_PALETTE->getSkinPartIteratorPtr();
		ASSERT(skin_iterator != NULL);
	}

	// see if we're current
	return (*skin_iterator == latest_iteration);
}

const wchar_t *SkinColor::operator =(const wchar_t *name) { setElementName(name); return name;}
const wchar_t *SkinColor::getColorName() { return name; }

ARGB32 SkinColor::GetColor(const wchar_t *name, const wchar_t *group, ARGB32 defaultColor)
{
		const wchar_t *colorGroup = NULL;
		const ARGB32 *color = WASABI_API_PALETTE->getColorElementRef(name, &colorGroup);

		if (!color)
			return defaultColor;

		/* TODO: benski> if we ever add color themes to Classic, we'll need to change this */
		if (WASABI_API_SKIN)
		{
			if (group)
				colorGroup = group;
			
			if (!colorGroup)
				colorGroup = L"Text";

			return WASABI_API_SKIN->filterSkinColor(*color, name, colorGroup);
		}
		else
		{
			return *color;
		}
}

bool SkinColor::TryGetColor(ARGB32 *returned_color, const wchar_t *name, const wchar_t *group)
{
		const wchar_t *colorGroup = NULL;
		const ARGB32 *color = WASABI_API_PALETTE->getColorElementRef(name, &colorGroup);

		if (!color)
			return false;

		/* TODO: benski> if we ever add color themes to Classic, we'll need to change this */
		if (WASABI_API_SKIN)
		{
			if (group)
				colorGroup = group;
			
			if (!colorGroup)
				colorGroup = L"Text";

			*returned_color = WASABI_API_SKIN->filterSkinColor(*color, name, colorGroup);
		}
		else
		{
			*returned_color = *color;
		}
		return true;
}