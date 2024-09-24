/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "asx.h"
#include "../jnetlib/api_httpget.h"
#include "../nu/AutoChar.h"
#include "WinampPlaylist.h"
#include "../nu/AutoWide.h"
#include "api.h"

#if 0 // keep around for reference 
void ASXLoader::LoadFile(const char *filename)
{
	HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return ;

	char data[1024];
	DWORD bytesRead;

	// check for ASXv2
	if (ReadFile(file, data, 11, &bytesRead, NULL) && bytesRead)
	{
		if (bytesRead == 11	&& !_strnicmp((char *)data, "[Reference]", 11))
		{
			loadasxv2fn(filename, 1); // can pass 0 since loadasxfn() already took care of this
			CloseHandle(file);
			return ;
		}
	}
	else
	{
		CloseHandle(file);
		return ;
	}
	if (!parser)
	{
		CloseHandle(file);
		return ;
	}

	GayASX_to_XML_converter(parser, data, bytesRead); // read the small amount we read when sniffing for asxv2

	while (true)
	{

		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
			GayASX_to_XML_converter(parser, data, bytesRead);
		else
			break;
	}

	CloseHandle(file);
	parser->xmlreader_feed(0, 0);
}
#endif

