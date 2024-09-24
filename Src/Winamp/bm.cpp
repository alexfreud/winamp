/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: 
 ** Created:
 **/
#include "main.h"
#include "../Plugins/Library/ml_bookmarks/bookmark.h"
#include <malloc.h>
#include "api.h"
#include "../nu/AutoCharFn.h"
#include "../nu/AutoChar.h"
#include <strsafe.h>

void Bookmark_additem(wchar_t *fn, wchar_t *ft)
{
	if ((!fn || fn && !*fn) || (!ft || ft && !*ft)) return;

	BookmarkWriter bookmarks;
	bookmarks.Open(BOOKMARKFILE);
	bookmarks.Write(AutoCharFn(fn), AutoCharFn(ft));
	bookmarks.Close();

	bookmarks.Open(BOOKMARKFILE8);
	bookmarks.Write(AutoChar(fn, CP_UTF8), AutoChar(ft, CP_UTF8));
	bookmarks.Close();

	size_t bufSize = wcslen(fn)+wcslen(ft)+3;
	wchar_t *buf=(wchar_t*)_malloca(bufSize*sizeof(wchar_t)), *buf2pass = buf;
	if (buf)
	{
		StringCchCopyExW(buf, bufSize, fn, &buf, &bufSize, 0);
		buf++; bufSize--;
		StringCchCopyW(buf, bufSize, ft);
		SendMessageW(hMainWindow,WM_WA_IPC,(WPARAM)buf2pass,IPC_ADDBOOKMARKW);
		AGAVE_API_STATS->IncrementStat(api_stats::BOOKMARK_COUNT);
		_freea(buf);
	}
}

void Bookmark_AddCommandline(wchar_t *lpszCmdParam)
{
	wchar_t *p = lpszCmdParam;
	if (p)
	{
		if (*p == L'\"') { p++; p[lstrlenW(p) - 1] = 0; }
		Bookmark_additem(p, PlayList_gettitle(p, 1));
	}
}

void WriteEscaped(FILE *fp, const char *str)
{
	// TODO: for speed optimization,
	// we should wait until we hit a special character 
	// and write out everything else so before it,
	// like how ASX loader does it
	while (str && *str)
	{
		switch(*str)
		{
			case '&':
				fputs("&amp;", fp);
				break;
			case '>':
				fputs("&gt;", fp);
				break;
			case '<':
				fputs("&lt;", fp);
				break;
			case '\'':
				fputs("&apos;", fp);
				break;
			case '\"':
				fputs("&quot;", fp);
				break;
			default:
				fputc(*str, fp);
				break;
		}
		// write out the whole character
		char *next = CharNextA(str);
		while (++str != next)
			fputc(*str, fp);
	}
}

void Bookmark_WriteAsXML(const wchar_t *filename, int max)
{
	FILE *destFp=_wfopen(filename,L"wb");
	FILE *fp=_wfopen(BOOKMARKFILE8,L"rt");

	if (fp && destFp)
	{
		//fprintf(destFp, "<?xml version=\"1.0\" encoding=\"windows-%u\"?>", GetACP());
		fprintf(destFp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		fputs("<bookmarks>\n", destFp);
		int x=0;
		for (;;)
		{
			char ft[4096] = {0}, fn[4096] = {0};
			fgets(fn,4096,fp);
			if (feof(fp)) break;
			fgets(ft,4096,fp);
			if (feof(fp)) break;
			if (ft[0] && fn[0])
			{
				if (fn[strlen(fn)-1]=='\n') fn[strlen(fn)-1]=0;
				if (ft[strlen(ft)-1]=='\n') ft[strlen(ft)-1]=0;
				if (ft[0] && fn[0])
				{
					fputs("<bookmark file=\"", destFp);
					WriteEscaped(destFp, fn);
					fputs("\" title=\"", destFp);
					WriteEscaped(destFp, ft);
					fputs("\"/>\n", destFp);
					x++;
				}
			}
			if (x == max)
				break;
		}
		fputs("</bookmarks>", destFp);
	}
	if (fp)
		fclose(fp);
	if (destFp)
		fclose(destFp);
}