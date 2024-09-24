#include "varlist.h"
#include <windows.h>

VarList::VarList()
{
	vars = 0;
}

VarList::~VarList()
{
	VAR * p = vars;
	while (p)
	{
		VAR *n = p->next;
		free(p->name);
		free(p->value);
		delete p;
		p = n;
	}
}

wchar_t *VarList::Get(const wchar_t *name)
{
	VAR * p = vars;
	while (p)
	{
		if (!_wcsicmp(name, p->name))
			return p->value;
		p = p->next;
	}
	return 0;
}

void VarList::Put(const wchar_t *name, const wchar_t *value)
{
	VAR *p = vars;
	while (p)
	{
		if (!_wcsicmp(name, p->name))
		{
			free(p->value);
			p->value = _wcsdup(value);
			return;
		}
		p = p->next;
	}
	p = new VAR;
	p->next = vars;
	vars = p;
	p->value = _wcsdup(value);
	p->name = _wcsdup(name);
}