#include "main.h"
#include "attributes.h"

_bool_base::_bool_base() : value(false) {}

bool _bool_base::GetBool()
{
	return value;
}

void _bool_base::SetBool(bool boolValue)
{
	value = boolValue;
}

intptr_t _bool_base::GetInt()
{
	return value ? 1 : 0;
}

void _bool_base::SetInt(intptr_t intValue)
{
	value = !!intValue;
}

_bool_base::operator intptr_t()
{
	return GetInt();
}

intptr_t _bool_base::operator =(intptr_t intValue)
{
	value = !!intValue;
	return GetInt();
}

bool _bool_base::operator =(bool boolValue)
{
	value = boolValue;
	return GetBool();
}


_bool_base::operator bool()
{
	return value;
}

_bool_base::operator UINT()
{
	return value?1:0;
}

bool _bool_base::operator !()
{
	return !value;
}

/* --------------------- */

_bool::_bool(bool defaultValue)
{
	value = defaultValue;
}

#define CBCLASS _bool
START_DISPATCH;
CB(IFC_CONFIGITEM_GETBOOL, GetBool)
CB(IFC_CONFIGITEM_GETINT, GetInt)
END_DISPATCH;
#undef CBCLASS

/* --------------------- */

_mutable_bool::_mutable_bool(bool defaultValue)
{
	value = defaultValue;
}

#define CBCLASS _mutable_bool
START_DISPATCH;
CB(IFC_CONFIGITEM_GETBOOL, GetBool)
CB(IFC_CONFIGITEM_GETINT, GetInt)
VCB(IFC_CONFIGITEM_SETINT, SetInt)
VCB(IFC_CONFIGITEM_SETBOOL, SetBool)
END_DISPATCH;
#undef CBCLASS