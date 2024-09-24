#ifndef _XUIGRADIENTWND_H
#define _XUIGRADIENTWND_H

#include <api/wnd/wndclass/gradientwnd.h>

#define XUIGRADIENTWND_PARENT GradientWnd
class XuiGradientWnd : public XUIGRADIENTWND_PARENT {
public:
	static const wchar_t *xuiobject_getXmlTag() { return L"Gradient"; }
	static const char *xuiobject_getServiceName() { return "Gradient XuiObject"; }
	XuiGradientWnd();

	virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

	void parsePoints(const wchar_t *pointlist);
protected:
	void CreateXMLParameters(int master_handle);
private:
	int myxuihandle;
	static XMLParamPair params[];

};

class GradientWndXuiSvc : public XuiObjectSvc2<XuiGradientWnd> {};

#endif
