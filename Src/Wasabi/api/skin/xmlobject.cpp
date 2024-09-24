#include <precomp.h>
#include <bfc/bfc_assert.h>
#include "xmlobject.h"
#include <new>
#define PILENODE_AUTOINCREMENT 50

/*

CreateXMLParameters(int master_handle)
addParam(master_handle, this_handle, name, id)

master_handle is key into array of  name/this-handle/id 


*/
class DeleteOnClose
{
public:
	~DeleteOnClose()
	{
		allocd.freeAll();
	}
	PtrList<XmlObjectParam> allocd;
};
DeleteOnClose deleter;
template <class t_>
struct PileNode
{
	PileNode(int num)
	{
		sizePile = num;
		pile = (t_ *)MALLOC(sizeof(t_) * num);
		deleter.allocd.addItem(pile);
	}
	void Rebuild(int num)
	{
		sizePile = num;
		pile = (t_ *)MALLOC(sizeof(t_) * num);
		deleter.allocd.addItem(pile);
	}
	size_t sizePile;
	t_ *pile;
	void *Get()
	{
		sizePile--;
		return pile++;
	}
};
template <class t_>
struct PileList
{
	PileList(int numPtrs, PileList<t_> *_next = 0) : node(numPtrs), next(_next)
	{
	}
	PileNode<t_> node;
	PileList *next;
	void *Get()
	{
		if (node.sizePile)
		{
			return node.Get();
		}
		else
		{
			while (next && !next->node.sizePile)
			{
				PileList<t_> *temp = next;
				next = temp->next;
				delete temp;
			}
			if (next)
				return next->Get();
			else
			{
				node.Rebuild(PILENODE_AUTOINCREMENT);
				return node.Get();
			}
		}
	}
};

PileList<XmlObjectParam> *paramPile = 0;

XmlObjectParam::XmlObjectParam(int xmlhandle, wchar_t *xmlattribute, int xmlattributeid)
		: xmlattributename(xmlattribute), attributeid(xmlattributeid), handle(xmlhandle)
{
KEYWORDUPPER(xmlattribute);
}

#define CBCLASS XmlObjectI
START_DISPATCH;
CB(SETXMLPARAM,      setXmlParam);
CB(GETXMLPARAMVALUE, getXmlParamValue);
CB(GETXMLPARAM,      getXmlParam);
END_DISPATCH;

XmlObjectI::XmlObjectI()
{
	handlepos = 0;
}

XmlObjectI::~XmlObjectI()
{
	params.removeAll();
}

void XmlObjectI::addParam(int xmlhandle, XMLParamPair &param, int unused)
//void XmlObjectI::addXmlParam(int xmlhandle, const wchar_t *xmlattribute, int xmlattributeid)
{
	if (!paramPile)
		paramPile = new PileList<XmlObjectParam>(PILENODE_AUTOINCREMENT);

	params.addItem(new(paramPile->Get()) XmlObjectParam(xmlhandle, param.name, param.id));
}

int XmlObjectI::setXmlParamById(int xmlhandle, int xmlattribute, const wchar_t *param, const wchar_t *value)
{
	return 0;
}

int XmlObjectI::setXmlParam(const wchar_t *param, const wchar_t *value)
{
	int pos = -1;
	int r = 0;
	params.findItem(param, &pos);
	if (pos >= 0)
	{
		XmlObjectParam *xuop = params.enumItem(pos);
		ASSERT(xuop != NULL);
		r = setXmlParamById(xuop->getXmlHandle(), xuop->getXmlAttributeId(), param, value);
		xuop->setLastValue(value);
	}
	else
	{
		onUnknownXmlParam(param, value);
	}
	return r;
}

const wchar_t *XmlObjectI::getXmlParamValue(int n)
{
	return params.enumItem(n)->getLastValue();
}

int XmlObjectI::getXmlParam(const wchar_t *param)
{
	int pos=-1;
	params.findItem(param, &pos);
	return pos;
}

const wchar_t *XmlObjectI::getXmlParamByName(const wchar_t *paramname)
{
	int pos = getXmlParam(paramname);
	if (pos < 0) return NULL;
	return getXmlParamValue(pos);
}

int XmlObjectI::newXmlHandle()
{
	return handlepos++;
}

int XmlObjectI::onUnknownXmlParam(const wchar_t *paramname, const wchar_t *strvalue)
{
	return 0;
}

void XmlObjectI::hintNumberOfParams(int xmlhandle, int numParams)
{
	paramPile = new PileList<XmlObjectParam>(numParams, paramPile);

	params.setMinimumSize(params.getNumItems() + numParams);
}