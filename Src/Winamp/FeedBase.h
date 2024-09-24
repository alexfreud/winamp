#ifndef NULLSOFT_WINAMP_FEEDBASE_H
#define NULLSOFT_WINAMP_FEEDBASE_H

#include <api/dependency/api_dependent.h>
#include <api/skin/feeds/api_textfeed.h>
#include <vector>
#include <bfc/multipatch.h>

enum {DependentPatch = 10,	TextFeedPatch = 20 };

class FeedBase 
	: public MultiPatch<DependentPatch, api_dependent>,
	public MultiPatch<TextFeedPatch, svc_textFeed>
{
private:
	void dependent_regViewer(api_dependentviewer *viewer, int add);
	void *dependent_getInterface(const GUID *classguid);

	virtual int hasFeed(const wchar_t *name)=0;
	virtual const wchar_t *getFeedText(const wchar_t *name)=0;
	virtual const wchar_t *getFeedDescription(const wchar_t *name)=0;
	api_dependent *getDependencyPtr();
protected:
	void CallViewers(const wchar_t *feedid, const wchar_t *text, size_t length);
	
protected:
	std::vector<api_dependentviewer*> viewers;

protected:
	RECVS_MULTIPATCH;
};

#endif