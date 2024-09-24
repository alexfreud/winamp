#pragma once
#include "../xml/XMLNode.h"
#include <bfc/platform/types.h>
bool PropertyIsTrue(const XMLNode *item, const wchar_t *property);
// not necessarily the opposite of PropertyIsTrue (returns false when field is empty
bool PropertyIsFalse(const XMLNode *item, const wchar_t *property);
const wchar_t *GetContent(const XMLNode *item, const wchar_t *tag);
const wchar_t *GetProperty(const XMLNode *item, const wchar_t *tag, const wchar_t *property);