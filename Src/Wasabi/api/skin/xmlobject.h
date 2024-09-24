#ifndef __XMLOBJECT_H
#define __XMLOBJECT_H

#include <bfc/nsguid.h>
#include <bfc/ptrlist.h>
#include <bfc/dispatch.h>
#include <bfc/string/StringW.h>
#include <bfc/wasabi_std.h>

#define XML_ATTRIBUTE_IMPLIED  0
#define XML_ATTRIBUTE_REQUIRED 1

class XmlObject : public Dispatchable
{
public:
	int setXmlParam(const wchar_t *name, const wchar_t *strvalue);
	const wchar_t *getXmlParamValue(int n);
	int getXmlParam(const wchar_t *paramname);

	enum
	{
		SETXMLPARAM=10,
		GETXMLPARAMVALUE=40,
		GETXMLPARAM=50,
	};
};

inline int XmlObject::setXmlParam(const wchar_t *name, const wchar_t *strvalue)
{
	return _call(SETXMLPARAM, 0, name, strvalue);
}

inline const wchar_t *XmlObject::getXmlParamValue(int n)
{
	return _call(GETXMLPARAMVALUE, (const wchar_t *)0, n);
}

inline int XmlObject::getXmlParam(const wchar_t *paramname)
{
	return _call(GETXMLPARAM, 0, paramname);
}

class XmlObjectParam
{

public:

	XmlObjectParam(int xmlhandle, wchar_t *xmlattribute, int xmlattributeid);
	virtual ~XmlObjectParam() {}

	const wchar_t *getXmlAttribute()
	{
		return xmlattributename;
	}
	int getXmlAttributeId()
	{
		return attributeid;
	}
	int getXmlHandle()
	{
		return handle;
	}
	void setLastValue(const wchar_t *val)
	{
		lastvalue = val;
	}
	const wchar_t *getLastValue()
	{
		return lastvalue;
	}

private:
	const wchar_t *xmlattributename;
	StringW lastvalue;
	int attributeid;
	int handle;

};

class XmlObjectParamSort
{
public:
	static inline int compareItem(XmlObjectParam *p1, XmlObjectParam*p2)
	{
		return wcscmp(p1->getXmlAttribute(), p2->getXmlAttribute());
	}
	static inline int compareAttrib(const wchar_t *attrib, XmlObjectParam *item)
	{
		return WCSICMP(attrib, item->getXmlAttribute());
	}
};

struct XMLParamPair
{
	int id;
	wchar_t name[64];
};

class XmlObjectI : public XmlObject
{
public:
	XmlObjectI();
	virtual ~XmlObjectI();

	virtual int setXmlParam(const wchar_t *name, const wchar_t *strvalue); // receives from system
	virtual int setXmlParamById(int xmlhandle, int xmlattributeid, const wchar_t *name, const wchar_t *strvalue); // distributes to inheritors
	virtual const wchar_t *getXmlParamValue(int n);
	virtual int getXmlParam(const wchar_t *paramname);
	const wchar_t *getXmlParamByName(const wchar_t *paramname);
	void addParam(int xmlhandle, XMLParamPair &param, int unused=0);
	//void addXmlParam(int xmlhandle, const wchar_t *xmlattribute, int xmlattributeid);
protected:
	virtual int onUnknownXmlParam(const wchar_t *param, const wchar_t *value);
	int newXmlHandle();
	void hintNumberOfParams(int xmlhandle, int params);

private:
	XmlObjectParam *enumParam(int n)
	{
		return params[n];
	}

	PtrListInsertSorted<XmlObjectParam, XmlObjectParamSort> params;
	int handlepos;

protected:

	RECVS_DISPATCH;
};


#endif
