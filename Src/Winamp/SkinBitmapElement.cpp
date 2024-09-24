#include "api.h"
#include "SkinBitmapElement.h"
#include "PaletteManager.h"

SkinBitmapElement::SkinBitmapElement(const wchar_t *_id, const wchar_t *_filename, const wchar_t *_rootpath, int _x, int _y, int _w, int _h, ifc_xmlreaderparams *pars, int script_id, int secondarycounter, const wchar_t *colorgrp)
		: filename(_filename), rootpath(_rootpath), x(_x), y(_y), w(_w), h(_h),
		scriptid(script_id), seccount(secondarycounter), colorgroup(colorgrp),
		region(NULL)
{
	id = _id;

	if (pars)
	{
		for (size_t i = 0;i != pars->getNbItems();i++)
			params.addItem(pars->getItemName(i), pars->getItemValue(i));
	}
}

SkinBitmapElement::~SkinBitmapElement()
{
	if (region != NULL) WASABI_API_PALETTE->garbageCollectRegionServer(region);
	region = NULL;
}

SkinItem *SkinBitmapElement::getAncestor()
{
	return WASABI_API_PALETTE->getBitmapAncestor(this);
}
