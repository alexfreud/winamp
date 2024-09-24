/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/


#include "main.h"
#include "TagProvider.h"
#include "tagz.h"
/* --- TagProvider --- */

wchar_t *TagProvider::GetTag(const wchar_t *name, ifc_tagparams *parameters)
{
	const wchar_t *filename = (const wchar_t *)parameters->GetParameter(&filenameParameterID);
	if (!filename)
		return 0;

	wchar_t *tag = Winamp::GetTag(name, filename);
	if (tag == reinterpret_cast<wchar_t *>(-1))
		return 0;

	if (!tag)
		tag = Winamp::GetExtendedTag(name, filename);

	if (tag == reinterpret_cast<wchar_t *>(-1))
		return 0;
	return tag;
}

void TagProvider::FreeTag(wchar_t *tag)
{
	free(tag);	
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS TagProvider
START_DISPATCH;
CB(IFC_TAGPROVIDER_GET_TAG, GetTag);
VCB(IFC_TAGPROVIDER_FREE_TAG, FreeTag);
END_DISPATCH;
#undef CBCLASS