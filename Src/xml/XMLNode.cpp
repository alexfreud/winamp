#include "XMLNode.h"

static int CompareStuff(const wchar_t *const &str1, const wchar_t *const &str2)
{
	return CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 0, str1, -1, str2, -1)-2;
}

XMLNode::XMLNode() : content(0), content_len(0), parent(0)
{
}

XMLNode::~XMLNode()
{
	for (PropMap::iterator mapItr=properties.begin();mapItr != properties.end();mapItr++)
	{
		free((wchar_t *)mapItr->first);
		free(mapItr->second);
	}

	for (NodeMap::iterator mapItr=nodes.begin();mapItr != nodes.end();mapItr++)
	{
		NodeList * const nodeList = mapItr->second;
		if (nodeList)
		{
			for (NodeList::iterator itr=nodeList->begin(); itr!= nodeList->end(); itr++)
			{
				delete static_cast<XMLNode *>(*itr);
			}
		}
		free((wchar_t *)mapItr->first);
	}

	if (content)
	{
		free(content);
		content = 0;
		content_len = 0;
	}
}

const XMLNode *XMLNode::Get(const wchar_t *tagName) const
{
	NodeMap::const_iterator itr = nodes.find(tagName);
	if (itr == nodes.end())
		return 0;
	else
	{
		NodeList *list = itr->second;
		return list->at(0);
	}
}

const XMLNode::NodeList *XMLNode::GetList(const wchar_t *tagName) const
{
	NodeMap::const_iterator itr = nodes.find(tagName);
	if (itr == nodes.end())
	{
		return 0;
	}
	else
	{
		NodeList *list = itr->second;
		return list;
	}
}

const bool XMLNode::Present(const wchar_t *tagName) const 
{
	return nodes.find(tagName) != nodes.end();
}

// LEGACY implementaions!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//void XMLNode::SetProperty(const wchar_t *prop, const wchar_t *value)
//{
//	PropMap::MapPair &pair = properties.getItem(prop);
//	if (pair.first == prop) // replace with a copy if we made a new entry
//		pair.first = _wcsdup(prop);
//	free(pair.second);
//	pair.second = _wcsdup(value);
//}
void XMLNode::SetProperty(const wchar_t* prop, const wchar_t* value)
{
	auto it = properties.find(prop);
	if (properties.end() == it)
	{
		properties.insert({ _wcsdup(prop), _wcsdup(value) });
	}
	else
	{
		if (nullptr != it->second)
		{
			free(it->second);
		}
		properties[prop] = _wcsdup(value);
	}
}

void XMLNode::SetContent_Own(wchar_t *new_content)
{
	if (content)
	{
		free(content);
		content = 0;
		content_len = 0;
	}

	if (new_content && *new_content)
	{
		content = new_content;
		content_len = wcslen(content);
	}
}

void XMLNode::AppendContent(wchar_t *append)
{
	if (append && *append)
	{
		if (content)
		{
			size_t len = wcslen(append), new_len = len + content_len;
			wchar_t *new_content = (wchar_t *)realloc(content, (new_len+1)*sizeof(wchar_t));
			if (new_content)
			{
				content = new_content;
				wcsncpy(content+content_len, append, len);
				*(content+content_len+len) = 0;
				content_len = new_len;
			}
			else
			{
				new_content = (wchar_t *)malloc((new_len+1)*sizeof(wchar_t));
				if (new_content)
				{
					memcpy(new_content, content, content_len*sizeof(wchar_t));
					free(content);
					content = new_content;

					wcsncpy(content+content_len, append, len);
					*(content+content_len+len) = 0;
					content_len = new_len;
				}
			}
		}
		else
		{
			content_len = wcslen(append);
			content = (wchar_t *)malloc((content_len+1)*sizeof(wchar_t));
			if (content)
				memcpy(content, append, (content_len+1)*sizeof(wchar_t));
		}
	}
}

// LEGACY implementaions!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//void XMLNode::AddNode(const wchar_t *name, XMLNode *new_node)
//{
//	// first, add entry in nodes map
//	NodeMap::MapPair &pair = nodes.getItem(name);
//	if (pair.first == name) // replace with a copy if we made a new entry
//		pair.first = _wcsdup(name);
//
//	// make the node list if we need it
//	if (!pair.second)
//		pair.second = new NodeList;
//
//	pair.second->push_back(new_node);
//}
void XMLNode::AddNode(const wchar_t* name, XMLNode* new_node)
{
	auto it = nodes.find(name);
	if (nodes.end() == it)
	{
		nodes.insert({ _wcsdup(name), new NodeList() });
	}
	else
	{
		if (nullptr == it->second)
		{
			nodes[name] = new NodeList();
		}
	}

	nodes[name]->push_back(new_node);
}

const wchar_t *XMLNode::GetContent() const
{
	return content;
}

const wchar_t *XMLNode::GetProperty(const wchar_t *prop) const
{
	for (PropMap::const_iterator mapItr = properties.begin(); mapItr != properties.end(); mapItr++)
	{
		if (CompareStuff(mapItr->first, prop) == 0)
		{
			return mapItr->second;
		}
	}
	return 0;
}