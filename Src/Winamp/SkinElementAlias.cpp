#include "api.h"
#include "SkinElementAlias.h"
#include "PaletteManager.h"

SkinItem *SkinElementAlias::getAncestor()
{
	return WASABI_API_PALETTE->getAliasAncestor(this);
}
