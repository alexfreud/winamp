#pragma once
#include "FlashDispInterface.h"

class FLVExternalInterface : public FlashDispInterface
{
private:
	BSTR ExternalInterface_call(BSTR xml);
};