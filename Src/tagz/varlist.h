#ifndef NULLSOFT_VARLISTH
#define NULLSOFT_VARLISTH

#include <wchar.h>
class VarList
{
private:
	struct VAR
	{
		VAR *next;
		wchar_t *name;
		wchar_t *value;
	};
	VAR *vars;
public:
	VarList() ;
	~VarList();
	wchar_t *Get(const wchar_t *name);
	void Put(const wchar_t *name, const wchar_t *value);
};

#endif