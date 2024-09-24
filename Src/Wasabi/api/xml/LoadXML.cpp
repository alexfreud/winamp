#include <precomp.h>
#include "LoadXML.h"
#include <bfc/file/wildcharsenum.h>
#include <wchar.h>

void LoadXmlServiceProvider(obj_xml *parser, const wchar_t *filename)
{
	/* TODO:
	svc = XmlProviderEnum(filename).getNext();
	pbuf = svc->getXmlData(filename, incpath, &p);
	SvcEnum::release(svc);
	svc = NULL;
	*/
}

// when filename begins with "buf:"
void LoadXmlBuffer(obj_xml *parser, const wchar_t *filename)
{
	filename += 4;
	if (parser->xmlreader_feed((void *)filename, wcslen(filename)*sizeof(wchar_t)) == API_XML_SUCCESS)
		parser->xmlreader_feed(0, 0);
}

bool LoadXmlFilename(obj_xml *parser, const wchar_t *filename)
{
	OSFILETYPE file = WFOPEN(filename, WF_READONLY_BINARY, NO_FILEREADERS);
//	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

//	if (file == INVALID_HANDLE_VALUE)
  	if (file == OPEN_FAILED)
		return false;

	size_t bytesRead=0;
	do
	{
		char data[4096] = {0};
		bytesRead = FREAD(data, 1, sizeof(data), file);
//		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		if (bytesRead)
		{
			if (parser->xmlreader_feed(data, bytesRead)!=API_XML_SUCCESS)
			{
//				CloseHandle(file);
				FCLOSE(file);
				return false;
			}
		}
		else
		{
			if (parser->xmlreader_feed(0, 0) != API_XML_SUCCESS)
			{
				FCLOSE(file);
//				CloseHandle(file);
				return false;
			}
			bytesRead=0;
		}		
	} while (bytesRead);

	FCLOSE(file);
//	CloseHandle(file);
	return true;
}

/*** TODO:
 ** move this to a separate file
 ** deal with wildcard filenames  (e.g. *.xml)
 */
void LoadXmlFile(obj_xml *parser, const wchar_t *filename)
{
	if (!WCSNICMP(filename, L"buf:", 4))
	{
		LoadXmlBuffer(parser, filename);
		return ;
	}
	WildcharsEnumerator e(filename);

	if (e.getNumFiles() > 0) // if we're including multiple files
	{
		for (int i = 0;i < e.getNumFiles();i++)
		{
			if (i) // don't reset the first time around
				parser->xmlreader_reset();
			LoadXmlFilename(parser, e.enumFile(i));
		}
	}
	else if (!LoadXmlFilename(parser, filename))
	{
		LoadXmlServiceProvider(parser, filename);
		return ;
	}
}
