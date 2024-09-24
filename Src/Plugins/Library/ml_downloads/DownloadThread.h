#ifndef NULLSOFT_DOWNLOADTHREADH
#define NULLSOFT_DOWNLOADTHREADH

#include "../xml/obj_xml.h"
#include "../xml/XMLDOM.h"
#include "../nu/Alias.h"
#include "api__ml_downloads.h"
#include <api/service/waServiceFactory.h>


class DownloadThread
{
public:
	DownloadThread();
	virtual ~DownloadThread();

	virtual void ReadNodes(const wchar_t *url) = 0;

	void DownloadFile(const wchar_t *fileName);
protected:
	XMLDOM xmlDOM;
private:
	obj_xml *parser;
	waServiceFactory *parserFactory;

};
#endif
