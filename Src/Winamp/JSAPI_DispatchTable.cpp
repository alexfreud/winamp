#include "JSAPI_DispatchTable.h"
#include <strsafe.h>

JSAPI::Dispatcher::Dispatcher(const wchar_t *_name, DISPID _id, IDispatch *_object)
:id(_id), object(_object)
{
	memset(name, 0, sizeof(name));

	if (NULL != object) 
		object->AddRef();

	StringCchCopyW(name, ARRAYSIZE(name), _name);
}

JSAPI::Dispatcher::~Dispatcher()
{
	if (NULL != object)
	{
		object->Release();
		object = NULL;
	}
}