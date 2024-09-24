#include "../tagz/ifc_tagprovider.h"
#include "api.h"
#include <api/service/waservicefactorybase.h>

class TagProvider : public ifc_tagprovider
{
public:
	wchar_t *GetTag(const wchar_t *name, ifc_tagparams *parameters);
	void FreeTag(wchar_t *Tag);
protected:
	RECVS_DISPATCH;
};
extern TagProvider *tagProvider;

// {9490752F-23BF-4923-86F1-E1186543EC64}
static const GUID WinampTagProviderGUID = 
{ 0x9490752f, 0x23bf, 0x4923, { 0x86, 0xf1, 0xe1, 0x18, 0x65, 0x43, 0xec, 0x64 } };


/*
class TagProviderFactory : public waServiceBase<ifc_tagprovider, TagProviderFactory> {
public:
  TagProviderFactory() : waServiceBase<ifc_tagprovider, TagProviderFactory>(WinampTagProviderGUID) {}
  static const char *getServiceName() { return "Winamp Tag Provider"; }
  virtual api_tagprovider *getService() { return tagProvider; }
	static FOURCC getServiceType() { return api_tagprovider::getServiceType(); }
};
*/
