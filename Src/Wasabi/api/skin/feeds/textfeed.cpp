#include <precomp.h>
#include "textfeed.h"
#include <bfc/pair.h>

int TextFeed::registerFeed(const wchar_t *feedid, const wchar_t *initial_text, const wchar_t *description)
{
	//if (feeds.getItem(StringW(feedid))) 
	//	return FALSE;
	auto it = feeds.find(feedid);
	if (feeds.end() != it)
	{
		return FALSE;
	}

	//std::pair<std::wstring, std::wstring> pair(initial_text, description);
	
	feeds.insert({ feedid, {initial_text, description} });

	dependent_sendEvent(svc_textFeed::depend_getClassGuid(), Event_TEXTCHANGE, (intptr_t)feedid, (void*)initial_text, wcslen(initial_text) + 1);
	return TRUE;
}

int TextFeed::sendFeed(const wchar_t *feedid, const wchar_t *text)
{
	//Pair <StringW, StringW> ft(L"", L"");
	//if (!feeds.getItem(StringW(feedid), &ft))
	//{
	//	//CUT    ASSERTALWAYS("hey, you're trying to send a feed you didn't register. stop it.");
	//	DebugString("TextFeed::sendFeed(), feedid '%s' not registered", feedid);
	//	return FALSE;
	//}
	auto it = feeds.find(feedid);
	if (feeds.end() == it)
	{
		return FALSE;
	}

	//StringW id(feedid);
	//feeds.getItem(id, &ft);
	//ft.a = StringW(text);
	//feeds.setItem(StringW(feedid), ft);

	auto &ft = feeds[feedid];
	ft.first = text;

	dependent_sendEvent(svc_textFeed::depend_getClassGuid(), Event_TEXTCHANGE, (intptr_t)feedid, (void*)text, wcslen(text) + 1);
	return TRUE;
}

const wchar_t *TextFeed::getFeedText(const wchar_t *name)
{
	//const Pair<StringW, StringW> *ft = feeds.getItemRef(StringW(name));
	//if (ft == NULL) 
	//	return NULL;
	//ft->a.getValue();

	auto it = feeds.find(name);
	if (it == feeds.end())
	{
		return NULL;
	}
	auto& ft = it->second;
	return ft.first.c_str();
}

const wchar_t *TextFeed::getFeedDescription(const wchar_t *name)
{
	//const Pair<StringW, StringW> *ft = feeds.getItemRef(StringW(name));
	//if (ft == NULL) return NULL;
	//return ft->b.getValue();

	auto it = feeds.find(name);
	if (it == feeds.end())
	{
		return NULL;
	}

	auto& ft = it->second;
	return ft.second.c_str();
}

int TextFeed::hasFeed(const wchar_t *name)
{
	return feeds.count(name);
}

void TextFeed::dependent_onRegViewer(api_dependentviewer *viewer, int add)
{
	if (add)
	{
		//for (int i = 0; i < feeds.getNumItems(); i++)
		//{
		//	StringW a = feeds.enumIndexByPos(i, StringW(L""));
		//	Pair<StringW, StringW> sp(L"", L"");
		//	StringW b = feeds.enumItemByPos(i, sp).a;
		//	dependent_sendEvent(svc_textFeed::depend_getClassGuid(), Event_TEXTCHANGE, (intptr_t)a.getValue(), (void*)b.getValue(), b.len() + 1, viewer); //send to this viewer only
		//}

		for (auto it = feeds.begin(); it != feeds.end(); it++)
		{
			std::wstring key = it->first;
			auto val = it->second;
			std::wstring val_first = val.first;
			dependent_sendEvent(svc_textFeed::depend_getClassGuid(), Event_TEXTCHANGE, (intptr_t)key.c_str(), (void*)val_first.c_str(), wcslen(val_first.c_str()) + 1, viewer); //send to this viewer only
		}
	}

	if (add) onRegClient();
	else onDeregClient();
}

void *TextFeed::dependent_getInterface(const GUID *classguid)
{
	HANDLEGETINTERFACE(svc_textFeed);
	return NULL;
}
