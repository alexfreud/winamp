#include "main.h"
#include "TagAlias.h"

struct TagAliases
{
	const wchar_t *winampTag;
	const wchar_t *wmaTag;
};

const TagAliases aliases[] =
    {
        {L"comment", g_wszWMDescription},
        {L"album", g_wszWMAlbumTitle},
        {L"genre", g_wszWMGenre},
        {L"year", g_wszWMYear},
        {L"track", g_wszWMTrackNumber},
        {L"artist", g_wszWMAuthor},
        {L"title", g_wszWMTitle},
        {L"copyright", g_wszWMCopyright},
        {L"composer", g_wszWMComposer},
        {L"albumartist", g_wszWMAlbumArtist},
        {L"bpm", g_wszWMBeatsPerMinute},
        {L"publisher", g_wszWMPublisher},
        {L"ISRC", g_wszWMISRC},
        {L"lyricist", g_wszWMWriter},
        {L"conductor", g_wszWMConductor},
        {L"tool", g_wszWMToolName},
        {L"encoder", g_wszWMEncodingSettings},
        {L"key", g_wszWMInitialKey},
        {L"mood", g_wszWMMood},
        {L"disc", g_wszWMPartOfSet},
				{L"height", g_wszWMVideoHeight},
				{L"width", g_wszWMVideoWidth},
				{L"category", g_wszWMCategory},
				{L"producer", g_wszWMProducer},
				{L"director", g_wszWMDirector},
				{L"fps", g_wszWMVideoFrameRate},
        {0, 0},
    };


const wchar_t *GetAlias(const wchar_t *tag)
{
	int i = 0;
	while (aliases[i].winampTag)
	{
		if (!lstrcmpiW(tag, aliases[i].winampTag))
			return aliases[i].wmaTag;
		i++;
	}
	return tag;
}

const wchar_t *GetAlias_rev(const wchar_t *tag)
{
	int i = 0;
	while (aliases[i].wmaTag)
	{
		if (!lstrcmpiW(tag, aliases[i].wmaTag))
			return aliases[i].winampTag;
		i++;
	}
	return tag;
}
