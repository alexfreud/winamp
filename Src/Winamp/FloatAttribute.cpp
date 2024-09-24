#include "main.h"
#include "attributes.h"

_float::_float()
{
	value = 0;
}

_float::_float(float defaultValue)
{
	value = defaultValue;
}

intptr_t _float::operator =(intptr_t uintValue) 
{
	value = static_cast<float>(uintValue); 
	return static_cast<intptr_t>(value); 
}

float _float::operator =(float uintValue) 
{
	value = uintValue; 
	return value; 
}

#define CBCLASS _float
START_DISPATCH;
CB(IFC_CONFIGITEM_GETINT, GetInt)
CB(IFC_CONFIGITEM_GETFLOAT, GetFloat)
END_DISPATCH;