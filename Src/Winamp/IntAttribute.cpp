#include "main.h"
#include "attributes.h"

_int::_int()
{
	value = 0;
}

_int::_int(intptr_t defaultValue)
{
	value = defaultValue;
}

intptr_t _int::operator =(intptr_t uintValue) 
{
	value = uintValue; 
	return value; 
}

#define CBCLASS _int
START_DISPATCH;
CB(IFC_CONFIGITEM_GETINT, GetInt)
CB(IFC_CONFIGITEM_GETFLOAT, GetFloat)
END_DISPATCH;