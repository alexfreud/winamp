#include "main.h"
#include "SimpleFilter.h"
#include "../nu/sort.h"
#include "resource.h"
#include <shlwapi.h>
#include <strsafe.h>
static size_t m_sort_by, m_sort_dir, m_sort_which;

void emptyQueryListObject(queryListObject *obj);
int reallocQueryListObject(queryListObject *obj);
void freeQueryListObject(queryListObject *obj);


int SimpleFilter::SimpleSortFunc(const void *elem1, const void *elem2)
{
	queryListItem *a=(queryListItem*)elem1;
	queryListItem *b=(queryListItem*)elem2;
	int use_by=m_sort_by;
	int use_dir=m_sort_dir;
#define RETIFNZ(v) if ((v)<0) return use_dir?1:-1; if ((v)>0) return use_dir?-1:1;
	if (use_by == 0)
	{
		int v=WCSCMP_NULLOK(a->name,b->name);
		RETIFNZ(v)
		v=b->ifields[0]-a->ifields[0];
		RETIFNZ(v)
		v=b->ifields[1]-a->ifields[1];
		return v;
	}
	if (use_by == 1)
	{
		int v=a->ifields[0]-b->ifields[0];
		RETIFNZ(v)

		if (m_sort_which == 0)
		{
			v=b->ifields[1]-a->ifields[1];
			RETIFNZ(v)
		}

		return WCSCMP_NULLOK(a->name,b->name);
	}
	if (use_by == 2)
	{
		int v=a->ifields[1]-b->ifields[1];
		RETIFNZ(v)
		if (m_sort_which == 0)
		{
			v=b->ifields[0]-a->ifields[0];
			RETIFNZ(v)
		}
		return WCSCMP_NULLOK(a->name,b->name);
	}

	if (use_by == 5)
	{
		__int64 v = a->size - b->size;
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}

	if (use_by == 6)
	{
		int v = a->length - b->length;
		RETIFNZ(v)
		return WCSCMP_NULLOK(a->name, b->name);
	}

#undef RETIFNZ
	return 0;
}
void SimpleFilter::SortResults(C_Config *viewconf, int which, int isfirst) // sorts the results based on the current sort mode
{
	if (viewconf)
	{
		wchar_t buf[64] = {0};
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_sort_by_%d", GetConfigId(), which);
		m_sort_by = viewconf->ReadInt(buf, 0);
		StringCchPrintfW(buf, ARRAYSIZE(buf), L"%hs_sort_dir_%d", GetConfigId(), which);
		m_sort_dir = viewconf->ReadInt(buf, 0);
		m_sort_which = which;

		if (showncolumns.size()>m_sort_by && m_sort_by>=0) m_sort_by = showncolumns[m_sort_by]->field;

		if (m_sort_dir == 0 && m_sort_by == 0 && isfirst) return;

		if (artistList.Size > 2) qsort(artistList.Items+1,artistList.Size-1,sizeof(queryListItem),SimpleSortFunc);
	}
}

void SimpleFilter::Fill(itemRecordW *items, int numitems, int *killswitch, int numitems2)
{
	if (numitems > 1) 
		qsort_itemRecord(items,numitems, this, BuildSortFunc);

	if (killswitch && *killswitch) return;

	emptyQueryListObject(&artistList);
	reallocQueryListObject(&artistList);
	ZeroMemory(&artistList.Items[0],sizeof(queryListItem));

	wchar_t *lastname=0;
	const wchar_t *lastalb=0;
	wchar_t albbuf[100] = {0}, albbuf2[100] = {0}, albbuf3[100] = {0};

	itemRecordW *p=items;
	int n=numitems;

	int isbl=0;
	numGroups = 0;
	bool do_wcsdup = !this->uses_nde_strings();
	while (n--)
	{
		if (killswitch && *killswitch) return;
		const wchar_t *name = this->GroupText(p,albbuf3,100);
		if (!lastname || WCSCMP_NULLOK(lastname, name))
		{
			artistList.Size++;
			if (reallocQueryListObject(&artistList)) break;
			ZeroMemory(&artistList.Items[artistList.Size],sizeof(queryListItem));
			lastalb=0;
			if (name == albbuf3)
				lastname = artistList.Items[artistList.Size].name = ndestring_wcsdup(name);
			else if (name)
			{
				if (do_wcsdup)
				{
					lastname = artistList.Items[artistList.Size].name = ndestring_wcsdup(name);
				}
				else
				{
					wchar_t *ndename = (wchar_t *)name;
					ndestring_retain(ndename);
					lastname = artistList.Items[artistList.Size].name = ndename;
				}
			}
			else
			{
					lastname = artistList.Items[artistList.Size].name = emptyQueryListString;
			}

			SKIP_THE_AND_WHITESPACEW(lastname) // optimization technique
			if (*lastname) numGroups++;
		}
		if (!name || !*name) isbl++;
		if (artistList.Size)
		{
			artistList.Items[artistList.Size].ifields[1]++;
			if (p->length>0) artistList.Items[artistList.Size].length += p->length;
			if (p->filesize>0) artistList.Items[artistList.Size].size += p->filesize;
		}
		if (nextFilter && (!lastalb ||  WCSCMP_NULLOK(lastalb,nextFilter->GroupText(p,albbuf2,100))))
		{
			lastalb = nextFilter->GroupText(p,albbuf,100);
			if (lastalb && *lastalb) artistList.Items[artistList.Size].ifields[0]++;
			if (lastalb) SKIP_THE_AND_WHITESPACEW(lastalb) // optimization technique
			}
		p++;
	}

	if (killswitch && *killswitch) return;

	wchar_t langbuf[2048] = {0}, buf[64] = {0};
	if (isbl)
	{
		StringCchPrintfW(buf,64,WASABI_API_LNGSTRINGW_BUF(IDS_ALL_X_S, langbuf, 2048),artistList.Size-1,artistList.Size==2?GetNameSingular():GetName());
	}
	else
	{
		StringCchPrintfW(buf,64,WASABI_API_LNGSTRINGW_BUF(IDS_ALL_X_S, langbuf, 2048),artistList.Size,artistList.Size==1?GetNameSingular():GetName());
	}

	// for some languages a lowercased first word is invalid so need to prevent this from happening
	process_substantives(buf);

	artistList.Items[0].name=ndestring_wcsdup(buf);
	artistList.Items[0].ifields[0]=numitems2;
	artistList.Items[0].ifields[1]=numitems;
	artistList.Size++;
}

int SimpleFilter::Size()
{
	return artistList.Size;
}

const wchar_t *SimpleFilter::GetText(int row) // gets main text (first column)
{
	return artistList.Items[row].name;
}

void SimpleFilter::Empty()
{
	freeQueryListObject(&artistList);
}

void SimpleFilter::CopyText(int row, size_t column, wchar_t *dest, int destCch)
{
	if (column >= showncolumns.size()) return;
	column = showncolumns[column]->field;
	if (row>=artistList.Size)
		return ;
	switch (column)
	{
	case 0: // artist name
		if (artistList.Items[row].name && *artistList.Items[row].name)
			lstrcpynW(dest,artistList.Items[row].name,destCch);
		else
		{
			StringCchPrintfW(dest, destCch, WASABI_API_LNGSTRINGW(IDS_NO_S), GetNameSingular());
			// for some languages a lowercased first word is invalid so need to prevent this from happening
			process_substantives(dest);
		}
		break;
	case 1: // albums
		StringCchPrintfW(dest,destCch,L"%d",artistList.Items[row].ifields[0]);
		break;
	case 2: // tracks
		StringCchPrintfW(dest,destCch,L"%d",artistList.Items[row].ifields[1]);
		break;
	case 5:
		if (row && artistList.Items[row].size)
			WASABI_API_LNG->FormattedSizeString(dest, destCch, artistList.Items[row].size);
		else dest[0]=0;
		break;
	case 6:
		if (row && artistList.Items[row].length)
			StringCchPrintfW(dest,destCch,L"%d:%02d", artistList.Items[row].length / 60, artistList.Items[row].length % 60);
		else dest[0]=0;
		break;
	}
}

const wchar_t * SimpleFilter::CopyText2(int row, size_t column, wchar_t *dest, int destCch)
{
	if (column >= showncolumns.size()) return NULL;
	column = showncolumns[column]->field;
	if (row>=artistList.Size)
		return NULL;
	switch (column)
	{
	case 0: // artist name
		if (artistList.Items[row].name && *artistList.Items[row].name)
			dest = artistList.Items[row].name;
		else
		{
			StringCchPrintfW(dest, destCch, WASABI_API_LNGSTRINGW(IDS_NO_S), GetNameSingular());
			// for some languages a lowercased first word is invalid so need to prevent this from happening
			process_substantives(dest);
		}
		break;
	case 1: // albums
		StringCchPrintfW(dest,destCch,L"%d",artistList.Items[row].ifields[0]);
		break;
	case 2: // tracks
		StringCchPrintfW(dest,destCch,L"%d",artistList.Items[row].ifields[1]);
		break;
	case 5:
		if (row && artistList.Items[row].size)
			WASABI_API_LNG->FormattedSizeString(dest, destCch, artistList.Items[row].size);
		else dest[0]=0;
		break;
	case 6:
		if (row && artistList.Items[row].length)
			StringCchPrintfW(dest,destCch,L"%d:%02d", artistList.Items[row].length / 60, artistList.Items[row].length % 60);
		else dest[0]=0;
		break;
	}
	return dest;
}
void SimpleFilter::AddColumns2()
{
	showncolumns.push_back(new ListField(0,155,GetNameSingular(),g_view_metaconf,GetConfigId()));
	if (nextFilter) showncolumns.push_back(new ListField(1,48,nextFilter->GetName(),g_view_metaconf,GetConfigId()));
	showncolumns.push_back(new ListField(2,45,IDS_TRACKS,g_view_metaconf,GetConfigId()));
	showncolumns.push_back(new ListField(5,45,IDS_SIZE,g_view_metaconf,GetConfigId(),true,true));
	showncolumns.push_back(new ListField(6,45,IDS_LENGTH,g_view_metaconf,GetConfigId(),true,true));
}