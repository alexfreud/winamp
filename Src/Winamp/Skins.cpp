/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author:
** Created:
**/

#include "main.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"
#include "minizip/unzip.h"
#include "api.h"

int g_skinloadedmanually;
int g_skinmissinggenff = 0;

HWND skin_hwnd;

BOOL _cleanupDirW(const wchar_t *dir)
{
	wchar_t dirmask[MAX_PATH] = {0};
	HANDLE h;
	WIN32_FIND_DATAW d = {0};
	PathCombineW(dirmask, dir, L"*.*");
	h = FindFirstFileW(dirmask, &d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t v[MAX_PATH] = {0};
			PathCombineW(v, dir, d.cFileName);
			if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp(d.cFileName,L".") && wcscmp(d.cFileName,L".."))
					_cleanupDirW(v);
			}
			else
			{
				if(!DeleteFileW(v))
				{
					// this handles some rogue cases where files in the wlz's aren't unloadable
					MoveFileExW(v, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					MoveFileExW(dir, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
		while (FindNextFileW(h, &d));
		FindClose(h);
	}
	
	return RemoveDirectoryW(dir);
}

// attempt to cleanup the last extracted temp folder for a skin incase Winamp crashed on exit
void Skin_CleanupAfterCrash(void)
{
	wchar_t buf[1024] = {0};
	char str[78] = {0};
	StringCchPrintfA(str,78,"skin_clean_up%ws",szAppName);
	_r_sW(str, buf, sizeof(buf));
	if (buf[0])
	{
		_cleanupDirW(buf);
		_w_sW(str, 0);
	}
}

void Skin_CleanupZip(void)
{
	if (!SKINTEMPDIR[0]) return ;
	if (_cleanupDirW(SKINTEMPDIR))
	{
		char str[78] = {0};
		StringCchPrintfA(str,78,"skin_clean_up%ws",szAppName);
		_w_s(str, 0);
	}
}

void CreateDirectoryForFileW(wchar_t *fn, wchar_t *base)
{
	wchar_t buf1[MAX_PATH] = {0};
	wchar_t *tmp;
	wchar_t *p;
	StringCchCopyW(buf1, MAX_PATH, fn);
	tmp = scanstr_backW(buf1, L"\\/", buf1);
	*tmp = 0;
	tmp = buf1;
	while (tmp && *tmp) { if (*tmp == L'/') *tmp = L'\\'; tmp++; }

	p = buf1 + wcslen(base);
	while (p && *p)
	{
		while (p && *p != L'\\' && *p) p = CharNextW(p);
		if (p && !*p) CreateDirectoryW(buf1, NULL);
		else
		{
			if (p) *p = 0;
			CreateDirectoryW(buf1, NULL);
			if (p) *p++ = L'\\';
		}
	}
}

static void make_skin_dir(void)
{
	if (config_skin[0])
	{
		if (_wcsicmp(extensionW(config_skin), L"zip") && _wcsicmp(extensionW(config_skin), L"wsz") && _wcsicmp(extensionW(config_skin), L"wal"))
		{
			if (PathIsFileSpecW(config_skin) || PathIsRelativeW(config_skin))
				PathCombineW(skin_directory, SKINDIR, config_skin);
			else 
				StringCchCopyW(skin_directory, MAX_PATH, config_skin);
		}
		else
		{
			wchar_t dirmask[MAX_PATH] = {0};
			char str[78] = {0};
			StringCchCopyW(skin_directory, MAX_PATH, SKINTEMPDIR);
			CreateDirectoryW(SKINTEMPDIR, NULL);
			StringCchPrintfA(str, 78, "skin_clean_up%ws", szAppName);
			_w_sW(str, SKINTEMPDIR);
			{
				unzFile f;
				if (PathIsFileSpecW(config_skin) || PathIsRelativeW(config_skin))
					PathCombineW(dirmask, SKINDIR, config_skin);
				else
					StringCchCopyW(dirmask, MAX_PATH, config_skin);
				f = unzOpen(AutoChar(dirmask)); 
				if (f)
				{
					int iswa3 = 0;

					if (unzGoToFirstFile(f) == UNZ_OK)
					{
						wchar_t buriedskinxml[MAX_PATH] = {0};
						wchar_t *p;
						StringCchCopyW(buriedskinxml, MAX_PATH, config_skin);
						p = wcschr(buriedskinxml, '.');
						if (p) *p = 0;
						StringCchCatW(buriedskinxml, MAX_PATH, L"/skin.xml");
						do
						{
							char filename[MAX_PATH] = {0};
							unzGetCurrentFileInfo(f, NULL, filename, sizeof(filename), NULL, 0, NULL, 0);
							if (!_stricmp(filename, "skin.xml") || !_stricmp(filename, AutoChar(buriedskinxml))) iswa3++;
						}
						while (!iswa3 && unzGoToNextFile(f) == UNZ_OK);
					}

					if (unzGoToFirstFile(f) == UNZ_OK)
					{
						OVERLAPPED asyncIO = {0};
						int isNT = (GetVersion() < 0x80000000);
						if (isNT)
						{
							asyncIO.hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
							asyncIO.OffsetHigh = 0;
						}
						do
						{
							char filename[MAX_PATH], *fn, *p;
							if (isNT)
								SetEvent(asyncIO.hEvent);
							unzGetCurrentFileInfo(f, NULL, filename, sizeof(filename), NULL, 0, NULL, 0);

							//Only extract the file-types that could be in a skin
							//If we don't filter here it's a security hole
							if (iswa3)
							{
								if (unzOpenCurrentFile(f) == UNZ_OK)
								{
									fn = filename;
									if (strstr(fn, ":")) fn = strstr(fn, ":") + 1;
									while (fn && *fn == '\\') fn++;
									p = extension(fn);

									// TODO: really should enum image loaders so we only extract supported image files									
									if (fn[0] && (p != NULL && (!_stricmp(p, "xml") || !_stricmp(p, "png") ||
										!_stricmp(p, "cur") || !_stricmp(p, "bmp") || !_stricmp(p, "txt") ||
										!_stricmp(p, "gif") || !_stricmp(p, "ttf") || !_stricmp(p, "m") ||
										!_stricmp(p, "maki") || !_stricmp(p, "mp3") || !_stricmp(p, "wma") ||
										!_stricmp(p, "nsv") || !_stricmp(p, "nsa") || !_stricmp(p, "m4a") ||
										!_stricmp(p, "avi") || !_stricmp(p, "wav") || !_stricmp(p, "mp4") ||
										!_stricmp(p, "ogg") || IsPlaylistExtension(AutoWide(p)) ||
										!_stricmp(p, "mpg") || !_stricmp(p, "mid") || !_stricmp(p, "midi") ||
										!_stricmp(p, "mpeg") || !_stricmp(p, "url") || !_stricmp(p, "jpg") ||
										!_stricmp(p, "mi") ) ) )
									{
										PathCombineW(dirmask, SKINTEMPDIR, AutoWide(fn));
										CreateDirectoryForFileW(dirmask, SKINTEMPDIR);
										goto do_write;
									}
								}
							}
							else
							{
								fn = scanstr_back(filename, "\\/", filename - 1);
								p = extension(++fn);
								if (!_stricmp(p, "txt") || !_stricmp(p, "cur") || !_stricmp(p, "bmp") || !_stricmp(p, "ini"))
								{
									int success = 0;
									if (unzOpenCurrentFile(f) == UNZ_OK)
									{
										HANDLE fp;

										PathCombineW(dirmask, SKINTEMPDIR, AutoWide(fn));

do_write:
										//fp = fopen(dirmask,"wb");
										fp = CreateFileW(dirmask, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | (isNT ? FILE_FLAG_OVERLAPPED : 0), NULL);
										if (fp != INVALID_HANDLE_VALUE)
										{
											int l = 0, pos = 0, bufNum=0;

											#define SKIN_ZIP_BUFFER_SIZE 2048
											char buf[SKIN_ZIP_BUFFER_SIZE*2] = {0};
											success = 1;
											do
											{
												DWORD written = 0;
												bufNum = !bufNum; // bufNum = (bufNUm + 1) %2;
												l = unzReadCurrentFile(f, buf+SKIN_ZIP_BUFFER_SIZE*bufNum, SKIN_ZIP_BUFFER_SIZE);
												if (!l)
													unzCloseCurrentFile(f);
												if (isNT)
												{
													WaitForSingleObject(asyncIO.hEvent, INFINITE);
													if (l > 0)
													{
														asyncIO.Offset = pos;
														if (WriteFile(fp, buf+SKIN_ZIP_BUFFER_SIZE*bufNum, l, NULL, &asyncIO) == FALSE
															&& GetLastError() != ERROR_IO_PENDING)
														{
															success=0;
														}
														pos += l;
													}
												}
												else
												{
													if (l > 0)
													{
														if (WriteFile(fp, buf+SKIN_ZIP_BUFFER_SIZE*bufNum, l, &written, NULL) == FALSE)
															success = 0;
													}
												}
											} while (l > 0 && success);

											CloseHandle(fp);
										}
									}
								}
							}
						}
						while (unzGoToNextFile(f) == UNZ_OK);
						if (isNT && asyncIO.hEvent)
						{
							CloseHandle(asyncIO.hEvent);
						}
					} 
					unzClose(f);
				}
			}
		}
	}
	else skin_directory[0] = 0;
}

int Skin_PLColors[6] =
{
	RGB(0, 255, 0),
		RGB(255, 255, 255),
		RGB(0, 0, 0),
		RGB(0, 0, 198),
		RGB(0, 255, 0),
		RGB(0, 0, 0),
}, Skin_UseGenNums = 0;
char Skin_PLFont[128] = "";
wchar_t Skin_PLFontW[128] = L"";

static BOOL CALLBACK skinDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static int *skin_rgn_points, *skin_rgn_numpoints, skin_rgn_numpoints_tot;
static int *skin_rgn_points_eq, *skin_rgn_numpoints_eq, skin_rgn_numpoints_tot_eq;
static int *skin_rgn_points_eq_ws, *skin_rgn_numpoints_eq_ws, skin_rgn_numpoints_tot_eq_ws;
static int *skin_rgn_points_ws, *skin_rgn_numpoints_ws, skin_rgn_numpoints_tot_ws;
static int *skin_rgn_points_pl, *skin_rgn_numpoints_pl, skin_rgn_numpoints_tot_pl;
static int *skin_rgn_points_pl_ws, *skin_rgn_numpoints_pl_ws, skin_rgn_numpoints_tot_pl_ws;

int Skin_GetRegionPointList( int eq, int **points, int **counts )
{
	if ( !eq )
	{
		if ( config_windowshade )
		{
			*points = skin_rgn_points_ws;
			*counts = skin_rgn_numpoints_ws;

			return skin_rgn_numpoints_tot_ws;
		}
		else
		{
			*points = skin_rgn_points;
			*counts = skin_rgn_numpoints;

			return skin_rgn_numpoints_tot;
		}
	}
	else if ( eq == 1 )
	{
		if ( config_eq_ws )
		{
			*points = skin_rgn_points_eq_ws;
			*counts = skin_rgn_numpoints_eq_ws;

			return skin_rgn_numpoints_tot_eq_ws;
		}
		else
		{
			*points = skin_rgn_points_eq;
			*counts = skin_rgn_numpoints_eq;

			return skin_rgn_numpoints_tot_eq;
		}
	}
	else if ( eq == 2 )
	{
		if ( config_pe_height == 14 )
		{
			*points = skin_rgn_points_pl_ws;
			*counts = skin_rgn_numpoints_pl_ws;

			return skin_rgn_numpoints_tot_pl_ws;
		}
		else
		{
			*points = skin_rgn_points_pl;
			*counts = skin_rgn_numpoints_pl;

			return skin_rgn_numpoints_tot_pl;
		}
	}


	return 0;
}

static int getnums( char *section, char *name, const wchar_t *fn, int **list )
{
	char buf[ 65535 ] = { 0 }, *p = buf;
	int n = 0, nn;
	GetPrivateProfileStringA( section, name, "", buf, sizeof( buf ), AutoChar( fn ) );

	while ( p )
	{
		int t = 0;
		while ( p && ( *p == ' ' || *p == '\t' || *p == ',' ) ) p++;
		if ( p && *p == '-' ) p++;
		while ( p && ( *p >= '0' && *p <= '9' ) )
		{
			t = 1; p++;
		}
		if ( t ) n++;
		else break;
	}

	if ( !n )
		return 0;

	p = buf;
	list[ 0 ] = (int *) GlobalAlloc( GPTR, sizeof( int ) * n );
	nn = n;
	n = 0;

	while ( p && n < nn )
	{
		int t = 0;
		int s = 0, sign = 1;

		while ( p && ( *p == ' ' || *p == '\t' || *p == ',' ) )
			p++;

		if ( p && *p == '-' )
		{
			sign = -1; p++;
		}

		while ( p && ( *p >= '0' && *p <= '9' ) )
		{
			s = s * 10 + *p++ - '0'; t = 1;
		}

		if ( t )
			list[ 0 ][ n++ ] = s * sign;
		else
			break;
	}
	return n;
}

static int hexGetPrivateProfileInt( char *sec, char *key, int def, const wchar_t *fn )
{
	char str[ 100 ] = { 0 }, *s = str;

	if ( !GetPrivateProfileStringA( sec, key, "", str, sizeof( str ), AutoChar( fn ) ) || !*str )
		return def;

	if ( s && *s == '#' )
		s++;

	if ( s )
		def = strtol( s, &s, 16 );

	int r = ( def >> 16 ) & 255;
	int g = ( def >> 8 ) & 255;
	int b = def & 255;

	return RGB( r, g, b );
}

HCURSOR Skin_Cursors[ N_CURSORS ];

static void Skin_LoadCursors( void )
{
	static struct
	{
		int cid;
		const wchar_t *fn;
	}
	cursor_loads[ N_CURSORS ] =
	{
		// main, non windowshade, start 0, end 8
		{IDC_LRSCROLL,     L"VolBal.cur"},   // vol & bal
		{IDC_LRSCROLL,     L"Posbar.cur"},   // pos
		{IDC_NORMALCURSOR, L"WinBut.cur"},   //wshade
		{IDC_NORMALCURSOR, L"Min.cur"},      //min
		{IDC_DANGER,       L"Close.cur"},    //close
		{IDC_NORMALCURSOR, L"MainMenu.cur"}, //mainmenu
		{IDC_MOVEMAIN,     L"TitleBar.cur"}, // titelbar
		{IDC_LRSCROLL,     L"SongName.cur"},
		{IDC_NORMALCURSOR, L"Normal.cur"},
		// main, windowshade, start 9, end 14
		{IDC_NORMALCURSOR, L"WinBut.cur"},   //wshade
		{IDC_NORMALCURSOR, L"Min.cur"},      //min
		{IDC_LRSCROLL,     L"WSPosbar.cur"}, // seeker
		{IDC_DANGER,       L"Close.cur"},    //close
		{IDC_NORMALCURSOR, L"MMenu.cur"},    //mainmenu
		{IDC_NORMALCURSOR, L"WSNormal.cur"},
		// playlist editor, normal, start 15 end 20
		{IDC_NORMALCURSOR, L"PWinBut.cur"},  //wshade
		{IDC_DANGER,       L"PClose.cur"},   //close
		{IDC_MOVEMAIN,     L"PTBar.cur"},    // titelbar
		{IDC_UDSCROLL,     L"PVScroll.cur"},
		{IDC_RESIZE,       L"PSize.cur"},
		{IDC_NORMALCURSOR, L"PNormal.cur"},
		// playlist editor, windowshade, start 21 end 24
		{IDC_NORMALCURSOR, L"PWinBut.cur"},  //wshade
		{IDC_DANGER,       L"PClose.cur"},   //close
		{IDC_LRSCROLL,     L"PWSSize.cur"},  //size
		{IDC_NORMALCURSOR, L"PWSNorm.cur"},
		// equalizer start 25, end 28
		{IDC_UDSCROLL,     L"EQSlid.cur"},
		{IDC_DANGER,       L"EQClose.cur"},  //close
		{IDC_MOVEMAIN,     L"EQTitle.cur"},  // titelbar
		{IDC_NORMALCURSOR, L"EQNormal.cur"},
	};

	int x;

	for ( x = 0; x < N_CURSORS; x++ )
	{
		if ( Skin_Cursors[ x ] )
			DestroyCursor( Skin_Cursors[ x ] );

		Skin_Cursors[ x ] = 0;
	}

	for ( x = 0; x < N_CURSORS; x++ )
	{
		HCURSOR hc = 0;

		if ( config_skin[ 0 ] )
		{
			wchar_t sn[ MAX_PATH ] = { 0 };
			PathCombineW( sn, skin_directory, cursor_loads[ x ].fn );
			hc = (HCURSOR) LoadImageW( NULL, sn, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE );
		}

		if ( !hc )
		{
			hc = (HCURSOR) LoadImageW( hMainInstance, MAKEINTRESOURCEW( cursor_loads[ x ].cid ), IMAGE_CURSOR, 0, 0, 0 );
		}

		Skin_Cursors[ x ] = hc;
	}
}

#define COLOR_ERROR (0xff00ff)

static ARGB32 parseColor( const char *color, int *error )
{
	*error = 0;

	if ( color == NULL || *color == '\0' )
	{
		*error = 1;
		
		return COLOR_ERROR;
	}

	if ( strchr( color, ',' ) )
	{
		int r = 0, g = 0, b = 0;

		if ( sscanf( color, "%d,%d,%d", &r, &g, &b ) != 3 )
		{
			*error = 1;
			
			return COLOR_ERROR;
		}

		return RGB( r, g, b ); // our colors are reversed internally
	}

	if ( *color == '#' )
	{
		int r = 0, g = 0, b = 0;

		if ( sscanf( color, "#%02x%02x%02x", &r, &g, &b ) != 3 )
		{
			*error = 1;
			
			return COLOR_ERROR;
		}

		return RGB( r, g, b );
	}

	*error = 1;

	return COLOR_ERROR;
}

static void Skin_LoadFreeformColors( const wchar_t *fn )
{
	WASABI_API_PALETTE->Reset();
	FILE *f = _wfopen( fn, L"rt" );

	if ( f )
	{
		char buffer[ 1024 ] = { 0 };

		if ( fgets( buffer, 1024, f ) )
		{
			size_t len = strlen( buffer );

			if ( buffer[ len - 1 ] == '\n' )
				buffer[ len - 1 ] = 0;

			if ( !strcmp( buffer, "[colors]" ) )
			{
				WASABI_API_PALETTE->StartTransaction();

				while ( !feof( f ) )
				{
					fgets( buffer, 1024, f );

					char *equal_pt = strchr( buffer, '=' );

					if ( equal_pt )
					{
						*equal_pt = 0;
						int error = 0;
						ARGB32 color = parseColor( equal_pt + 1, &error );

						if ( !error )
						{
							wchar_t utf8_expand[ 1024 ] = { 0 };

							MultiByteToWideCharSZ( CP_UTF8, 0, buffer, -1, utf8_expand, 1024 );

							WASABI_API_PALETTE->AddColor( utf8_expand, color );
						}
					}
				}

				WASABI_API_PALETTE->EndTransaction();
			}
		}

		fclose( f );
	}
}

BOOL Skin_Check_Modern_Support()
{
	wchar_t fn[ MAX_PATH ] = { 0 };

	PathCombineW( fn, skin_directory, L"skin.xml" );

	if ( PathFileExistsW( fn ) && !GetModuleHandleW( L"gen_ff.dll" ) )
	{
		g_skinmissinggenff = 1;

		return FALSE;
	}

	g_skinmissinggenff = 0;

	return TRUE;
}

void Skin_Load(void)
{
	Skin_CleanupZip();
	make_skin_dir();
	g_skinmissinggenff = 0;

	if (skin_rgn_points) GlobalFree(skin_rgn_points);
	if (skin_rgn_numpoints) GlobalFree(skin_rgn_numpoints);
	skin_rgn_numpoints = NULL;
	skin_rgn_points = NULL;

	if (skin_rgn_points_ws) GlobalFree(skin_rgn_points_ws);
	if (skin_rgn_numpoints_ws) GlobalFree(skin_rgn_numpoints_ws);
	skin_rgn_numpoints_ws = NULL;
	skin_rgn_points_ws = NULL;

	if (skin_rgn_points_eq) GlobalFree(skin_rgn_points_eq);
	if (skin_rgn_numpoints_eq) GlobalFree(skin_rgn_numpoints_eq);
	skin_rgn_numpoints_eq = NULL;
	skin_rgn_points_eq = NULL;

	if (skin_rgn_points_eq_ws) GlobalFree(skin_rgn_points_eq_ws);
	if (skin_rgn_numpoints_eq_ws) GlobalFree(skin_rgn_numpoints_eq_ws);
	skin_rgn_numpoints_eq_ws = NULL;
	skin_rgn_points_eq_ws = NULL;

	if (skin_rgn_points_pl) GlobalFree(skin_rgn_points_pl);
	if (skin_rgn_numpoints_pl) GlobalFree(skin_rgn_numpoints_pl);
	skin_rgn_numpoints_pl = NULL;
	skin_rgn_points_pl = NULL;

	if (skin_rgn_points_pl_ws) GlobalFree(skin_rgn_points_pl_ws);
	if (skin_rgn_numpoints_pl_ws) GlobalFree(skin_rgn_numpoints_pl_ws);
	skin_rgn_numpoints_pl_ws = NULL;
	skin_rgn_points_pl_ws = NULL;

	skin_rgn_numpoints_tot = 0;
	skin_rgn_numpoints_tot_ws = 0;
	skin_rgn_numpoints_tot_eq = 0;
	skin_rgn_numpoints_tot_eq_ws = 0;
	skin_rgn_numpoints_tot_pl = 0;
	skin_rgn_numpoints_tot_pl_ws = 0;

	int t_Skin_PLColors[6] =
	{
		RGB(0, 255, 0),
		RGB(255, 255, 255),
		RGB(0, 0, 0),
		RGB(0, 0, 198),
		RGB(0, 255, 0),
		RGB(0, 0, 0),
	};
	memcpy(Skin_PLColors, t_Skin_PLColors, sizeof(Skin_PLColors));
	Skin_PLFont[0] = 0;
	Skin_PLFontW[0] = 0;
	Skin_UseGenNums = !config_skin[0];

	// load default freeform colors
	if (!config_skin[0])
	{
		WASABI_API_PALETTE->Reset();
		WASABI_API_PALETTE->StartTransaction();
		WASABI_API_PALETTE->AddColor(L"wasabi.popupmenu.background.selected", RGB(0x75, 0x74, 0x8B));
		WASABI_API_PALETTE->AddColor(L"wasabi.popupmenu.background", RGB(0x38, 0x37, 0x57));
		WASABI_API_PALETTE->AddColor(L"wasabi.popupmenu.text", RGB(0xFF, 0xFF, 0xFF));
		WASABI_API_PALETTE->AddColor(L"wasabi.popupmenu.text.selected", RGB(0xFF, 0xFF, 0xFF));
		WASABI_API_PALETTE->AddColor(L"wasabi.popupmenu.text.inactive", RGB(0x73, 0x73, 0x89));
		WASABI_API_PALETTE->AddColor(L"wasabi.popupmenu.frame", RGB(0x75, 0x74, 0x8B));
		WASABI_API_PALETTE->AddColor(L"wasabi.popupmenu.separator", RGB(0x75, 0x74, 0x8B));
		WASABI_API_PALETTE->AddColor(L"wasabi.tooltip.background", RGB(0x38, 0x37, 0x57));
		WASABI_API_PALETTE->AddColor(L"wasabi.tooltip.text", RGB(0xFF, 0xFF, 0xFF));
		WASABI_API_PALETTE->AddColor(L"wasabi.tooltip.frame", RGB(0x75, 0x74, 0x8B));
		WASABI_API_PALETTE->EndTransaction();
	}

	if (skin_directory[0])
	{
		if (!Skin_Check_Modern_Support())
		{
			Skin_LoadCursors();
			return;
		}

		wchar_t fn[MAX_PATH] = {0};
		PathCombineW(fn, skin_directory, L"region.txt");
		skin_rgn_numpoints_tot = getnums("Normal", "NumPoints", fn, &skin_rgn_numpoints);
		if (skin_rgn_numpoints_tot)
		{
			int all_p;
			if ((all_p = getnums("Normal", "PointList", fn, &skin_rgn_points)))
			{
				int o = 0;
				int x;
				for (x = 0; x < skin_rgn_numpoints_tot; x ++)
					o += skin_rgn_numpoints[x];
				if (o == all_p / 2 && !(all_p & 1))	goto s1;
				// else MessageBox(NULL,"region.txt:\n[Normal]\npoints in PointList\ndon't match NumPoints","Error in skin:",0);
			}
		}

		if (skin_rgn_numpoints) GlobalFree(skin_rgn_numpoints);
		if (skin_rgn_points) GlobalFree(skin_rgn_points);
		skin_rgn_numpoints_tot = 0;
		skin_rgn_numpoints = NULL;
		skin_rgn_points = NULL;

s1:
		skin_rgn_numpoints_tot_ws = getnums("WindowShade", "NumPoints", fn, &skin_rgn_numpoints_ws);
		if (skin_rgn_numpoints_tot_ws)
		{
			int all_p;
			if ((all_p = getnums("WindowShade", "PointList", fn, &skin_rgn_points_ws)))
			{
				int o = 0;
				int x;
				for (x = 0; x < skin_rgn_numpoints_tot_ws; x ++)
					o += skin_rgn_numpoints_ws[x];
				if (o == all_p / 2 && !(all_p & 1))	goto s2;
				// else MessageBox(NULL,"region.txt:\n[Windowshade]\npoints in PointList\ndon't match NumPoints","Error in skin:",0);
			}
		}
		if (skin_rgn_numpoints_ws) GlobalFree(skin_rgn_numpoints_ws);
		if (skin_rgn_points_ws) GlobalFree(skin_rgn_points_ws);
		skin_rgn_numpoints_ws = NULL;
		skin_rgn_numpoints_tot_ws = 0;
		skin_rgn_points_ws = NULL;

s2:
		skin_rgn_numpoints_tot_eq = getnums("Equalizer", "NumPoints", fn, &skin_rgn_numpoints_eq);
		if (skin_rgn_numpoints_tot_eq)
		{
			int all_p;
			if ((all_p = getnums("Equalizer", "PointList", fn, &skin_rgn_points_eq)))
			{
				int o = 0;
				int x;
				for (x = 0; x < skin_rgn_numpoints_tot_eq; x ++)
					o += skin_rgn_numpoints_eq[x];
				if (o == all_p / 2 && !(all_p & 1))	goto s3;
				// else MessageBox(NULL,"region.txt:\n[Equalizer]\npoints in PointList\ndon't match NumPoints","Error in skin:",0);
			}
		}
		if (skin_rgn_numpoints_eq) GlobalFree(skin_rgn_numpoints_eq);
		if (skin_rgn_points_eq) GlobalFree(skin_rgn_points_eq);
		skin_rgn_numpoints_eq = NULL;
		skin_rgn_numpoints_tot_eq = 0;
		skin_rgn_points_eq = NULL;

s3:
		skin_rgn_numpoints_tot_eq_ws = getnums("EqualizerWS", "NumPoints", fn, &skin_rgn_numpoints_eq_ws);
		if (skin_rgn_numpoints_tot_eq_ws)
		{
			int all_p;
			if ((all_p = getnums("EqualizerWS", "PointList", fn, &skin_rgn_points_eq_ws)))
			{
				int o = 0;
				int x;
				for (x = 0; x < skin_rgn_numpoints_tot_eq_ws; x ++)
					o += skin_rgn_numpoints_eq_ws[x];
				if (o == all_p / 2 && !(all_p & 1))	goto s4;
				// else MessageBox(NULL,"region.txt:\n[EqualizerWS]\npoints in PointList\ndon't match NumPoints","Error in skin:",0);
			}
		}
		if (skin_rgn_numpoints_eq_ws) GlobalFree(skin_rgn_numpoints_eq_ws);
		if (skin_rgn_points_eq_ws) GlobalFree(skin_rgn_points_eq_ws);
		skin_rgn_numpoints_eq_ws = NULL;
		skin_rgn_numpoints_tot_eq_ws = 0;
		skin_rgn_points_eq_ws = NULL;

s4:
		/*skin_rgn_numpoints_tot_pl = getnums("Playlist", "NumPoints", fn, &skin_rgn_numpoints_pl);
		if (skin_rgn_numpoints_tot_pl)
		{
			int all_p;
			if ((all_p = getnums("Playlist", "PointList", fn, &skin_rgn_points_pl)))
			{
				int o = 0;
				int x;
				for (x = 0; x < skin_rgn_numpoints_tot_pl; x ++)
					o += skin_rgn_numpoints_pl[x];
				if (o == all_p / 2 && !(all_p & 1))	goto s5;
				// else MessageBox(NULL,"region.txt:\n[Playlist]\npoints in PointList\ndon't match NumPoints","Error in skin:",0);
			}
		}*/
		if (skin_rgn_numpoints_pl) GlobalFree(skin_rgn_numpoints_pl);
		if (skin_rgn_points_pl) GlobalFree(skin_rgn_points_pl);
		skin_rgn_numpoints_pl = NULL;
		skin_rgn_numpoints_tot_pl = 0;
		skin_rgn_points_pl = NULL;


//s5:
		/*skin_rgn_numpoints_tot_pl_ws = getnums("PlaylistWS", "NumPoints", fn, &skin_rgn_numpoints_pl_ws);
		if (skin_rgn_numpoints_tot_pl_ws)
		{
			int all_p;
			if ((all_p = getnums("PlaylistWS", "PointList", fn, &skin_rgn_points_pl_ws)))
			{
				int o = 0;
				int x;
				for (x = 0; x < skin_rgn_numpoints_tot_pl_ws; x ++)
					o += skin_rgn_numpoints_pl_ws[x];
				if (o == all_p / 2 && !(all_p & 1))	goto s6;
				// else MessageBox(NULL,"region.txt:\n[PlaylistWS]\npoints in PointList\ndon't match NumPoints","Error in skin:",0);
			}
		}*/
//		if (skin_rgn_numpoints_pl_ws) GlobalFree(skin_rgn_numpoints_pl_ws);
//		if (skin_rgn_points_pl_ws) GlobalFree(skin_rgn_points_pl_ws);
//		skin_rgn_numpoints_pl_ws = NULL;
//		skin_rgn_numpoints_tot_pl_ws = 0;
//		skin_rgn_points_pl_ws = NULL;

//s6:
//		;

#if 0
		{
			char str[1024] = {0};
			PathCombine(fn, skin_directory, "mb.ini");
			GetPrivateProfileString("Winamp", "MBOpen", "", str, sizeof(str), fn);
			str[1023] = 0;
			if (str[0])
			{
				if (hMBWindow)
				{
					if (g_skinloadedmanually) SendMessageW(hMainWindow, WM_USER, (WPARAM)0, IPC_MBOPEN);
					SendMessageW(hMainWindow, WM_USER, (WPARAM)str, IPC_MBOPEN);
				}
				else
				{
					extern char page_nav[1024];
					if (g_skinloadedmanually) config_mb_open = 1;
					lstrcpy(page_nav, str);
				}
			}
		}
#endif

		PathCombineW(fn, skin_directory, L"pledit.txt");

		Skin_PLColors[0] = hexGetPrivateProfileInt("Text", "Normal",     Skin_PLColors[0], fn);
		Skin_PLColors[1] = hexGetPrivateProfileInt("Text", "Current",    Skin_PLColors[1], fn);
		Skin_PLColors[2] = hexGetPrivateProfileInt("Text", "NormalBG",   Skin_PLColors[2], fn);
		Skin_PLColors[3] = hexGetPrivateProfileInt("Text", "SelectedBG", Skin_PLColors[3], fn);
		Skin_PLColors[4] = hexGetPrivateProfileInt("Text", "mbFG",       Skin_PLColors[4], fn);
		Skin_PLColors[5] = hexGetPrivateProfileInt("Text", "mbBG",       Skin_PLColors[5], fn);

		char plfont[256] = {0};

		GetPrivateProfileStringA("Text", "Font", "", plfont, 256, AutoChar(fn));
		MultiByteToWideCharSZ(CP_UTF8, 0, plfont, -1, Skin_PLFontW, sizeof(Skin_PLFontW)/sizeof(*Skin_PLFontW));
		WideCharToMultiByteSZ(CP_ACP, 0, Skin_PLFontW, -1, Skin_PLFont, sizeof(Skin_PLFont)/sizeof(*Skin_PLFont), 0, 0);
		// used to allow for the gen.bmp numbers support in 5.64 - need skin to say as too many have junk where it's taken from
		Skin_UseGenNums = GetPrivateProfileIntW(L"Misc", L"UseGenNums", 0, fn);

		PathCombineW(fn, skin_directory, L"colors.ini");
		Skin_LoadFreeformColors(fn);
	}
	Skin_LoadCursors();
}

void Skin_Random(void)
{
	if (config_randskin)
	{
		HANDLE h;
		WIN32_FIND_DATAW d = {0};
		int v, x;
		wchar_t dirmask[MAX_PATH] = {0};
		PathCombineW(dirmask, SKINDIR, L"*");
		x = 1;
		h = FindFirstFileW(dirmask, &d);
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (wcscmp(d.cFileName, L".") && wcscmp(d.cFileName, L"..")) x++;
				}
				else if (!_wcsicmp(extensionW(d.cFileName), L"zip") || !_wcsicmp(extensionW(d.cFileName), L"wsz") || !_wcsicmp(extensionW(d.cFileName), L"wal"))
					x++;
			}
			while (FindNextFileW(h, &d));
			FindClose(h);
		}
		v = warand() % x;
		if (!v)
		{
			config_skin[0] = 0;
			return ;
		}
		x = 1;
		h = FindFirstFileW(dirmask, &d);
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				int t = 0;
				if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (wcscmp(d.cFileName, L".") && wcscmp(d.cFileName, L"..")) t = 1;
				}
				else if (!_wcsicmp(extensionW(d.cFileName), L"zip") || !_wcsicmp(extensionW(d.cFileName), L"wsz") || !_wcsicmp(extensionW(d.cFileName), L"wal")) t = 1;
				if (t)
				{
					if (x == v)
					{
						StringCchCopyW(config_skin, MAX_PATH, d.cFileName);
						break;
					}
					x++;
				}
			}
			while (FindNextFileW(h, &d));
			FindClose(h);
		}
		PostMessageW(hMainWindow, WM_COMMAND, WINAMP_REFRESHSKIN, 0);

		if (prefs_last_page == 40 && IsWindow(prefs_hwnd))
		{
			prefs_last_page = 0;
			prefs_dialog(1);
			prefs_last_page = 40;
			prefs_dialog(1);
		}
	}
}