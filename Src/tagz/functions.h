#ifndef NULLSOFT_FUNCTIONSH
#define NULLSOFT_FUNCTIONSH

#include "string.h"
#include "varlist.h"

typedef void (*TEXTFUNC)(size_t n_src, wchar_t **src, size_t *, tagz_::string *out, VarList *vars);

struct TextFunction
{
	TEXTFUNC func;
	const wchar_t *name;
};

extern TextFunction FUNCS[];

#define TABSIZE(x) (sizeof(x)/sizeof(*x))

#endif