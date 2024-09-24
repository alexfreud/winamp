#include "main.h"
#include "LocalMediaCOM.h"


void sortResults(int column, int dir, itemRecordListW *obj);

static void saveQueryToList(nde_scanner_t s, itemRecordListW *obj, int sortColumn, int sortDir)
{
	emptyRecordList(obj);

	EnterCriticalSection(&g_db_cs);
	NDE_Scanner_First(s);

	int r;
	do
	{
		nde_field_t f = NDE_Scanner_GetFieldByID(s, MAINTABLE_ID_FILENAME);
		if (f)
		{
			allocRecordList(obj, obj->Size + 1);
			if (!obj->Alloc) break;

			obj->Items[obj->Size].filename = NDE_StringField_GetString(f);
			ndestring_retain(obj->Items[obj->Size].filename);
			ScannerRefToObjCacheNFNW(s, obj, true);
		}

		r = NDE_Scanner_Next(s);
	}
	while (r && !NDE_Scanner_EOF(s));

	if (((Table *)g_table)->HasErrors()) // TODO: don't use C++ NDE API
	{
		wchar_t *last_query = NULL;
		if (m_media_scanner)
		{
			const wchar_t *lq = NDE_Scanner_GetLastQuery(m_media_scanner);
			if (lq) last_query = _wcsdup(lq);
			NDE_Table_DestroyScanner(g_table, m_media_scanner);
		}
		NDE_Table_Compact(g_table);
		if (m_media_scanner)
		{
			m_media_scanner = NDE_Table_CreateScanner(g_table);
			if (last_query != NULL)
			{
				NDE_Scanner_Query(m_media_scanner, last_query);
				free(last_query);
			}
		}
	}
	LeaveCriticalSection(&g_db_cs);

	compactRecordList(obj);

	sortResults(sortColumn, sortDir, obj);
}

void WriteEscaped(FILE *fp, const wchar_t *str)
{
	// TODO: for speed optimization,
	// we should wait until we hit a special character 
	// and write out everything else so before it,
	// like how ASX loader does it
	while (str && *str)
	{
		switch(*str)
		{
		case L'&':
			fputws(L"&amp;", fp);
			break;
		case L'>':
			fputws(L"&gt;", fp);
			break;
		case L'<':
			fputws(L"&lt;", fp);
			break;
		case L'\'':
			fputws(L"&apos;", fp);
			break;
		case L'\"':
			fputws(L"&quot;", fp);
			break;
		default:
			fputwc(*str, fp);
			break;
		}
		// write out the whole character
		wchar_t *next = CharNextW(str);
		while (++str != next)
			fputwc(*str, fp);

	}
}

bool SaveListToXML(const itemRecordListW *const obj, const wchar_t *filename, int limit)
{
	int i=0;
	FILE *fp = _wfopen(filename, L"wb");
	if (!fp)
		return false;
	wchar_t BOM = 0xFEFF;
	fwrite(&BOM, sizeof(BOM), 1, fp);
	fwprintf(fp, L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>");
	fputws(L"<itemlist>\r\n", fp);

	while (i < obj->Size)
	{
		fputws(L"<item ", fp);

		if (obj->Items[i].filename)
		{
			fputws(L"filename=\"", fp);
			WriteEscaped(fp, obj->Items[i].filename);
			fputws(L"\" ", fp);
		}

		if (obj->Items[i].title)
		{
			fputws(L"title=\"", fp);
			WriteEscaped(fp, obj->Items[i].title);
			fputws(L"\" ", fp);
		}

		if (obj->Items[i].album)
		{
			fputws(L"album=\"", fp);
			WriteEscaped(fp, obj->Items[i].album);
			fputws(L"\" ", fp);
		}

		if (obj->Items[i].artist)
		{
			fputws(L"artist=\"", fp);
			WriteEscaped(fp, obj->Items[i].artist);
			fputws(L"\" ", fp);
		}

		if (obj->Items[i].comment)
		{
			fputws(L"comment=\"", fp);
			WriteEscaped(fp, obj->Items[i].comment);
			fputws(L"\" ", fp);
		}

		if (obj->Items[i].genre)
		{
			fputws(L"genre=\"", fp);
			WriteEscaped(fp, obj->Items[i].genre);
			fputws(L"\" ", fp);
		}

		if (obj->Items[i].year > 0)
			fwprintf(fp, L"year=\"%d\" ",obj->Items[i].year);

		if (obj->Items[i].track > 0)
			fwprintf(fp, L"track=\"%d\" ",obj->Items[i].track);

		if (obj->Items[i].length > 0)
		fwprintf(fp, L"length=\"%d\" ",obj->Items[i].length);
		
		// TODO: extended info fields
		fputws(L"/>", fp);
		if (++i == limit)
			break;
	}
	fputws(L"</itemlist>", fp);
	fclose(fp);

	return true;
}

enum
{
    DISP_LOCALMEDIA_XMLQUERY = 777,

};

#define CHECK_ID(str, id) 		if (_wcsicmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT LocalMediaCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("XMLQuery", DISP_LOCALMEDIA_XMLQUERY)
		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT LocalMediaCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT LocalMediaCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

void Bookmark_WriteAsXML(const wchar_t *filename, int max);

HRESULT LocalMediaCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISP_LOCALMEDIA_XMLQUERY:
		if (pdispparams->cArgs == 4)
		{
      openDb();
      int max,dir,column;

      // Strict-ish type checking
      if ( pdispparams->rgvarg[0].vt == VT_BSTR )
        max = _wtoi(pdispparams->rgvarg[0].bstrVal);
      else
        max = pdispparams->rgvarg[0].uiVal;
      
      if ( pdispparams->rgvarg[1].vt == VT_BSTR )
        dir = _wtoi(pdispparams->rgvarg[2].bstrVal);
      else
        dir = pdispparams->rgvarg[1].uiVal;

      if ( pdispparams->rgvarg[2].vt == VT_BSTR )
        column = _wtoi(pdispparams->rgvarg[2].bstrVal);
      else
        column = pdispparams->rgvarg[2].uiVal;

 // run query
			EnterCriticalSection(&g_db_cs);
			nde_scanner_t s=NDE_Table_CreateScanner(g_table);
			NDE_Scanner_Query(s, pdispparams->rgvarg[3].bstrVal);

// create itemRecordList (necessary because NDE doesn't sort)
			itemRecordListW obj;
      obj.Alloc = 0;
      obj.Items = NULL;
      obj.Size = 0;
			saveQueryToList(s, &obj, column, dir); 
			NDE_Table_DestroyScanner(g_table, s);
			LeaveCriticalSection(&g_db_cs);

			// write to a temporary XML file
			wchar_t tempPath[MAX_PATH] = {0};
			GetTempPathW(MAX_PATH, tempPath);
			wchar_t tempFile[MAX_PATH] = {0};
			GetTempFileNameW(tempPath, L"lmx", 0, tempFile);
			SaveListToXML(&obj, tempFile, max);
			freeRecordList(&obj);
			
				// open the resultant file to read into a buffer
				// (we're basically using the filesystem as an automatically growing buffer)
			HANDLE plFile = CreateFileW(tempFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
			size_t flen = SetFilePointer(plFile, 0, NULL, FILE_END);
			SetFilePointer(plFile, 0, NULL, FILE_BEGIN);

			SAFEARRAY *bufferArray=SafeArrayCreateVector(VT_UI1, 0, flen);
			void *data;
			SafeArrayAccessData(bufferArray, &data);
			DWORD bytesRead = 0;
			ReadFile(plFile, data, flen, &bytesRead, 0);
			SafeArrayUnaccessData(bufferArray);
			CloseHandle(plFile);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_ARRAY|VT_UI1;
			V_ARRAY(pvarResult) = bufferArray;
			DeleteFileW(tempFile);
		}
		return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}


STDMETHODIMP LocalMediaCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG LocalMediaCOM::AddRef(void)
{
	return 0;
}


ULONG LocalMediaCOM::Release(void)
{
	return 0;
}
