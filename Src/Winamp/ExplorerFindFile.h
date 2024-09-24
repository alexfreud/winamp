#ifndef NULLSOFT_WINAMP_EXPLORERFINDFILE_H
#define NULLSOFT_WINAMP_EXPLORERFINDFILE_H

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"
#include <map>
#include <vector>

class ExplorerFindFile : public api_explorerfindfile
{
public:
	ExplorerFindFile();
	~ExplorerFindFile();
	static const char *getServiceName() { return "ExplorerFindFile API"; }
	static const GUID getServiceGuid() { return ExplorerFindFileApiGUID; }
	BOOL AddFile(wchar_t* file);
	BOOL ShowFiles();
	void Reset();
protected:
	RECVS_DISPATCH;

	typedef std::map<LPITEMIDLIST, std::vector<LPCITEMIDLIST>> PIDLListMap;
	PIDLListMap pidlList;
};

extern ExplorerFindFile *explorerFindFileManager;

#endif