#include "playlist.h"
#include "main.h"
#include "api__ml_plg.h"
#include "../../General/gen_ml/ml.h"
#include <atlbase.h>
#include "IDScanner.h"

IConnectionPoint *GetConnectionPoint(IUnknown *punk, REFIID riid)
{
	if (!punk)
		return 0;

	IConnectionPointContainer *pcpc;
	IConnectionPoint *pcp = 0;

	HRESULT hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **) & pcpc);
	if (SUCCEEDED(hr))
	{
		pcpc->FindConnectionPoint(riid, &pcp);
		pcpc->Release();
	}
	return pcp;
}

bool IDScanner::SetupMusicID()
{
	if (!SetupPlaylistSDK())
		return false;

	if (musicID)
		return true;

	musicID = AGAVE_API_GRACENOTE->GetMusicID();
	if (musicID)
	{
		IConnectionPoint *icp = GetConnectionPoint(musicID, DIID__ICDDBMusicIDManagerEvents);
		if (icp)
		{
			icp->Advise(static_cast<IDispatch *>(this), &m_dwCookie);
			icp->Release();
		}
		return true;
	}
	return false;
}
