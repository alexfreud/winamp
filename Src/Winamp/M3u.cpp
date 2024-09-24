/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: 
 ** Created:
 **/
#include "Main.h"
#include "strutil.h"
#include "api.h"
#include "WinampPlaylist.h"
#include "../nu/AUtoWide.h"
#include "../nu/AUtoChar.h"

#include "../WAT/WAT.h"

int savem3ufn(const wchar_t *filename, int rel, int useBase)
{
	UINT conv=CP_ACP;
	if (!lstrcmpiW(PathFindExtensionW(filename), L".m3u8"))
		conv=CP_UTF8;
	FILE *fp = 0;
	int ext = 1;
	int pos = PlayList_getPosition();
	fp = _wfopen(filename, L"wt");
	if (!fp) return -1;
	if (conv==CP_UTF8)
		fputs("\xEF\xBB\xBF", fp); // write UTF-8 BOM
	if (ext) fprintf(fp, "#EXTM3U\n");
	PlayList_setposition(0);
	if (PlayList_getlength()) for (;;)
	{
		if ( !PlayList_gethidden(PlayList_getPosition()) 
			&& !PlayList_hasanycurtain(PlayList_getPosition())
			)
		{
			wchar_t fnbuf[FILENAME_SIZE] = {0};
			wchar_t ftbuf[FILETITLE_SIZE] = {0};
			PlayList_getitem2W(PlayList_getPosition(), fnbuf, ftbuf);
			int len = PlayList_getcurrentlength();
			if (rel) 
			{
				PlayList_makerelative(filename, fnbuf, useBase);
				if (fnbuf[0]=='#') // woops, can't start a line with #
					PlayList_getitem2W(PlayList_getPosition(), fnbuf, 0); // retrieve file name again
			}


			if ( ext && PlayList_getcached( PlayList_getPosition() ) )
			{
				const char *l_ext_inf = PlayList_getExtInf( PlayList_getPosition() );

				wa::strings::wa_string l_filename( fnbuf );

				if ( ext && l_ext_inf && *l_ext_inf && l_filename.contains( "://" ) )
				{
					fprintf( fp, "#EXTINF:%d%s,%s\n%s\n", len, l_ext_inf, (char *)AutoChar( ftbuf, conv ), (char *)AutoChar( fnbuf, conv ) );

					delete( l_ext_inf );
				}
				else
					fprintf( fp, "#EXTINF:%d,%s\n%s\n", len, (char *)AutoChar( ftbuf, conv ), (char *)AutoChar( fnbuf, conv ) );
			}
			else 
				fprintf(fp, "%s\n", (char *)AutoChar(fnbuf, conv));
		}
		if (PlayList_advance(1) < 0)
			break;
	}
	fclose(fp);
	PlayList_setposition(pos);
	return 0;
}
