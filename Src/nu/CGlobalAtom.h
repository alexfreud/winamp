#pragma once

#include  <windows.h>
class CGlobalAtom
{
public:
	CGlobalAtom(LPCWSTR name)
	{
		prop = GlobalAddAtomW(name);
	}
	~CGlobalAtom()
	{
		if (prop)
			GlobalDeleteAtom(prop);
		prop=0;
	}
	operator ATOM() { return prop; }
	operator LPCWSTR() { return (LPCWSTR) prop; }
private:
	ATOM prop;
};