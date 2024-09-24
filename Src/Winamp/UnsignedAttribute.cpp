#include "main.h"
#include "attributes.h"

_unsigned::_unsigned()
{
	value = 0;
}

_unsigned::_unsigned(uintptr_t defaultValue)
{
	value = defaultValue;
}

uintptr_t _unsigned::operator =(uintptr_t uintValue) 
{
	value = uintValue; 
	return value; 
}

#define CBCLASS _unsigned
START_DISPATCH;
CB(IFC_CONFIGITEM_GETUNSIGNED, GetUnsigned)
END_DISPATCH;