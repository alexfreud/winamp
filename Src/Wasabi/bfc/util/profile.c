#pragma warn -aus
//----------------------------------------------------------------------------
// MS Windows Style .ini File Interface for C++
// This is the first version.
// Programed by xuyifeng, 1995.10, china
//----------------------------------------------------------------------------
// Test history:
//         Compiler              OS                          TEST
//         ---------------------------------------------------------------
//         Watcom  C++ 10.0a     Rational System DOS4GW      100% tested
//         Borland C++ 3.1       DOS                         100% tested
// 10/4/97 - Bugfix: added fclose(is); at the end of getProfileString -lonerunnr/ags
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <string.h>
#include <ctype.h>
#ifdef LINUX
#include <X11/Xos.h>
#define _strnicmp strncasecmp
#endif

#include "profile.h"

#define False 0
#define True  1

//------------------------------------------------------------------------------
// titlePos: get a section title position & length in a string.
//------------------------------------------------------------------------------
static char *titlePos( char *buf, int *len )
{
	char *p = buf, *q;

	while( *p && isspace(*p) ) p++;
	if( *p != '[' )
		return 0;

	q = p+1;
	while( *q && *q != ']' ) q++;
	if( *q != ']' )
		return 0;
	if( len )
		*len = (int)(q - p - 1);
	return p+1;
}

//------------------------------------------------------------------------------
// isTitleLine: check if a string is a section title line
//------------------------------------------------------------------------------
static int isTitleLine( char *bufPtr )
{
	return titlePos( bufPtr, 0 ) != 0;
}

//------------------------------------------------------------------------------
// containTitle: check if a string contain a section a title
//------------------------------------------------------------------------------
static int containTitle( char *buf, const char *section )
{
	int len = 0;
	char *p = titlePos(buf, &len);
	if (p)
	{
		if( strlen( section ) == len && _strnicmp( section, p, len ) == 0 )
			return True;
	}
	return False;
}

//------------------------------------------------------------------------------
// gotoSection: move file position to start line of a section
//------------------------------------------------------------------------------
static int gotoSection( FILE *is, const char *section )
{
	char line[256] = {0};
	while( fgets(line, 256, is) != NULL)
		if( containTitle( line, section ) )
			return True;
	return False;
}

//------------------------------------------------------------------------------
// textPos: get content's position of a entry
//------------------------------------------------------------------------------
static char *textPos( char *buf, const char *entry )
{
	if( buf[0] == ';' ) // it is comment line
		return 0;

	char *p = strchr( buf, '=' );
	if (!p)
		return 0;

	int len = (int)(p - buf);
	if( strlen(entry) == len && _strnicmp( buf, entry, len ) == 0 )
		return p+1;

	return 0;
}

//------------------------------------------------------------------------------
// stripQuotationChar: strip a pair of quotation chars in a string
//------------------------------------------------------------------------------
static void stripQuotationChar( char *buf )
{
	char *p = buf;
	while( *p && isspace(*p) ) p++;

	if( !(*p == '\"' || *p == '\'') )
		return;

	char *q = p+strlen(p);
	while( *q != *p && q > p ) q--;
	if( q == p )
		return;
	int len = (int)(q - p - 1);
	memmove( buf, p+1, len );
	buf[len] = 0;
}

//------------------------------------------------------------------------------
// readEntry: read content of entry
//------------------------------------------------------------------------------
static int readEntry( FILE *is, const char *entry, char *buf, int bufSize,
					  int strip )
{
	char lineBuf[256] = {0};
	char *cur = buf;
	*cur = '\0';
	int len  = -1;
	while( fgets(lineBuf, 256, is) != NULL)
	{
		if (lineBuf[strlen(lineBuf)-1] == '\n')
			lineBuf[strlen(lineBuf)-1] = 0;

		if( isTitleLine( lineBuf ) )       // section is ended
			break;

		char *p = textPos( lineBuf, entry );     // not equal this entry
		if( p == 0 )
			continue;

		if( strip )
			stripQuotationChar( p );

		len = strlen(p);
		if( bufSize-1 < len )
			len = bufSize-1;

		strncpy( cur, p, len );
		cur[len] = 0;
		break;
	}

	return len;
}

//------------------------------------------------------------------------------
// getProfileString:
//------------------------------------------------------------------------------
int GetPrivateProfileString( const char *section,
					  const char *entry,
					  const char *defaultString,
					  char *buffer,
					  int   bufLen,
					  const char *fileName )
{
    FILE *is;
	int len = -1;

    is = fopen(fileName, "rt");

	if( is && gotoSection( is, section ) )
		len = readEntry(is, entry, buffer, bufLen, True);

	if (len < 0) //can not read entry, use default string
	{
		strncpy( buffer, defaultString, bufLen-1 );
		buffer[bufLen-1] = 0;
		len = strlen(buffer);
	}
	if (is) fclose(is);
	return len;
}

//----------------------------------------------------------------------------
// getProfileInt:
//----------------------------------------------------------------------------
long GetPrivateProfileInt( const char *section,
				   const char *entry,
				   long defaultInt,
				   const char *fileName )
{
	char buf[256];
    char iBuf[34];   //"34" is max space "_itoa" required under 32 bit C++

	sprintf(iBuf, "%d", defaultInt);
	GetPrivateProfileString( section, entry, iBuf, buf, 256, fileName );
	return atol( buf );
}

static void writeEntry( FILE *os, const char *entry, const char *string )
{
    fprintf(os, "%s=%s\n", entry, string);
}

//------------------------------------------------------------------------------
// writeProfileString:
//------------------------------------------------------------------------------
int WritePrivateProfileString( const char *section,
						const char *entry,
						const char *string,
						const char *fileName )
{
	char path [8192] = {0};
	char drive[256] = {0};
	char dir  [256] = {0};
	char file [256] = {0};
	char ext  [256] = {0};
	char buf  [256] = {0};

	int titleFound;

	// work better on network!
	_splitpath( path, drive, dir, file, ext );
	_makepath(  path, drive, dir, mkstemp("iniXXXXXX"), "" );

	FILE *is = fopen(fileName, "rt");
	FILE *os = fopen(path, "wt");

	if(!is || !os || entry == 0)     // maybe can not create file or invalid entry
	{
		if (is) fclose(is);
		if (os) fclose(os);
		return 0;
	}

	titleFound = False;
	if (is)
	{
		while (fgets(buf, 256, is) != NULL)
		{
			fputs(buf, os);
			if( containTitle(buf, section) )
			{
				titleFound = True;
				break;
			}
		}
	}

	if (!titleFound)   // add section
	{
		fprintf(os, "[%s]\n", section);
		writeEntry( os, entry, string );
	}
	else
	{
		while (fgets(buf, 256, is) != NULL)
		{
			if (isTitleLine(buf))     // section ended, but still not found the entry
				break;

			if (textPos(buf, entry))  // entry found, so rewrite it
			{
				break;
			}
			fputs(buf, os);
		}

		writeEntry(os, entry, string);

		if (is)
		{
			while(fgets(buf, 256, is) != NULL)  // copy left lines
				fputs(buf, os);
		}
	}
	if (is) fclose(is);
	if (os) fclose(os);
	unlink(fileName);
	rename(path, fileName);
	return strlen(string);
}

#pragma warn .aus