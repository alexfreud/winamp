#include "IDScanner.h"
#include "playlist.h"
#include <atlbase.h>
#include "main.h"
#include "api__ml_plg.h"

static ICddbFileInfoList *CreateScanList(volatile int *killswitch)
{
	// benski> this function REALLY SUCKS but there's not much we can do about it unfortunately.
	// because Gracenote's Playlist SDK doesn't give us a good way to run queries like this
	ICddbPL2FindDataPtr	pFindData;
	ICddbDisc2Ptr		pDisc;

	if (FAILED(pFindData.CreateInstance(CLSID_CddbPL2FindData)))
		return 0;

	if (FAILED(playlistMgr->FindOpen(pFindData)))
		return 0;

	ICddbFileInfoListPtr infoList;
	infoList.CreateInstance(CLSID_CddbFileInfoList);

	while (SUCCEEDED(playlistMgr->FindNext(pFindData, &pDisc)))
	{
		if (*killswitch)
			return 0;
		CComBSTR path,filespec;
		wchar_t file[MAX_PATH] = {0};
		CComBSTR phase;

		pDisc->GetProperty(PROP_Default, PLM_Filename, &filespec);
		pDisc->GetProperty(PROP_Default, PLM_Pathname, &path);
		PathCombineW(file, path, filespec);
		playlistMgr->FileGetFieldVal(file, gnpl_crit_field_xdev1, &phase);
		if (!phase || !phase[0] || phase[0]!='2')
			continue;

		ICddbFileInfoPtr info;
		info.CreateInstance(CLSID_CddbFileInfo);
		info->put_Filename(file);
		infoList->AddFileInfo(info);
	}

	playlistMgr->FindClose(pFindData);
	infoList->AddRef();
	return infoList;
}


/*
Pass 2 Algorithm
Find all files with gnpl_crit_field_xdev1 == "2" and run them through MusicID
*/
void IDScanner::Pass2()
{
	if (SetupMusicID())
	{
		ICddbFileInfoList *infoList = CreateScanList(&killswitch);
		if (infoList)
		{
			musicID->LibraryID(infoList, MUSICID_RETURN_EXACT_ONLY | MUSICID_GET_FP_FROM_APP | MUSICID_GET_TAG_FROM_APP | MUSICID_PREFER_WF_MATCHES);
			infoList->Release();
		}
	}
}

int IDScanner::Pass2OnThread(HANDLE handle, void *user_data, intptr_t id)
{
	IDScanner *scanner = (IDScanner *)id;
	scanner->Pass2();
	return 0;
}
