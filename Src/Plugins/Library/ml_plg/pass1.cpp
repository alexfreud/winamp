#include "main.h"
#include "playlist.h"
#include <atlbase.h>
#include "IDScanner.h"
#include "api__ml_plg.h"

static long PlaylistManagerCount()
{
	ICddbPL2FindDataPtr	pFindData;
	ICddbDisc2Ptr		pDisc;
	long count =0;
	HRESULT	hr = pFindData.CreateInstance(CLSID_CddbPL2FindData);
	if (FAILED(hr))
		return count;

	// empty FindData iterator means ALL FILES
	hr = playlistMgr->FindOpen(pFindData);
	if (FAILED(hr))
		return count;

	while (SUCCEEDED(playlistMgr->FindNext(pFindData, &pDisc)))
	{
		count++;
	}
	playlistMgr->FindClose(pFindData);
	return count;

}
/*
Pass 1 Algorithm

Find all files with gnpl_crit_field_xdev1 == "1"
for each:
GetFileInfo "GracenoteFileID"
if success
{
xdev1 = "2"
GetFileInfo "GracenoteExtData"
if success
xdev1 = "done"
}
else
xdev="3"
*/

void IDScanner::Pass1()
{
	// benski> this function REALLY SUCKS but there's not much we can do about it unfortunately.
	// because Gracenote's Playlist SDK doesn't give us a good way to run queries like this

	ICddbPL2FindDataPtr	pFindData;
	ICddbDisc2Ptr		pDisc;

	HRESULT				hr;
	filesTotal = PlaylistManagerCount(); // super slow, but hey this whole function is slow, anyway

	hr = pFindData.CreateInstance(CLSID_CddbPL2FindData);
	if (FAILED(hr))
		return ;

	// empty FindData iterator means ALL FILES
	hr = playlistMgr->FindOpen(pFindData);
	if (FAILED(hr))
		return;

	while (SUCCEEDED(playlistMgr->FindNext(pFindData, &pDisc)))
	{
		if (killswitch)
			break;
		CComBSTR path,filespec;
		wchar_t file[MAX_PATH] = {0};
		CComBSTR phase;

		pDisc->GetProperty(PROP_Default, PLM_Filename, &filespec);
		pDisc->GetProperty(PROP_Default, PLM_Pathname, &path);
		PathCombineW(file, path, filespec);
		playlistMgr->FileGetFieldVal(file, gnpl_crit_field_xdev1, &phase);
		if (phase && phase[0] && phase[0]=='1')
		{
			wchar_t gracenoteFileId[256]=L"";
			if (GetFileInfo(file, L"GracenoteFileID", gracenoteFileId, 256) && gracenoteFileId[0])
			{
				wchar_t gracenoteExtData[65536]=L"";
				GetFileInfo(file, L"GracenoteExtData", gracenoteExtData, 65536);

				// write back to Media Library database (since if we got here, it wasn't in the itemRecordList)
				AGAVE_API_MLDB->SetField(file, "GracenoteFileID", gracenoteFileId);
				if (gracenoteExtData[0]) AGAVE_API_MLDB->SetField(file, "GracenoteExtData", gracenoteExtData);

				SetGracenoteData(file, gracenoteFileId, gracenoteExtData);
				playlistMgr->FileSetFieldVal(file, gnpl_crit_field_xdev1, L"0"); // mark as done!
			}
			else // no Tag ID, so we'll have to use AlbumID
			{
				playlistMgr->FileSetFieldVal(file, gnpl_crit_field_xdev1, L"2"); // move to phase 2
			}
		}
		filesComplete++;
	}
	playlistMgr->FindClose(pFindData);
}