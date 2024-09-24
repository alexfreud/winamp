#pragma once

#include <windows.h>
#include "../nu/Alias.h"
#include <vector>
#include <map>
class MapUnicodeComp
{
public:

	// CSTR_LESS_THAN            1           // string 1 less than string 2
	// CSTR_EQUAL                2           // string 1 equal to string 2
	// CSTR_GREATER_THAN         3           // string 1 greater than string 2
	bool operator()(const wchar_t* str1, const wchar_t* str2) const
	{
		return (CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 0, str1, -1, str2, -1)-2) ==  CSTR_LESS_THAN;
	}
};

class XMLNode
{
public:
	typedef std::map<const wchar_t *, wchar_t*, MapUnicodeComp> PropMap;
	typedef std::vector<XMLNode*> NodeList;
	typedef std::map<const wchar_t *, NodeList*, MapUnicodeComp> NodeMap;

	XMLNode();
	~XMLNode();
	const XMLNode *Get(const wchar_t *) const;
	const NodeList *GetList(const wchar_t *) const;
	const bool Present(const wchar_t *) const;
	void SetProperty(const wchar_t *prop, const wchar_t *value);
	const wchar_t *GetProperty(const wchar_t *prop) const;
	const wchar_t *GetContent() const;
	void SetContent_Own(wchar_t *new_content);
	void AppendContent(wchar_t *append);
	void AddNode(const wchar_t *name, XMLNode *new_node);
	XMLNode *parent;

private:
	PropMap properties;
	wchar_t *content;
	size_t content_len; // number of characters in curtext, not including null terminator
	NodeMap nodes;	
};