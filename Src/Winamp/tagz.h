#ifndef NULLSOFT_WINAMP_TAGZ_H
#define NULLSOFT_WINAMP_TAGZ_H

#include "wa_ipc.h"
void FormatTitle(waFormatTitle *format);
void FormatTitleExtended(waFormatTitleExtended *format);
int FormatTitle(waHookTitleStructW *hts);

namespace Winamp
{
	wchar_t *GetTag(const wchar_t *tag, const wchar_t *filename); // for simple tags
	wchar_t *GetExtendedTag(const wchar_t *tag, const wchar_t *filename); //return 0 if not found
}

#endif