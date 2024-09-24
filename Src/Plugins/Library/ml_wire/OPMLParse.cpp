#if 0
#include "main.h"
#include "OPMLParse.h"

#ifdef DEBUG
#include <iostream>
static void DisplayNodes(XMLNode &node)
{
	XMLNode::NodeMap::iterator nodeItr;
	for (nodeItr = node.nodes.begin();nodeItr != node.nodes.end(); nodeItr++)
	{

		for (XMLNode::NodeList::iterator itr = nodeItr->second.begin(); itr != nodeItr->second.end(); itr++)
		{
			std::wcerr << L"<" << nodeItr->first << L">" << std::endl;
			DisplayNodes(**itr);
			std::wcerr << L"</" << nodeItr->first << L">" << std::endl;
		}

	}
}
#endif

void OPMLParse::ReadNodes(const wchar_t *url)
{
//	DisplayNodes(xmlNode);
	Alias<XMLNode> curNode;
	curNode = xmlNode.Get(L"opml");
	if (!curNode)
		return;

}
#endif