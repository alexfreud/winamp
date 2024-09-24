#include "ParamList.h"

ParamList::~ParamList()
{
	for (auto v : parms_list)
	{
		delete v;
	}
	parms_list.clear();
}

const wchar_t *ParamList::getItemName(int i) 
{
	if(((size_t)i)>=getNbItems())
		return L"";
	return 
	parms_list[i]->parm;
}

const wchar_t *ParamList::getItemValueIndex(int i) 
{
	if(((size_t)i)>=getNbItems()) 
		return L"";
	return 
		parms_list[i]->value;
}

const wchar_t *ParamList::getItemValue(const wchar_t *name) 
{
	for(size_t i=0;i!=getNbItems();i++)
		if(!_wcsicmp(parms_list[i]->parm, name))
			return parms_list[i]->value;
	return NULL;
}

const wchar_t *ParamList::enumItemValues(const wchar_t *name, int nb)
{
	int f=0;
	for(size_t i=0;i!=getNbItems();i++)
		if(!_wcsicmp(parms_list[i]->parm, name))
			if(f==nb)
				return parms_list[i]->value;
		else f++;
	return NULL;
}

int ParamList::getItemValueInt(const wchar_t *name, int def) 
{
	for(size_t i=0;i!=getNbItems();i++)
		if(!_wcsicmp(parms_list[i]->parm, name))
		{
			return (parms_list[i]->value ? _wtoi(parms_list[i]->value) : def);
		}
	return def;
}

size_t ParamList::getNbItems() 
{
	return parms_list.size();
}

void ParamList::addItem(const wchar_t *parm, const wchar_t *value) 
{
	parms_struct *p= new parms_struct;
	p->parm = _wcsdup(parm);
	p->ownValue = true;
	p->value = _wcsdup(value);
	parms_list.push_back(p);
}

void ParamList::removeItem(const wchar_t *parm) 
{
	//for (size_t i=0;i!=parms_list.size();i++) 
	//{
	//	parms_struct *s = parms_list[i];
	//	if (!_wcsicmp(parm, s->parm)) 
	//	{
	//		delete s;
	//		parms_list.eraseindex(i);
	//		i--;
	//	}
	//}

	for (auto it = parms_list.begin(); it != parms_list.end(); it++)
	{
		parms_struct* s = *it;
		if (!_wcsicmp(parm, s->parm))
		{
			delete s;
			it = parms_list.erase(it);
		}
	}

}

int ParamList::findItem(const wchar_t *parm) 
{
	for(size_t i=0;i!=getNbItems();i++)
		if(!_wcsicmp(parms_list[i]->parm, parm))
			return (int)i;
	return -1;
}

#define CBCLASS ParamList
START_DISPATCH;
CB(XMLREADERPARAMS_GETITEMNAME, getItemName)
CB(XMLREADERPARAMS_GETITEMVALUE, getItemValueIndex)
CB(XMLREADERPARAMS_GETITEMVALUE2, getItemValue)
CB(XMLREADERPARAMS_ENUMITEMVALUES, enumItemValues)
CB(XMLREADERPARAMS_GETITEMVALUEINT, getItemValueInt)
CB(XMLREADERPARAMS_GETNBITEMS, getNbItems)
END_DISPATCH;
#undef CBCLASS