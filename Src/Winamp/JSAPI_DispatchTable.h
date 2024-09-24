#pragma once
#include <vector>
#include <ocidl.h>
namespace JSAPI
{
	struct Dispatcher
	{
		Dispatcher():id(0),object(0){ name[0] = 0; }
		Dispatcher(const wchar_t *_name, DISPID _id, IDispatch *_object);
		~Dispatcher();
		wchar_t name[128];
		DISPID id;
		IDispatch *object;
	};

	typedef std::vector<JSAPI::Dispatcher*> DispatchTable;
};