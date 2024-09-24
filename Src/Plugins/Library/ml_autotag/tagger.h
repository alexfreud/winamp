#ifndef _NULLSOFT_AUTOTAGGER_TAGGER_H_
#define _NULLSOFT_AUTOTAGGER_TAGGER_H_
#include <vector>

class TagItem
{
public:
	TagItem(const char * filename);
	TagItem(const wchar_t * filename, bool copy=true);
	~TagItem();
	void init();
	void SetStatus(CddbMusicIDStatus status, HWND hwndDlg);
	void SetStatus(const wchar_t *str, HWND hwndDlg);
	void TagUpdate(HWND hwndDlg);
	void Check(HWND, BOOL);

	const wchar_t *filename;
	bool freefn;
	int num;

	CddbMusicIDMatchCode last_match;

	ICddbID3TagPtr oldTag;
	ICddbFileTagPtr newTag;
	ICddbDisc2Ptr disc;
};

class Tagger : public _ICDDBMusicIDManagerEvents
{
public:
	Tagger(std::vector<TagItem*> &list, ICDDBMusicIDManager3 *musicid); // 0 = trackid, 1 = albumid, 2 = libraryid
	~Tagger();

	TagItem* FindTagItem(const wchar_t * filename);

protected:
	HRESULT OnTrackIDStatusUpdate(CddbMusicIDStatus Status, BSTR filename, long* Abort);
	HRESULT OnAlbumIDStatusUpdate(CddbMusicIDStatus Status, BSTR filename, long current_file, long total_files, long* Abort);
	HRESULT OnTrackIDComplete(CddbMusicIDMatchCode match_code, ICddbFileInfo* pInfoIn, ICddbFileInfoList* pListOut);
	HRESULT OnAlbumIDComplete(LONG match_code, ICddbFileInfoList* pListIn, ICddbFileInfoLists* pListsOut);
	HRESULT OnLibraryIDListStarted(ICddbFileInfoList* pList, long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError, long *Abort);
	HRESULT OnLibraryIDListComplete(ICddbFileInfoList* pList, long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError, long *Abort);
	HRESULT OnLibraryIDComplete(long FilesComplete, long FilesTotal, long FilesExact, long FilesFuzzy, long FilesNoMatch, long FilesError);
	
	HRESULT FillTag(ICddbFileInfo *info, BSTR filename);

	// com shit
	STDMETHODIMP STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr);
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR * pctinfo);

public:
	HWND hwndDlg;
	std::vector<TagItem*> list;
	ICDDBMusicIDManager3 *musicid;
	long abort;
	W_ListView listview;
	IConnectionPoint *icp;
	DWORD m_dwCookie;
};

extern INT_PTR CALLBACK autotagger_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // _NULLSOFT_AUTOTAGGER_TAGGER_H_