// verctrl.cpp : Defines the entry point for the console application.
//

#include "verctrl.h"

static char *branding = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	_tprintf(_T("Version Control Utility ver 1.0\n"));
	_tprintf(_T("Patching version information in the winamp project source files\n\n"));
	
	bool setted = false;
	if (argc > 1)
	{	
		if (0 == _tcsicmp(argv[1], _T("QA")))
		{
			type = Beta;
			setted = true;
		}
		else if (0 == _tcsicmp(argv[1], _T("BETA")))
		{
			type = Beta;
			setted = true;
		}
		else if (0 == _tcsicmp(argv[1], _T("NIGHT")))
		{
			type = Night;
			setted = true;
		}
		else if (0 == _tcsicmp(argv[1], _T("FINAL")))
		{
			type = Final;
			setted = true;
		}
	}
	if (argc > 2)
	{
		branding = strdup(argv[2]);
	}
	else
		branding=_T("");


	if (!setted) 
	{ 
		Help();
		_tprintf(_T("\nError! Not enough arguments.\n"));
		return 1;
	}
	else
	{

		_tprintf(_T("Build Type: "));
		
		switch(type)
		{
			case Final:
				_tprintf(_T("FINAL"));
				break;
			case Beta:
				_tprintf(_T("BETA"));
				break;
			case Night:
				_tprintf(_T("NIGHT"));
				break;
		}
		_tprintf(_T("\n"));
		_tprintf(_T("Branding: %s\n"), branding);
		_tprintf(_T("\n"));
	}
	InitData();
	_tprintf(_T("Reading version info...                         "));
	if (!LoadInfoFile()) 
	{
		_tprintf(_T("Failed\n\nError! Unable to load version data from '%s'.\n\n"), verInfoFileName);
		return 1;
	}
    _tprintf(_T("Ok\n"));
	_tprintf(_T("Reading build info...                           "));
	if (!LoadBuildNumber())
	{
		_tprintf(_T("Failed\n\nError! Unable to load version data from '%s'.\n\n"), verBuildFileName);
		return 1;

	}
	_tprintf(_T("Ok\n"));

	if (argc > 3 &&  (0 == _tcsicmp(argv[3], _T("inc")))) ver_build++; // increment build number 

	_tprintf(_T("Checking version info data...                   "));
	if (!AllValuesSet()) 
	{
		_tprintf(_T("Failed\n\nError! Not all version data is set. Check version information files syntax.\n\n"));
		return 1;
	}
	_tprintf(_T("Ok\n"));

	bool success[9];
	for(int i = 0; i < sizeof(success)/sizeof(success[0]); i++) success[i] = true;

	_tprintf(_T("\nPatching '%s'                "), fileConstantsH);
	if (!PatchConstantsH()) { _tprintf(_T("Failed\n")); success[0] = false;}
	else _tprintf(_T("Ok\n"));

	_tprintf(_T("Patching '%s'   "), fileVerInfoNSH);
	if (!PatchVerInfoNSH()) { _tprintf(_T("Failed\n")); success[1] = false;}
	else _tprintf(_T("Ok\n"));

	_tprintf(_T("Patching '%s'                  "), fileMainH);
	if (!PatchMainH()) { _tprintf(_T("Failed\n")); success[2] = false;}
	else _tprintf("Ok\n");


	_tprintf(_T("Patching '%s'             "), fileBuildTypeH);
	if (!PatchBuildTypeH()) { _tprintf(_T("Failed\n")); success[3] = false;}
	else _tprintf("Ok\n");
	

	_tprintf(_T("Patching '%s'             "), fileWasabiCfgH);
	if (!PatchWasabiCfgH()) { _tprintf(_T("Failed\n")); success[4] = false;}
	else _tprintf("Ok\n");

	_tprintf(_T("Patching '%s'             "), fileManifest);
	if (!PatchManifest(fileManifest)) { _tprintf(_T("Failed\n")); success[5] = false;}
	else _tprintf("Ok\n");

	_tprintf(_T("Patching '%s'             "), fileManifest64);
	if (!PatchManifest(fileManifest64)) { _tprintf(_T("Failed\n")); success[6] = false;}
	else _tprintf("Ok\n");

	_tprintf(_T("Patching '%s'              "), fileFileNamesCmd);
	if (!PatchFileNamesCmd()) { _tprintf(_T("Failed\n")); success[7] = false;}
	else _tprintf("Ok\n");

	_tprintf(_T("Patching '%s'           "), fileMakeNsisCmd);
	if (!PatchMakeNsisCmd()) { _tprintf(_T("Failed\n")); success[8] = false;}
	else _tprintf("Ok\n");

	_tprintf(_T("Patching '%s'                  "), fileTalkbackIni);
	if (!PatchTalkbackIni()) { _tprintf(_T("Failed\n")); success[9] = false;}
	else _tprintf("Ok\n");


	_tprintf(_T("\n"));
	bool iserr = false;
	for (int i = 0 ; i <9 ; i++)
	{
		if (!success[i])
		{
			iserr = true;
			TCHAR const *fn;
			switch(i)
			{
				case 0:
					fn = fileConstantsH;
					break;
				case 1:
					fn = fileVerInfoNSH;
					break;
				case 2:
					fn = fileMainH;
					break;
				case 3:
					fn = fileBuildTypeH;
					break;
				case 4:
					fn = fileWasabiCfgH;
					break;
				case 5:
					fn = fileManifest;
					break;
				case 6:
					fn = fileManifest64;
					break;
				case 7:
					fn = fileFileNamesCmd;
					break;
				case 8:
					fn = fileMakeNsisCmd;
					break;
				case 9:
					fn = fileTalkbackIni;
					break;
			}
			_tprintf(_T("Error! Unable to patch file - '%s'.\n"), fn);

		}
	}
	
	if (iserr)
	{
		_tprintf(_T("\nData patched with errors.\n\n"));
	}
	else
	{
		_tprintf(_T("Data patched successfully.\n\n"));
	}
	
	return 0;
}
void InitData(void)
{	
	ver_major = 5;
	ver_minor = 0;
	ver_minor2 = 0;
	ver_build = 0;
	ver_api[0] = 0;
}

bool LoadInfoFile(void)
{
	FILE* pstream;
	if( (pstream = _tfsopen( verInfoFileName, "r", 0x20 )) == NULL )
		return false;
	fseek( pstream, 0L, SEEK_SET );

	TCHAR line[LINE_LENGTH];
	while( !feof(pstream) )
	{
		_fgetts(line,LINE_LENGTH,pstream);
		RemoveCrap(line, (int)_tcslen(line));
		ParseLine(line);
    }
	fclose(pstream);
	
	return true;
}


bool LoadBuildNumber(void)
{
	FILE* pstream;
	if( (pstream = _tfsopen( verBuildFileName, "r", 0x20 )) == NULL )
		return false;
	fseek( pstream, 0L, SEEK_SET );

	TCHAR line[LINE_LENGTH];
	
	while( !feof(pstream) )
	{
		_fgetts(line,LINE_LENGTH,pstream);
		if (_tcsstr(line, _T("DG_BUILD_NUMBER")) != NULL)
		{
			TCHAR *end;
			end = line + (int)_tcslen(line) -1;
			while (end[0] <= 32) {end[0] = 0; end--;}
			while ( end[0] >= _T('0') && end[0] <= _T('9') ) end--;
			if ((int)_tcslen(line) > 0) 
			{
				_stscanf(end, "%d", &ver_build);
			}

		}
		
    }
	fclose(pstream);
	
	return true;
}
void ParseLine(TCHAR* line)
{
	TCHAR *name, *value;
	int pos = 0;
	while (line[pos] != _T('=') && line[pos] != 0) pos++;
	if (pos == 0) return; // no name
	name = line;
	line[pos] = 0;
	value = line + pos +1;

	if (0 == _tcscmp(name, _T("VERSION_MAJOR"))) _stscanf(value, "%d", &ver_major);
	else if (0 == _tcscmp(name, _T("VERSION_MINOR"))) _stscanf(value, "%d", &ver_minor);

	// look if VERSION_MINOR_SECOND more than one char length only first one goes to the number 
	else if (0 == _tcscmp(name, _T("VERSION_MINOR_SECOND"))) 
	{
		_tcscpy(ver_minor2_full, value);
		if (_tcslen(value) > 1) value[1] = 0x00;

		if (!_tcscmp(ver_minor2_full, _T("0")))
			_tcscpy(ver_minor2_full, _T(""));

		_stscanf(value, "%d", &ver_minor2);
	}

	else if (0 == _tcscmp(name, _T("VERSION_API"))) _tcscpy(ver_api, value);
	
}

TCHAR* RemoveCrap(TCHAR* str, int len)
{	
	int pos = 0;
	while (pos < len)
	{
		if (str[pos] < 33)
		{
			for (int i = pos + 1; i < len; i++)	str[i -1] = str[i];
			str[len-1] = 0;
			len --;
		}
		else pos++;
	}
	return str;
}

bool AllValuesSet(void)
{
	return !((ver_major > -1) && (ver_minor > -1) && (ver_minor2 > -1) && (ver_build > -1) > (ver_api[0] > 0));
}

bool PatchConstantsH(void)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileConstantsH, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	const TCHAR* lookFor =  _T("#define   DG_VERSION /*CFGMGMT_VERSION*/");
	TCHAR line[LINE_LENGTH];
	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;
		if (NULL != _tcsstr(line, lookFor))
		{
			TCHAR *data;
			data = line + (int)_tcslen(lookFor);
			const TCHAR ending[128] = _T("\"\n\0");

			TCHAR newStr[256];
			_stprintf(newStr, _T("   \"%d.%d.%d"), ver_major, ver_minor, ver_minor2);

			switch(type)
			{
				case Beta:
					_tcscat(newStr,_T(" Beta"));
					break;
				case Night:
					_tcscat(newStr,_T(" Nightly"));
					break;
			}
												
			_tcscat(newStr, ending);
			_tcscpy(data, newStr);
		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fileConstantsH, "w+", streamIn) == NULL ) return false;

	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}
bool PatchVerInfoNSH(void)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileVerInfoNSH, "rb", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	
	wchar_t line[LINE_LENGTH];
	wchar_t *data;
	const wchar_t ending[] = L"\"\n\0";
	wchar_t newStr[256];
	const wchar_t lookMajor[] = L"VERSION_MAJOR";
	const wchar_t lookMin[] = L"VERSION_MINOR";
	const wchar_t lookMin2[] = L"VERSION_MINOR_SECOND";
	const wchar_t lookMin3[] = L"VERSION_MINOR_SECOND_SHORT";
	const wchar_t lookBuild[] = L"BUILD_NUM";
	wchar_t *start;
	wchar_t BOM = 0xFEFF;
	fwrite(&BOM, sizeof(BOM), 1, streamOut);
	switch(type)
	{
		case Beta:
			fputws(L"!define BETA\n\0", streamOut);
			break;
		case Night:
			fputws(L"!define NIGHT\n\0", streamOut);
			break;
	}
	fread(&BOM, 2, 1, streamIn);
	while( !feof(streamIn) )
	{
		if (NULL == fgetws(line,LINE_LENGTH, streamIn)) continue;
		if (NULL != wcsstr(line, L"!define"))
		{
			
			if (NULL != (start = wcsstr(line, lookMajor)))
			{
				data = start + (int)wcslen(lookMajor);
				swprintf(newStr, L"   \"%d%s", ver_major, ending);
				wcscpy(data, newStr);
			}
			else if (NULL != (start = wcsstr(line, lookMin3)))
			{
				data = start + (int)wcslen(lookMin3);
				swprintf(newStr, L"   \"%d%s", ver_minor2, ending);
				wcscpy(data, newStr);
			}
			else if (NULL != (start = wcsstr(line, lookMin2)))
			{
				data = start + (int)wcslen(lookMin2);
				swprintf(newStr, L"   \"%S%s", ver_minor2_full, ending);
				wcscpy(data, newStr);
			}
			else if (NULL != (start = wcsstr(line, lookMin)))
			{
				data = start + (int)wcslen(lookMin);
				if (L' ' == data[0])
				{
				 swprintf(newStr, L"   \"%d%s", ver_minor, ending);
				 wcscpy(data, newStr);
				}
			}
			else if (NULL != (start = wcsstr(line, lookBuild)))
			{
				data = start + (int)wcslen(lookBuild);
				swprintf(newStr, L"   \"%d%s", ver_build, ending);
				wcscpy(data, newStr);
			}
			else if (NULL != wcsstr(line, L"BETA") || NULL != wcsstr(line, L"NIGHT")) continue; // ignore it - we will define it on our one :)
		}
		fputws(line, streamOut);
    }
	fputws( L"\n\0", streamOut);
	fflush(streamOut);
	if( _tfreopen(fileVerInfoNSH, "w+b", streamIn) == NULL ) return false;
	
	fwcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}
bool PatchMainH(void)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileMainH, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	
	TCHAR line[LINE_LENGTH];
	TCHAR *data;
	const TCHAR ending[128] = _T("\"\n\0");
	TCHAR version[128]; 
	TCHAR versionStr[128]; 

	TCHAR newStr[256];
	const TCHAR lookBuildNumber[128] = _T("#define BUILD_NUMBER");
		const TCHAR lookVer[128] = _T("#define APP_VERSION");
	const TCHAR lookAPI[128] = _T("_NUM");
	const TCHAR lookStr[128] = _T("_STRING");
	TCHAR *start;

	_stprintf(version, _T("%d.%d%s"), ver_major, ver_minor, ver_minor2_full);
	/*if (Final == type)
	{
		_stprintf(versionStr, _T("%d.%d%s"), ver_major, ver_minor, ver_minor2_full);
	}
	else*/
	{
		_stprintf(versionStr, _T("%d.%d%s Build %d"), ver_major, ver_minor, ver_minor2_full, ver_build);
	}
	
	switch(type)
	{
		case Beta:
			_tcscat(versionStr,_T(" Beta"));
			break;
		case Night:
			_tcscat(versionStr,_T(" Nightly"));
			break;
	}
	
	
	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;
		if (NULL != _tcsstr(line, lookVer) )
		{
if (NULL != (start = _tcsstr(line, lookAPI)))
			{
				data = start + (int)_tcslen(lookAPI);
				_stprintf(newStr, _T(" %s\n\0"), ver_api);
				_tcscpy(data, newStr);
			}
			else if (NULL != (start = _tcsstr(line, lookStr)))
			{
				data = start + (int)_tcslen(lookStr);
				_stprintf(newStr, _T(" \"%s%s"), versionStr, ending);
				_tcscpy(data, newStr);
			}
			else 
			{
				data = line + (int)_tcslen(lookVer);
				if (_T(' ') == data[0] ) // just a version 
				{
					_stprintf(newStr, _T(" \"%s%s"), version, ending);
					_tcscpy(data, newStr);
				}
			}
		}
		else if (NULL != _tcsstr(line, lookBuildNumber))
		{
			data = line + (int)_tcslen(lookBuildNumber);
			if (_T(' ') == data[0] ) // just a version 
			{
					_stprintf(newStr, _T(" %d"), ver_build);
					_tcscpy(data, newStr);
			}
		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fileMainH, "w+", streamIn) == NULL ) return false;
	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}




bool PatchTalkbackIni(void)
{
FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileTalkbackIni, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	
	TCHAR line[LINE_LENGTH];
	const TCHAR ending[128] = _T("\"\n\0");


	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;
		if (line == _tcsstr(line, _T("BuildID = \"")))
		{
			_stprintf(line, _T("BuildID = \"%d\"\n\0"), ver_build);
			
		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fileTalkbackIni, "w+", streamIn) == NULL ) return false;
	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}



bool PatchBuildTypeH(void)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileBuildTypeH, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	
	TCHAR line[LINE_LENGTH];
	const TCHAR ending[128] = _T("\"\n\0");

	const TCHAR lookBeta[128] = _T("#define BETA");
	const TCHAR lookNight[128] = _T("#define NIGHT");
	const TCHAR lookInternal[128] = _T("#define INTERNAL");
	const TCHAR lookNokia[128] = _T("#define NOKIA");
	
	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;
		if (NULL != _tcsstr(line, lookBeta)) 
		{
			if (Beta == type) _stprintf(line, _T("%s\n\0"), lookBeta);
			else  _stprintf(line, _T("/*%s*/\n\0"), lookBeta);
			
		}
		else if (NULL != _tcsstr(line, lookNight)) 
		{
			if (Night == type) _stprintf(line, _T("%s\n\0"), lookNight);
			else  _stprintf(line, _T("/*%s*/\n\0"), lookNight);
			
		}
		else if (NULL != _tcsstr(line, lookInternal)) 
		{
			if (Night == type) _stprintf(line, _T("%s\n\0"), lookInternal);
			else  _stprintf(line, _T("/*%s*/\n\0"), lookInternal);
			
		}
		else if (NULL != _tcsstr(line, lookNokia)) 
		{
			if (!stricmp(branding, "NOKIA")) _stprintf(line, _T("%s\n\0"), lookNokia);
			else  _stprintf(line, _T("/*%s*/\n\0"), lookNokia);

		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fileBuildTypeH, "w+", streamIn) == NULL ) return false;
	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
	
}



bool PatchWasabiCfgH(void)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileWasabiCfgH, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	
	TCHAR line[LINE_LENGTH];
	const TCHAR ending[128] = _T("\"\n\0");

	const TCHAR lookBeta[128] = _T("#define BETA");
	const TCHAR lookNight[128] = _T("#define NIGHT");
	const TCHAR lookNokia[128] = _T("#define NOKIA");
	
	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;
		if (NULL != _tcsstr(line, lookBeta)) 
		{
			if (Beta == type) _stprintf(line, _T("%s\n\0"), lookBeta);
			else  _stprintf(line, _T("/*%s*/\n\0"), lookBeta);
			
		}
		else if (NULL != _tcsstr(line, lookNight)) 
		{
			if (Night == type) _stprintf(line, _T("%s\n\0"), lookNight);
			else  _stprintf(line, _T("/*%s*/\n\0"), lookNight);
			
		}
		else if (NULL != _tcsstr(line, lookNokia)) 
		{
			if (!stricmp(branding, "NOKIA")) _stprintf(line, _T("%s\n\0"), lookNokia);
			else  _stprintf(line, _T("/*%s*/\n\0"), lookNokia);

		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fileWasabiCfgH, "w+", streamIn) == NULL ) return false;
	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}



bool PatchFileNamesCmd(void)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileFileNamesCmd, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	TCHAR *data;
	TCHAR newStr[LINE_LENGTH];
	TCHAR line[LINE_LENGTH];
	const TCHAR ending[] = _T("\n\0");

	const TCHAR lookStr[] = _T("SET INSTALL_NAME");
	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;
		if (NULL != _tcsstr(line, lookStr)) 
		{
			data = line + (int)_tcslen(lookStr);
			_stprintf(newStr, _T("=winamp%d%d%s"), ver_major, ver_minor, ver_minor2_full);
			TCHAR tmp[64];
			switch(type)
			{
				case Beta:
				case Night:
					_stprintf(tmp, ((Beta == type) ? "_%04d_beta" : "_%04d_nightly"), ver_build);	
					_tcscat(newStr,tmp);
					break;
			}
			_tcscat(newStr, ending);
			_tcscpy(data, newStr);
			
		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fileFileNamesCmd, "w+", streamIn) == NULL ) return false;
	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}



bool PatchMakeNsisCmd(void)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fileMakeNsisCmd, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	TCHAR *data;
	TCHAR newStr[LINE_LENGTH];
	TCHAR line[LINE_LENGTH];
	const TCHAR ending[] = _T("\n\0");

	const TCHAR *szLookTable[] = 
	{
		_T("SET WINAMP_VERSION_MAJOR"),
		_T("SET WINAMP_VERSION_MINOR_SECOND"),
		_T("SET WINAMP_VERSION_MINOR"),
		
	};
				
	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;

		for (int i = 0; i < sizeof(szLookTable)/sizeof(szLookTable[0]); i++)
		{
			TCHAR *p = _tcsstr(line, szLookTable[i]);
			if (NULL != p && *(p + _tcslen(szLookTable[i])) != _T('_')) 
			{
				data = line + (int)_tcslen( szLookTable[i]);
				switch(i)
				{
					case 0: _stprintf(newStr, _T("=%d"), ver_major); break;
					case 1: _stprintf(newStr, _T("=%s"), ver_minor2_full); break;
					case 2: _stprintf(newStr, _T("=%d"), ver_minor); break;
				}
				_tcscat(newStr, ending);
				_tcscpy(data, newStr);
			}
		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fileMakeNsisCmd, "w+", streamIn) == NULL ) return false;
	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}

bool PatchManifest(const TCHAR *fn)
{
	FILE *streamIn, *streamOut;
	if( (streamIn = _tfsopen( fn, "r", 0x20 )) == NULL ) return false;
	fseek( streamIn, 0L, SEEK_SET );

	streamOut = tmpfile();
	fseek( streamOut, 0L, SEEK_SET );

	
	TCHAR line[LINE_LENGTH];
	TCHAR *data;
	const TCHAR ending[128] = _T("\"\n\0");
	TCHAR version[128]; 

	TCHAR newStr[256];
	const TCHAR lookSec[128] = _T("<assemblyIdentity");
	const TCHAR lookSecEnd[4] = _T("/>");
	const TCHAR lookVer[128] = _T("version");
		
	_stprintf(version, _T("%d.%d.%d.%d"), ver_major, ver_minor, ver_minor2, ver_build);

	bool fixed = false;
	bool inSection = false;
	TCHAR *start;
	while( !feof(streamIn) )
	{
		if (NULL == _fgetts(line,LINE_LENGTH, streamIn)) continue;
		if (!fixed)
		{
			if (NULL != _tcsstr(line, lookSec) )
			{
				inSection = true;
			}
			else if(NULL != _tcsstr(line, lookSecEnd) )
			{
				if (inSection)  fixed = true; // protection allows only ones to came to the section
				
				inSection = false;
			}
			else if (inSection && (NULL != (start = _tcsstr(line, lookVer))))
			{
				data = start + (int)_tcslen(lookVer);
				_stprintf(newStr, _T("=\"%s%s"), version, ending);
				_tcscpy(data, newStr);
				fixed = true;
			}

		}
		_fputts(line, streamOut);
    }
	_fputts( _T("\n\0"), streamOut);
	fflush(streamOut);
	if( _tfreopen(fn, "w+", streamIn) == NULL ) return false;
	fcopy(streamIn, streamOut);
	fclose(streamOut);
	fclose(streamIn);
	return true;
}


void fcopy (FILE *dest, FILE *source)
{
	char line[LINE_LENGTH];
	fseek(source, 0L, SEEK_SET );
	fseek(dest, 0L, SEEK_SET );
	_fgetts( line, LINE_LENGTH, source );
	while(!feof(source))
	{		
		_fputts(line,dest);
		_fgetts( line, LINE_LENGTH, source );
	}
}

void fwcopy (FILE *dest, FILE *source)
{
	wchar_t line[LINE_LENGTH];
	fseek(source, 0L, SEEK_SET );
	fseek(dest, 0L, SEEK_SET );
	fgetws( line, LINE_LENGTH, source );
	while(!feof(source))
	{		
		fputws(line,dest);
		fgetws( line, LINE_LENGTH, source );
	}
}

void Help(void)
{
	_tprintf(_T("Usage:  verctrl..exe NIGHT|BETA|FINAL [INC]\n"));
	_tprintf(_T("        NIGHT - night build\n"));
	_tprintf(_T("        BETA  - beta build\n"));
	_tprintf(_T("        FINAL - final build\n\n"));
	_tprintf(_T("        INC - increment build number\n"));


}