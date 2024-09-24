#ifndef NULLSOFT_AGAVE_IFC_CONFIGITEM_H
#define NULLSOFT_AGAVE_IFC_CONFIGITEM_H

#include <bfc/dispatch.h>
#include <stddef.h>
/*
notes:
The Set() functions are "public-facing", meaning that they can be called by anyone.  If you want to make your config item read-only,
then simply don't implement these.  You can always make "private" Set functions in your implementation.

SetStringInternal and GetStringInternal are written for use with classes to load and save from INI files (or XML files or whatever).
It's up to you to figure out a clever way to encode yourself.

*/

enum
{
	CONFIG_ITEM_TYPE_STRING = 0,
	CONFIG_ITEM_TYPE_INT = 1,
	CONFIG_ITEM_TYPE_UNSIGNED =2,
	CONFIG_ITEM_TYPE_BOOL =3,
	CONFIG_ITEM_TYPE_BINARY =4,
	CONFIG_ITEM_TYPE_INT_ARRAY = 5,
};

class ifc_configitem : public Dispatchable
{
protected:
	ifc_configitem() {}
	~ifc_configitem() {}
public:
	const wchar_t *GetName();
	int GetType();

	const wchar_t *GetString();
	void SetString(const wchar_t *stringValue);

	intptr_t GetInt();
	void SetInt(intptr_t intValue);

	uintptr_t GetUnsigned();
	void SetUnsigned(uintptr_t unsignedValue);

	bool GetBool();
	void SetBool(bool boolValue);

	float GetFloat();
	void SetFloat(float floatValue);

	size_t GetBinarySize();
	size_t GetBinaryData(void *data, size_t bytes); // returns bytes written
	void SetBinaryData(void *data, size_t bytes);

	size_t GetIntArrayElements();
	size_t GetIntArray(intptr_t *array, size_t elements); // returns elements written
	void SetIntArray(intptr_t *array, size_t elements);

	const wchar_t *GetStringInternal(); // gets a string suitable for saving in an INI file or XML
	void SetStringInternal(const wchar_t *internalString);

public:
	DISPATCH_CODES
	{
		IFC_CONFIGITEM_GETNAME = 10,
		IFC_CONFIGITEM_GETTYPE = 20,

		IFC_CONFIGITEM_GETSTRING= 30,
		IFC_CONFIGITEM_SETSTRING= 40,

		IFC_CONFIGITEM_GETINT= 50,
		IFC_CONFIGITEM_SETINT= 60,

		IFC_CONFIGITEM_GETUNSIGNED= 70,
		IFC_CONFIGITEM_SETUNSIGNED= 80,

		IFC_CONFIGITEM_GETBOOL= 90,
		IFC_CONFIGITEM_SETBOOL= 100,

		IFC_CONFIGITEM_GETBINARYSIZE= 110,
		IFC_CONFIGITEM_GETBINARYDATA= 120,
		IFC_CONFIGITEM_SETBINARYDATA= 130,

		IFC_CONFIGITEM_GETINTARRAYELEMENTS= 140,
		IFC_CONFIGITEM_GETINTARRAY= 150,
		IFC_CONFIGITEM_SETINTARRAY= 160,

		IFC_CONFIGITEM_GETSTRINGINTERNAL= 170,
		IFC_CONFIGITEM_SETSTRINGINTERNAL= 180,

		IFC_CONFIGITEM_GETFLOAT= 190,
		IFC_CONFIGITEM_SETFLOAT= 200,
	};
};



inline const wchar_t *ifc_configitem::GetName()
{
	return _call(IFC_CONFIGITEM_GETNAME, (const wchar_t *)0);
}

inline int ifc_configitem::GetType()
{
	return _call(IFC_CONFIGITEM_GETTYPE, (int)0);
}

inline const wchar_t *ifc_configitem::GetString()
{
	return _call(IFC_CONFIGITEM_GETSTRING, (const wchar_t *)0);
}

inline void ifc_configitem::SetString(const wchar_t *stringValue)
{
	_voidcall(IFC_CONFIGITEM_SETSTRING, stringValue);
}


inline intptr_t ifc_configitem::GetInt()
{
	return _call(IFC_CONFIGITEM_GETINT, (intptr_t)0);
}
#pragma warning(push)
#pragma warning(disable: 4244)
inline void ifc_configitem::SetInt(intptr_t intValue)
{
	_voidcall(IFC_CONFIGITEM_SETINT, intValue);
}
#pragma warning(pop)

inline uintptr_t ifc_configitem::GetUnsigned()
{
	return _call(IFC_CONFIGITEM_GETUNSIGNED, (uintptr_t)0);
}

inline void ifc_configitem::SetUnsigned(uintptr_t unsignedValue)
{
	_voidcall(IFC_CONFIGITEM_SETUNSIGNED, unsignedValue);
}


inline bool ifc_configitem::GetBool()
{
	return _call(IFC_CONFIGITEM_GETBOOL, (bool)false);
}

inline void ifc_configitem::SetBool(bool boolValue)
{
	_voidcall(IFC_CONFIGITEM_SETBOOL, boolValue);
}

inline size_t ifc_configitem::GetBinarySize()
{
	return _call(IFC_CONFIGITEM_GETBINARYSIZE, (size_t)0);
}

inline size_t ifc_configitem::GetBinaryData(void *data, size_t bytes)
{
	return _call(IFC_CONFIGITEM_GETBINARYDATA, (size_t)0, data, bytes);
}

inline void ifc_configitem::SetBinaryData(void *data, size_t bytes)
{
	_voidcall(IFC_CONFIGITEM_SETBINARYDATA, data, bytes);
}

inline size_t ifc_configitem::GetIntArrayElements()
{
	return _call(IFC_CONFIGITEM_GETINTARRAYELEMENTS, (size_t)0);
}

inline size_t ifc_configitem::GetIntArray(intptr_t *array, size_t elements)
{
	return _call(IFC_CONFIGITEM_GETINTARRAY, (size_t)0, array, elements);
}
inline void ifc_configitem::SetIntArray(intptr_t *array, size_t elements)
{
	_voidcall(IFC_CONFIGITEM_SETINTARRAY, array, elements);
}

inline const wchar_t *ifc_configitem::GetStringInternal()
{
	return _call(IFC_CONFIGITEM_GETSTRINGINTERNAL, (const wchar_t *)0);
}
inline void ifc_configitem::SetStringInternal(const wchar_t *internalString)
{
	_voidcall(IFC_CONFIGITEM_SETSTRINGINTERNAL, internalString);
}

inline float ifc_configitem::GetFloat()
{
	return _call(IFC_CONFIGITEM_GETFLOAT, (float)0);
}

inline void ifc_configitem::SetFloat(float floatValue)
{
	_voidcall(IFC_CONFIGITEM_SETFLOAT, floatValue);
}


#endif