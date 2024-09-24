#include <precomp.h>

#include "xuigradientwnd.h"
#ifndef _WASABIRUNTIME

BEGIN_SERVICES(XuiGradient_Svc);
DECLARE_SERVICE(XuiObjectCreator<GradientWndXuiSvc>);
END_SERVICES(XuiGradient_Svc, _XuiGradient_Svc);

#ifdef _X86_
extern "C" { int _link_GradientXuiSvc; }
#else
extern "C" { int __link_GradientXuiSvc; }
#endif

#endif


enum { P_X1, P_Y1, P_X2, P_Y2, P_POINTS, P_GG };

XMLParamPair XuiGradientWnd::params[] = {
                                            {P_X1, L"GRADIENT_X1"},
                                            {P_Y1, L"GRADIENT_Y1"},
                                            {P_X2, L"GRADIENT_X2"},
                                            {P_Y2, L"GRADIENT_Y2"},
                                            {P_POINTS, L"POINTS"},
                                            {P_GG, L"GAMMAGROUP"},
                                        };
XuiGradientWnd::XuiGradientWnd()
{
	myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);	
}

void XuiGradientWnd::CreateXMLParameters(int master_handle)
{
	//XUIGRADIENTWND_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
	{
		if (params[i].id == P_GG)
			addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
		else
			addParam(myxuihandle, params[i], XUI_ATTRIBUTE_REQUIRED);
	}
}

int XuiGradientWnd::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value)
{
	if (xuihandle != myxuihandle)
		return XUIGRADIENTWND_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);
	switch (xmlattributeid)
	{
	case P_X1: setX1((float)WTOF(value)); break;
	case P_Y1: setY1((float)WTOF(value)); break;
	case P_X2: setX2((float)WTOF(value)); break;
	case P_Y2: setY2((float)WTOF(value)); break;
	case P_POINTS: setPoints(value); break;
	case P_GG: setGammaGroup(value); break;
	default:
		return 0;
	}
	invalidate();	// what the hell
	return 1;
}