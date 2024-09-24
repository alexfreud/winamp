#include "FLVExternalInterface.h"
#include "../xml/obj_xml.h"
#include "api.h"
#include "SWFParameters.h"
#include "../Winamp/wa_ipc.h"
#include "main.h"
#include <api/service/waServiceFactory.h>
#include <strsafe.h>

BSTR FLVExternalInterface::ExternalInterface_call(BSTR xml)
{
	obj_xml *parser=0;
	waServiceFactory *parserFactory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (parser)
	{
		{ // artificial scope for SWFParameters 
			SWFParameters parameters(parser);		
			parser->xmlreader_open();
			parser->xmlreader_setEncoding(L"UTF-16");
			parser->xmlreader_feed(xml, wcslen(xml)*sizeof(*xml));
			parser->xmlreader_feed(0, 0);
			parser->xmlreader_close();
			if (parameters.functionName)
			{
				if (!wcscmp(parameters.functionName, L"Benski"))
				{
				}
				else if (!wcscmp(parameters.functionName, L"Ready"))
				{
					unsigned int width, height;
					if (parameters.GetUnsigned(0, &width) && parameters.GetUnsigned(1, &height))
						videoOutput->open(width, height, 0, 1.0f /*(double)x/(double)y*/, VIDEO_MAKETYPE('N','O','N','E'));
					// TODO:
					// Play (if not paused during buffering)
				}
				else if (!wcscmp(parameters.functionName, L"Complete"))
				{
					PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				}
				else if (!wcscmp(parameters.functionName, L"Metadata"))
				{
				//	MessageBox(NULL, xml, L"Flash ExternalInterface.call()", MB_OK);
					double duration;
					if (parameters.GetDouble(0, &duration))
						playLength = (int)(duration * 1000.0);
					
				}
				else if (!wcscmp(parameters.functionName, L"Buffering"))
				{
					Nullsoft::Utility::AutoLock autolock(statusGuard);
					StringCchCopy(status, 256, L"buffering");
					PostMessage(plugin.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
				}
				else if (!wcscmp(parameters.functionName, L"Playing"))
				{
					Nullsoft::Utility::AutoLock autolock(statusGuard);
					status[0]=0;
					PostMessage(plugin.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
				}
				else if (!wcscmp(parameters.functionName, L"Playhead"))
				{
					double playhead;
					if (parameters.GetDouble(0, &playhead))
						playPosition = (int)(playhead * 1000.0);
				}
			}
		}
		parserFactory->releaseInterface(parser);
	}

	return 0;
}