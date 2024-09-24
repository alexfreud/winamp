#include "Main.h"
#include "FeedParse.h"


#include "RFCDate.h"

#include "RSSParse.h"
#include "AtomParse.h"
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

void FeedParse::ReadNodes(const wchar_t *url)
{
	const XMLNode *curNode = xmlDOM.GetRoot();

	curNode = curNode->Get(L"rss");
	if (curNode)
	{
		ReadRSS(curNode, sync, loadingOwnFeed, url);
		return ;
	}
}