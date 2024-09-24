#include "Main.h"
#include "ml_local.h"
#include "resource.h"

static INT_PTR CALLBACK CompactWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM(0, 400));
			SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETPOS, 0, 0);

			int *progress = (int *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (progress[1] == -102)
			{
				progress[1] = -101;
				SetWindowTextW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_REFRESH_FILESIZE_DATEADDED));
			}

			SetTimer(hwndDlg, 666, 500, 0);
			break;
		}
		case WM_TIMER:
			if (wParam == 666)
			{
				int *progress = (int *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (progress[0] == 666)
				{
					KillTimer(hwndDlg, 666);
					EndDialog(hwndDlg, 0);
				}
				else if (progress[1] != -101)
				{
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETPOS, progress[1] + 300, 0);
				}
				else
				{
					SendDlgItemMessage(hwndDlg, IDC_PROGRESS1, PBM_SETPOS, progress[0] + 100, 0);
				}
			}
			break;
	}
	return 0;
}

static DWORD CALLBACK CompactThread(LPVOID param)
{
	WASABI_API_DIALOGBOXPARAMW(IDD_REINDEX, NULL, CompactWndProc, (LPARAM)param);
	return 0;
}

static int sortFunc(const void *a, const void *b)
{
	const wchar_t **fn1 = (const wchar_t **)a;
	const wchar_t **fn2 = (const wchar_t **)b;
	return _wcsicmp(*fn1, *fn2);
}

void RetypeFilename(nde_table_t table)
{
	int totalRecords = NDE_Table_GetRecordsCount(g_table);
	if (totalRecords == 0) // bail out early so we don't flash a dialog
		return;

	int progress[2] = {-100, -101};
	DWORD threadId = 0;
	HANDLE compactThread = 0;

	nde_scanner_t pruneScanner = NDE_Table_CreateScanner(table);
	if (pruneScanner)
	{
		bool first=true;
		int recordNum = 0;
		NDE_Scanner_First(pruneScanner);
		while (!NDE_Scanner_EOF(pruneScanner))
		{
			progress[0] = MulDiv(recordNum, 100, totalRecords)-100;
			nde_field_t f = NDE_Scanner_GetFieldByID(pruneScanner, MAINTABLE_ID_FILENAME);
			if (f && NDE_Field_GetType(f) == FIELD_STRING)
			{
				wchar_t *s = NDE_StringField_GetString(f);
				ndestring_retain(s);

				NDE_Scanner_DeleteField(pruneScanner, f);

				nde_field_t new_f = NDE_Scanner_NewFieldByID(pruneScanner, MAINTABLE_ID_FILENAME);
				NDE_StringField_SetString(new_f, s);

				ndestring_release(s);
				NDE_Scanner_Post(pruneScanner);
			}
			else if (f)
			{
				first = false; // skips creating the thread
				break;
			}

			recordNum++;
			NDE_Scanner_Next(pruneScanner);
			if (first)
			{
				compactThread = CreateThread(0, 0, CompactThread, progress, 0, &threadId);
				DumpArtCache(); // go ahead and dump the album art cache if we had to rebuild this table.  ideally we could perform the same logic but this is easier and it's 1 in the morning and i don't feel like doing it :)
			}
			first=false;
		}

		NDE_Table_DestroyScanner(table, pruneScanner);
		if (compactThread)
			NDE_Table_Sync(table);
	}
	progress[0] = 666;
	if (compactThread)
	{
		WaitForSingleObject(compactThread, INFINITE);
		CloseHandle(compactThread);
	}
}

void RefreshFileSizeAndDateAddedTable(nde_table_t table)
{
	int totalRecords = NDE_Table_GetRecordsCount(g_table);
	if (totalRecords == 0) // bail out early so we don't flash a dialog
		return;

	int progress[2] = {-100, -102};
	DWORD threadId = 0;
	HANDLE compactThread = 0;

	nde_scanner_t scanner = NDE_Table_CreateScanner(table);
	if (scanner)
	{
		bool first=true;
		int recordNum = 0;
		NDE_Scanner_First(scanner);
		while (!NDE_Scanner_EOF(scanner))
		{
			progress[0] = MulDiv(recordNum, 400, totalRecords);

			// converts filesize from a int and kb scaled value to the actual filesize as a int64
			nde_field_t f = NDE_Scanner_GetFieldByID(scanner, MAINTABLE_ID_FILESIZE);
			if (f && NDE_Field_GetType(f) == FIELD_INTEGER)
			{
				__int64 size = NDE_IntegerField_GetValue(f);
				if (size) size *= 1024;

				NDE_Scanner_DeleteField(scanner, f);

				nde_field_t new_f = NDE_Scanner_NewFieldByType(scanner, FIELD_INT64, MAINTABLE_ID_FILESIZE);
				NDE_Int64Field_SetValue(new_f, size);

				NDE_Scanner_Post(scanner);
			}

			// takes the lastupd value and applies it to dateadded so we've got something to use
			f = NDE_Scanner_GetFieldByID(scanner, MAINTABLE_ID_LASTUPDTIME);
			if (f && NDE_Field_GetType(f) == FIELD_DATETIME)
			{
				int lastupd = NDE_IntegerField_GetValue(f);
				nde_field_t new_f = NDE_Scanner_NewFieldByID(scanner, MAINTABLE_ID_DATEADDED);
				NDE_IntegerField_SetValue(new_f, lastupd);

				NDE_Scanner_Post(scanner);
			}

			NDE_Scanner_Next(scanner);
			recordNum++;
			if (first)
			{
				compactThread = CreateThread(0, 0, CompactThread, progress, 0, &threadId);
			}
			first=false;
		}

		NDE_Table_DestroyScanner(table, scanner);
		if (compactThread)
			NDE_Table_Sync(table);
	}
	progress[0] = 666;
	if (compactThread)
	{
		WaitForSingleObject(compactThread, INFINITE);
		CloseHandle(compactThread);
	}
}

void ReindexTable(nde_table_t table)
{
	int totalRecords = NDE_Table_GetRecordsCount(g_table);
	if (totalRecords == 0) // bail out early so we don't flash a dialog
		return;

	int progress[2] = {-100, -101};
	DWORD threadId;
	HANDLE compactThread = CreateThread(0, 0, CompactThread, progress, 0, &threadId);

	nde_scanner_t pruneScanner = NDE_Table_CreateScanner(table);
	if (pruneScanner)
	{
		int recordNum = 0;
		NDE_Scanner_First(pruneScanner);
		while (!NDE_Scanner_EOF(pruneScanner))
		{
			int total = MulDiv(recordNum, 100, totalRecords);
			progress[0] = total - 100;
			#ifndef elementsof
			#define elementsof(x) (sizeof(x)/sizeof(*x))
			#endif
			unsigned char STR_IDS[] = {MAINTABLE_ID_TITLE, MAINTABLE_ID_ARTIST, MAINTABLE_ID_ALBUM, MAINTABLE_ID_GENRE,
			                           MAINTABLE_ID_COMMENT, MAINTABLE_ID_GRACENOTE_ID, MAINTABLE_ID_ALBUMARTIST,
									   MAINTABLE_ID_TRACKGAIN, MAINTABLE_ID_PUBLISHER, MAINTABLE_ID_COMPOSER,
									   MAINTABLE_ID_PODCASTCHANNEL, MAINTABLE_ID_GRACENOTEFILEID, MAINTABLE_ID_GRACENOTEEXTDATA,
									   MAINTABLE_ID_CATEGORY, MAINTABLE_ID_CODEC, MAINTABLE_ID_DIRECTOR, MAINTABLE_ID_PRODUCER
			                          };
			for (size_t i = 0;i != elementsof(STR_IDS);i++)
			{
				nde_field_t f = NDE_Scanner_GetFieldByID(pruneScanner, STR_IDS[i]);
				if (f)
				{
					const wchar_t *s = NDE_StringField_GetString(f);
					if (!s || !*s)
					{
						NDE_Scanner_DeleteField(pruneScanner, f);
						NDE_Scanner_Post(pruneScanner);
					}
				}
			}

			unsigned char INT_IDS_ZEROOK[] = {MAINTABLE_ID_LENGTH, MAINTABLE_ID_PLAYCOUNT, MAINTABLE_ID_FILESIZE,
											  MAINTABLE_ID_TYPE, MAINTABLE_ID_ISPODCAST, MAINTABLE_ID_LOSSLESS
											 };
			for (size_t i = 0;i != elementsof(INT_IDS_ZEROOK);i++)
			{
				nde_field_t f = NDE_Scanner_GetFieldByID(pruneScanner, INT_IDS_ZEROOK[i]);
				if (f)
				{
					int s = NDE_IntegerField_GetValue(f);
					if (s < 0)
					{
						NDE_Scanner_DeleteField(pruneScanner, f);
						NDE_Scanner_Post(pruneScanner);

					}
				}
			}

			unsigned char INT_IDS[] = {MAINTABLE_ID_TRACKNB, MAINTABLE_ID_LASTUPDTIME, MAINTABLE_ID_LASTPLAY, MAINTABLE_ID_RATING,
			                           MAINTABLE_ID_FILETIME, MAINTABLE_ID_BITRATE, MAINTABLE_ID_DISC, MAINTABLE_ID_BPM, MAINTABLE_ID_DISCS,
									   MAINTABLE_ID_TRACKS, MAINTABLE_ID_PODCASTPUBDATE, MAINTABLE_ID_FILESIZE, MAINTABLE_ID_DATEADDED
			                          };
			for (size_t i = 0;i != elementsof(INT_IDS);i++)
			{
				nde_field_t f = NDE_Scanner_GetFieldByID(pruneScanner, INT_IDS[i]);
				if (f)
				{
					int s = NDE_IntegerField_GetValue(f);
					if (s <= 0)
					{
						NDE_Scanner_DeleteField(pruneScanner, f);
						NDE_Scanner_Post(pruneScanner);
					}
				}
			}
			NDE_Scanner_Next(pruneScanner);
			recordNum++;
		}

		NDE_Table_DestroyScanner(table, pruneScanner);
		NDE_Table_Sync(table);
	}

	NDE_Table_Compact(table, &progress[0]);
	assert(((Table *)table)->CheckIndexing());
	// now remove duplicates
	nde_scanner_t dupscanner = NDE_Table_CreateScanner(table);
	if (dupscanner)
	{
		int count = NDE_Scanner_GetRecordsCount(dupscanner);
		wchar_t **filenames = new wchar_t *[count];
		int i = 0;
		for (NDE_Scanner_First(dupscanner);!NDE_Scanner_EOF(dupscanner);NDE_Scanner_Next(dupscanner))
		{
			nde_field_t fileName = NDE_Scanner_GetFieldByID(dupscanner, MAINTABLE_ID_FILENAME);
			if (fileName)
			{
				filenames[i] = NDE_StringField_GetString(fileName);
				ndestring_retain(filenames[i]);
				i++;
			}
		}
		count = i;
		if (count)
		{
			qsort(filenames, count, sizeof(wchar_t *), sortFunc);
			for (int x = 0;x < (count - 1);x++)
			{
				int total = MulDiv(x, 100, count);
				progress[1] = total - 100;

				if (!_wcsicmp(filenames[x], filenames[x+1]))
				{
					wchar_t query[1024] = {0};
					wnsprintfW(query, 1024, L"filename == \"%s\"", filenames[x]);

					nde_scanner_t scanner = NDE_Table_CreateScanner(table);
					NDE_Scanner_Query(scanner, query);

					NDE_Scanner_First(scanner);
					NDE_Scanner_Next(scanner);
					while (!NDE_Scanner_EOF(scanner))
					{
						NDE_Scanner_Delete(scanner);
						NDE_Scanner_Post(scanner);
						NDE_Scanner_Next(scanner);
					}
					NDE_Table_DestroyScanner(table, scanner);
				}
				ndestring_release(filenames[x]);
				filenames[x]=0;
			}
		}
		delete[] filenames;
	}
	NDE_Table_DestroyScanner(table, dupscanner);
	NDE_Table_Sync(table);
	NDE_Table_Compact(table, &progress[1]);

	progress[0] = 666;
	WaitForSingleObject(compactThread, INFINITE);
	CloseHandle(compactThread);
}