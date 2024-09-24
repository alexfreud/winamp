#include <precomp.h>
#include "SkinElementAlias.h"
#include <api/skin/PaletteManager.h>

SkinItem *SkinElementAlias::getAncestor()
{
	return paletteManager.getAliasAncestor(this);
}
