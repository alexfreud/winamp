#ifndef NULLSOFT_ATOMPARSEH
#define NULLSOFT_ATOMPARSEH
#if 0
#include "RFCDate.h"
#include "XMLNode.h"
#include "Feeds.h"
#include "../nu/AutoChar.h"
#include "ChannelSync.h"

void ReadAtomItem(XMLNode *item, Channel &channel)
{
}

void ReadAtomChannel(XMLNode *node, Channel &newChannel)
{
	XMLNode *curNode = 0;
	curNode = node->Get(L"title");
	if (curNode)
		newChanneltitle =  curNode->content;

	curNode = node->Get(L"subtitle");
	if (curNode)
		newChannel.description = curNode->content;

	XMLNode::NodeList &links = node->GetList(L"link");
	XMLNode::NodeList::iterator linkItr;
	for (linkItr=links.begin();linkItr!=links.end();linkItr++)
	{
		if ((*linkItr)->properties[L"rel"].empty()
			|| (*linkItr)->properties[L"rel"]== L"alternate")
			newChannel.link = (*linkItr)->properties[L"href"];
	}

	XMLNode::NodeList &entries = node->GetList(L"entry");
	XMLNode::NodeList::iterator entryItr;
	for (entryItr=entries.begin();entryItr!=entries.end();entryItr++)
	{
	}
}

void ReadAtom(XMLNode *atom, ChannelSync *sync)
{
	sync->BeginChannelSync();
	Channel newChannel;
	ReadAtomChannel(atom, newChannel);
		sync->NewChannel(newChannel);

	sync->EndChannelSync();
}
#endif
#endif