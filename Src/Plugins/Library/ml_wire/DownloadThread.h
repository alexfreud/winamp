#ifndef NULLSOFT_DOWNLOADTHREADH
#define NULLSOFT_DOWNLOADTHREADH

#include "../xml/obj_xml.h"
#include "../xml/XMLDOM.h"
#include "../nu/Alias.h"
#include "api__ml_wire.h"
#include <api/service/waServiceFactory.h>


class DownloadThread
{
public:
	DownloadThread();
	virtual ~DownloadThread();

	virtual void ReadNodes(const wchar_t *url) = 0;

	int  DownloadURL(const wchar_t *url);
	void DownloadFile(const wchar_t *fileName);
protected:
	XMLDOM xmlDOM;
private:
	obj_xml *parser;
	waServiceFactory *parserFactory;

};
#endif
