#ifndef NULLSOFT_XML_OBJ_XML_H
#define NULLSOFT_XML_OBJ_XML_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class ifc_xmlreadercallback;

enum
{
OBJ_XML_SUCCESS=0,
OBJ_XML_FAILURE=1,
OBJ_XML_NOTIMPLEMENTED=2,

/* these two are for backwards compatability. we'll get rid of them eventually so don't use these values */
API_XML_SUCCESS=0,
API_XML_FAILURE=1,
};
class NOVTABLE obj_xml : public Dispatchable
{
public:
	void xmlreader_registerCallback(const wchar_t *matchstr, ifc_xmlreadercallback *callback);
	void xmlreader_unregisterCallback(ifc_xmlreadercallback *callback);
	int xmlreader_open();
	int xmlreader_open_namespace();
  void xmlreader_oldfeed(const void *data, size_t dataSize); // no error return value, for backwards compat
	int xmlreader_feed(const void *data, size_t dataSize); // call with 0, 0 to flush fed data.  use at the end of a file
	void xmlreader_close();
	void xmlreader_interrupt(); // causes parsing of the already-fed data to stop, and picks up with any new data you feed
	void xmlreader_resume();  // call resume when you're ready to go back to the already-fed data
	void xmlreader_reset(); // call to allow an existing obj_xml object to parse a new file.  keeps your existing callbacks
	void xmlreader_setEncoding(const wchar_t *encoding); // call to manually set encoding (maybe from HTTP headers)

	/** by default, callback matches are not case sensitive.
	 ** also, the xmlpath value sent to callbacks is convertered to UPPERCASE
 	 ** although this behaviour might not make sense, it is the default for compatability reasons.
   ** call this function to make matches case sensitive and to make the object pass you the xmlpath "as-is"
	 **/
	int xmlreader_setCaseSensitive(); // makes the callback matching case sensitive.  call this before registering callbacks. 
	DISPATCH_CODES 
	{
	    OBJ_XML_REGISTERCALLBACK = 0,
	    OBJ_XML_UNREGISTERCALLBACK = 10,
			OBJ_XML_OPEN = 20,
			OBJ_XML_OPEN2 = 21,
      OBJ_XML_OLDFEED =30,
			OBJ_XML_FEED = 31,
			OBJ_XML_CLOSE = 40,
			// OBJ_XML_CLONE = 50,
			OBJ_XML_INTERRUPT = 60,
			OBJ_XML_RESUME = 70,
			OBJ_XML_RESET = 80,
			OBJ_XML_SETENCODING = 90,
			OBJ_XML_SETCASESENSITIVE=100,
	};
};

inline void obj_xml::xmlreader_registerCallback(const wchar_t *matchstr, ifc_xmlreadercallback *callback)
{
	_voidcall(OBJ_XML_REGISTERCALLBACK, matchstr, callback);
}

inline void obj_xml::xmlreader_unregisterCallback(ifc_xmlreadercallback *callback)
{
	_voidcall(OBJ_XML_UNREGISTERCALLBACK, callback);
}

inline int obj_xml::xmlreader_open()
{
	return _call(OBJ_XML_OPEN, (int)OBJ_XML_FAILURE);
}

inline int obj_xml::xmlreader_open_namespace()
{
	return _call(OBJ_XML_OPEN2, (int)OBJ_XML_FAILURE);
}

inline void obj_xml::xmlreader_oldfeed(const void *data, size_t dataSize)
{
	_voidcall(OBJ_XML_OLDFEED, data, dataSize);
}

inline int obj_xml::xmlreader_feed(const void *data, size_t dataSize)
{
	return _call(OBJ_XML_FEED, (int)OBJ_XML_FAILURE, data, dataSize);
}

inline void obj_xml::xmlreader_close()
{
	_voidcall(OBJ_XML_CLOSE);
}

inline void obj_xml::xmlreader_interrupt()
{
	_voidcall(OBJ_XML_INTERRUPT);
}

inline void obj_xml::xmlreader_resume()
{
	_voidcall(OBJ_XML_RESUME);
}

inline void obj_xml::xmlreader_reset()
{
	_voidcall(OBJ_XML_RESET);
}

inline void obj_xml::xmlreader_setEncoding(const wchar_t *encoding)
{
	_voidcall(OBJ_XML_SETENCODING, encoding);
}

inline int obj_xml::xmlreader_setCaseSensitive()
{
	return _call(OBJ_XML_SETCASESENSITIVE, (int)OBJ_XML_NOTIMPLEMENTED);
}

// {3DB2A390-BE91-41f3-BEC6-B736EC7792CA}
static const GUID obj_xmlGUID = 
{ 0x3db2a390, 0xbe91, 0x41f3, { 0xbe, 0xc6, 0xb7, 0x36, 0xec, 0x77, 0x92, 0xca } };

extern obj_xml *xmlApi;

#endif
