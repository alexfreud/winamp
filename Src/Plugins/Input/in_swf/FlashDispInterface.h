#pragma once
#include <wtypes.h>
class FlashDispInterface
{
public:
	virtual BSTR ExternalInterface_call(BSTR xml) = 0;
};