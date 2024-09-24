#include "main.h"
#define WA_DLG_IMPLEMENT
#include "wa_dlg.h"

const char *GetFontName()
{
	if (config_custom_plfont && *playlist_custom_font)
		return playlist_custom_font;
	if (!Skin_PLFont[0])
	{
		static char lang_font[128]; // benski> this is thread-safe because the language pack won't change (changing lang packs requires winamp restart)
		return getString(IDS_PLFONT, lang_font, 128);
	}
	return Skin_PLFont;
}

const wchar_t *GetFontNameW()
{
	if (config_custom_plfont && *playlist_custom_fontW)
		return playlist_custom_fontW;
	if (!Skin_PLFontW[0])
	{
		static wchar_t lang_fontW[128]; // benski> this is thread-safe because the language pack won't change (changing lang packs requires winamp restart)
		return getStringW(IDS_PLFONT, lang_fontW, 128);
	}
	return Skin_PLFontW;
}

int GetFontSize()
{
	return ScaleY(config_pe_fontsize);
}