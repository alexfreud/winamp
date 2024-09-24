#include "main.h"
#include "FeedBase.h"
#include <assert.h>

void FeedBase::dependent_regViewer(api_dependentviewer *viewer, int add)
{
	if (viewer)
	{
		if (add)
		{
			//if (!viewers.contains(viewer))
			if(viewers.end() == std::find(viewers.begin(), viewers.end(), viewer))
			{
				viewers.push_back(viewer);
			}
		}
		else
		{
			//viewers.eraseObject(viewer);
			auto it = std::find(viewers.begin(), viewers.end(), viewer);
			if (it != viewers.end())
			{
				viewers.erase(it);
			}
		}
	}
}

void *FeedBase::dependent_getInterface(const GUID *classguid)
{
	HANDLEGETINTERFACE(svc_textFeed);
	return NULL;
}

api_dependent *FeedBase::getDependencyPtr()
{
	return static_cast<api_dependent *>(this);
}

void FeedBase::CallViewers( const wchar_t *feedid, const wchar_t *text, size_t length )
{
	for ( api_dependentviewer *l_viewer : viewers )
		l_viewer->dependentViewer_callback( static_cast<api_dependent *>( this ), svc_textFeed::depend_getClassGuid(), DependentCB::DEPCB_EVENT, Event_TEXTCHANGE, (intptr_t)feedid, (void *)text, length );
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS FeedBase
START_MULTIPATCH;
START_PATCH(DependentPatch)
      M_VCB(DependentPatch, api_dependent, API_DEPENDENT_REGVIEWER,    dependent_regViewer);
       M_CB(DependentPatch, api_dependent, API_DEPENDENT_GETINTERFACE, dependent_getInterface);
 NEXT_PATCH(TextFeedPatch)
       M_CB(TextFeedPatch,  svc_textFeed,  SVCTEXTFEED_HASFEED,     hasFeed);
       M_CB(TextFeedPatch,  svc_textFeed,  SVCTEXTFEED_GETFEEDTEXT, getFeedText);
       M_CB(TextFeedPatch,  svc_textFeed,  SVCTEXTFEED_GETFEEDDESC, getFeedDescription);
       M_CB(TextFeedPatch,  svc_textFeed,  SVCTEXTFEED_GETDEPENDENCYPTR, getDependencyPtr);
END_PATCH
END_MULTIPATCH;
#undef CBCLASS