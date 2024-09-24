#include <precomp.h>

#include "cfgitemi.h"
#include <api/config/items/attrcb.h>
#include <api/config/items/attribs.h>

#include <bfc/wasabi_std.h>
#include <bfc/memblock.h>

CfgItemI::CfgItemI(const wchar_t *name, GUID guid) 
:NamedW(name), myguid(guid), parent_guid(INVALID_GUID) { }

CfgItemI::~CfgItemI()
{
	deregisterAll();
}

const wchar_t *CfgItemI::cfgitem_getName()
{
	return NamedW::getName();
}

GUID CfgItemI::cfgitem_getGuid()
{
	return myguid;
}

void CfgItemI::cfgitem_setPrefix(const wchar_t *_prefix)
{
	prefix = _prefix;
}

const wchar_t *CfgItemI::cfgitem_getPrefix()
{
	return prefix.c_str();
}

int CfgItemI::cfgitem_getNumAttributes()
{
	return attributes.getNumItems();
}

const wchar_t *CfgItemI::cfgitem_enumAttribute(int n)
{
	Attribute *attr = attributes[n];
	if (attr) return attr->getAttributeName();
	return NULL;
}

const wchar_t *CfgItemI::cfgitem_getConfigXML()
{
	return cfgxml.c_str();
}

int CfgItemI::cfgitem_getNumChildren()
{
	return children.getNumItems();
}

CfgItem *CfgItemI::cfgitem_enumChild(int n)
{
	return children[n];
}

GUID CfgItemI::cfgitem_getParentGuid()
{
	return parent_guid;
}

void CfgItemI::cfgitem_onRegister()
{
	foreach(children)
	WASABI_API_CONFIG->config_registerCfgItem(children.getfor());
	endfor
}
void CfgItemI::cfgitem_onDeregister()
{
	foreach(children)
	WASABI_API_CONFIG->config_deregisterCfgItem(children.getfor());
	endfor
}

Attribute *CfgItemI::getAttributeByName(const wchar_t *name)
{
	Attribute *attr;
	foreach(attributes)
	attr = attributes.getfor();
	if (!WCSICMP(name, attr->getAttributeName())) return attr;
	endfor
	return NULL;
}

int CfgItemI::cfgitem_getAttributeType(const wchar_t *name)
{
	Attribute *attr = getAttributeByName(name);
	if (attr == NULL) return AttributeType::NONE;
	return attr->getAttributeType();
}

const wchar_t *CfgItemI::cfgitem_getAttributeConfigGroup(const wchar_t *name)
{
	Attribute *attr = getAttributeByName(name);
	if (attr == NULL) return NULL;
	return attr->getConfigGroup();
}

int CfgItemI::cfgitem_getDataLen(const wchar_t *name)
{
	Attribute *attr = getAttributeByName(name);
	if (attr == NULL) return -1;
	return attr->getDataLen();
}

int CfgItemI::cfgitem_getData(const wchar_t *name, wchar_t *data, int data_len)
{
	Attribute *attr = getAttributeByName(name);
	if (attr == NULL) return -1;
	return attr->getData(data, data_len);
}

int CfgItemI::cfgitem_setData(const wchar_t *name, const wchar_t *data)
{
	Attribute *attr = getAttributeByName(name);
	if (attr == NULL) return -1;
	int ret = attr->setDataNoCB(data);
	if (ret) cfgitem_onAttribSetValue(attr);
	return ret;
}

int CfgItemI::cfgitem_onAttribSetValue(Attribute *attr)
{
	// notify dependency watchers that an attribute changed
	dependent_sendEvent(CfgItem::depend_getClassGuid(), Event_ATTRIBUTE_CHANGED, 0, (void*)attr->getAttributeName());

	//for (int i = 0; ; i++)
	//{
	//	AttrCallback *acb;
	//	if (!callbacks.multiGetItem(attr, i, &acb)) 
	//		break;
	//	
	//	acb->onValueChange(attr);
	//}
	auto elements = callbacks.equal_range(attr);
	for (auto& it = elements.first; it != elements.second; ++it)
	{
		AttrCallback* acb = it->second;
		if (acb)
		{
			acb->onValueChange(attr);
		}
	}

	return 0;
}

void CfgItemI::cfgitem_setGUID(GUID guid)
{
	myguid = guid;
}

int CfgItemI::setName(const wchar_t *name)
{
	NamedW::setName(name);
	// notify dependency watchers that name changed?
	dependent_sendEvent(CfgItem::depend_getClassGuid(), Event_NAMECHANGE);
	return 1;
}

int CfgItemI::registerAttribute(Attribute *attr, AttrCallback *acb)
{
	if (attributes.haveItem(attr)) return 0;
	int ret = attributes.addItem(attr) != NULL;
	if (!ret) return ret;

	attr->setCfgItem(this);

	// set optional callback
	if (acb != NULL)
	{
		addCallback(attr, acb);
	}

	// notify dependency watchers of new attribute
	dependent_sendEvent(CfgItem::depend_getClassGuid(), Event_ATTRIBUTE_ADDED, 0, (void*)attr->getAttributeName());

	return ret;
}

int CfgItemI::deregisterAttribute(Attribute *attr)
{
	if (!attributes.haveItem(attr)) return 0;
	int ret = attributes.removeItem(attr);
	// notify dependency watchers of attribute going away
	dependent_sendEvent(CfgItem::depend_getClassGuid(), Event_ATTRIBUTE_REMOVED, 0, (void*)attr->getAttributeName());

	// remove callbacks
	//callbacks.multiDelAllForItem(attr, TRUE);
	auto elements = callbacks.equal_range(attr);
	for (auto& it = elements.first; it != elements.second; ++it)
	{
		AttrCallback* acb = it->second;
		if (acb)
		{
			delete acb;
		}
	}
	callbacks.erase(attr);


	attr->disconnect();

	return ret;
}

void CfgItemI::addCallback(Attribute *attr, AttrCallback *acb)
{
	ASSERT(attr != NULL);
	ASSERT(acb != NULL);
	//callbacks.multiAddItem(attr, acb);
	callbacks.insert({ attr, acb });
}

void CfgItemI::deregisterAll()
{
	foreach(children)
	children.getfor()->deregisterAll();
	endfor
	while (attributes.getNumItems()) deregisterAttribute(attributes[0]);
}

void CfgItemI::addChildItem(CfgItemI *child)
{
	ASSERT(child != NULL);
	if (!children.haveItem(child))
	{
		children.addItem(child);
		child->setParentGuid(myguid);
	}
}

void CfgItemI::setCfgXml(const wchar_t *groupname)
{
	cfgxml = groupname;
}

void CfgItemI::setParentGuid(GUID guid)
{
	parent_guid = guid;
}

void *CfgItemI::dependent_getInterface(const GUID *classguid)
{
	HANDLEGETINTERFACE(CfgItem);
	return NULL;
}

int CfgItemI::cfgitem_addAttribute(const wchar_t *name, const wchar_t *defval)
{
	if (getAttributeByName(name)) return 0;
	registerAttribute(newattribs.addItem(new _string(name, defval)));
	return 1;
}

int CfgItemI::cfgitem_delAttribute(const wchar_t *name)
{
	Attribute *attr = getAttributeByName(name);
	if (!newattribs.haveItem(attr)) return 0;
	deregisterAttribute(attr);
	delete attr;
	newattribs.removeItem(attr);
	return 1;
}


