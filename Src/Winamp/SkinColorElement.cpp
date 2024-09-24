#include "api.h"
#include "SkinColorElement.h"
#include "PaletteManager.h"

SkinItem *SkinColorElement::getAncestor()
{
	return WASABI_API_PALETTE->getColorAncestor(this);
}

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS SkinColorElement
START_DISPATCH;
CB(SKINITEM_GETXMLROOTPATH, getXmlRootPath);
CB(SKINITEM_GETNAME, getName);
CB(SKINITEM_GETPARAMS, getParams);
CB(SKINITEM_GETSKINPARTID, getSkinPartId);
CB(SKINITEM_GETANCESTOR, getAncestor);
END_DISPATCH;
