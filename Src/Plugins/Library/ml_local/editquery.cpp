#include "main.h"
#include "ml_local.h"
#include <windowsx.h>
#include "editquery.h"
#include "../nde/NDE.h"
#include "resource.h"
#include "..\..\General\gen_ml/gaystring.h"
#include "time.h"
#include "../Agave/Language/api_language.h"
#include "../replicant/nu/AutoChar.h"
#define ListBox_GetTextLenW(hwndCtl, index)          ((int)(DWORD)SendMessageW((hwndCtl), LB_GETTEXTLEN, (WPARAM)(int)(index), 0L))
#define ListBox_GetTextW(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SendMessageW((hwndCtl), LB_GETTEXT, (WPARAM)(int)(index), (LPARAM)(LPCWSTR)(lpszBuffer)))

int myatoi(wchar_t *p, int len) {
	wchar_t *w = (wchar_t *)calloc((len+1), sizeof(wchar_t));
	if (w)
	{
		wcsncpy(w, p, len);
		w[len] = 0;
		int a = _wtoi(w);
		free(w);
		return a;
	}
	return 0;
}

static wchar_t *monthtable[12] = {L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };
INT_PTR CALLBACK editQueryDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK editDateTimeDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static wchar_t *qe_field = NULL;

static GayStringW qe_curquery;
static GayStringW qe_curexpr;
static GayStringW qe_origquery;
static GayStringW qe_curorigin;
static GayStringW qe_curdate;

void qe_freeStuff() {
  qe_curquery.Set(L"");
  qe_curquery.Compact();
  qe_curexpr.Set(L"");
  qe_curexpr.Compact();
  qe_curorigin.Set(L"");
  qe_curorigin.Compact();
  qe_origquery.Set(L"");
  qe_origquery.Compact();
}

static GayStringW te_ret;
static GayStringW te_result;
int te_cur_selected_month=0;

const wchar_t *editTime(HWND wnd, const wchar_t *curfield) {
  te_ret.Set(curfield);

  TimeParse tp;
	NDE_Time_ApplyConversion(0, curfield, &tp);

  if (tp.relative_day == -2 || tp.relative_kwday != -1) {
	  wchar_t titleStr[64] = {0};
	  int r = MessageBoxW(wnd, WASABI_API_LNGSTRINGW(IDS_DATE_TIME_IS_TOO_COMPLEX),
						 WASABI_API_LNGSTRINGW_BUF(IDS_DATE_TIME_EDITOR_QUESTION,titleStr,64), MB_YESNO);
    if (r == IDNO) {
      return curfield;
    }
  }

  INT_PTR r = WASABI_API_DIALOGBOXW(IDD_TIMEEDITOR, wnd, editDateTimeDialogProc);
  if (r == IDOK) {
    te_ret.Set(te_result.Get());
    return te_ret.Get();
  }
  
  return NULL;
}

const wchar_t *editQuery(HWND wnd, const wchar_t *curquery, wchar_t *newQuery, size_t len) 
{
  qe_field = NULL;

  qe_curquery.Set(curquery);
  qe_origquery.Set(curquery);

  INT_PTR r = WASABI_API_DIALOGBOXW(IDD_EDIT_QUERY, wnd, editQueryDialogProc);

  if (qe_field) free(qe_field);
  
  if (r == IDOK) 
	{
		lstrcpynW(newQuery, qe_curquery.Get(),  len);
    qe_freeStuff();
    return newQuery;
  }
  qe_freeStuff();
  return NULL;
}

static bool FillListProc(Record *record, Field *entry, void *context)
{
	HWND hwndDlg = (HWND)context;
	SendMessage(hwndDlg, LB_ADDSTRING, 0, (LPARAM)NDE_ColumnField_GetFieldName((nde_field_t)entry));
	return true;
}

void qe_fillFieldsList( HWND hwndDlg )
{
    ListBox_ResetContent( hwndDlg );

    Record *cr = ( (Table *)g_table )->GetColumns(); // TODO: don't use C++ NDE API
    if ( cr )
    {
        cr->WalkFields( FillListProc, (void *)hwndDlg );
    }

    int len = ListBox_GetTextLenW( hwndDlg, 0 );

    if ( qe_field != NULL )
        free( qe_field );

    qe_field = (wchar_t *)calloc( ( len + 1 ), sizeof( wchar_t ) );
    ListBox_GetTextW( hwndDlg, 0, qe_field );

    qe_field[ len ] = 0;
    ListBox_SetCurSel( hwndDlg, 0 );
}

void qe_doEnableDisable( HWND hwndDlg, int *n, int num, int how )
{
    for ( int i = 0; i < num; i++ )
        EnableWindow( GetDlgItem( hwndDlg, n[ i ] ), how );
}

void qe_enableDisableItem( HWND hwndDlg, int id, int tf )
{
    EnableWindow( GetDlgItem( hwndDlg, id ), tf );
}

void qe_fallback( HWND hwndDlg, int disabling, int enabled )
{
    if ( IsDlgButtonChecked( hwndDlg, disabling ) )
    {
        CheckDlgButton( hwndDlg, disabling, BST_UNCHECKED );
        CheckDlgButton( hwndDlg, enabled, BST_CHECKED );
    }
}

int ctrls_datetime[] = {IDC_STATIC_DATETIME, IDC_EDIT_DATETIME, IDC_BUTTON_EDITDATETIME, IDC_STATIC_CURDATETIME};
int ctrls_datetime_base[] = {IDC_STATIC_DATETIME, IDC_CHECK_ABSOLUTE, IDC_CHECK_RELATIVE};
int ctrls_datetime_relative[] = {IDC_CHECK_SELECTIVE, IDC_CHECK_TIMEAGO, IDC_STATIC_QUERYTIME, IDC_STATIC_RESULT};
int ctrls_datetime_relative_ago[] = {IDC_STATIC_TIMEAGO, IDC_EDIT_TIMEAGO, IDC_RADIO_TIMEAGO_Y,
      IDC_RADIO_TIMEAGO_H, IDC_RADIO_TIMEAGO_M, IDC_RADIO_TIMEAGO_MIN, IDC_RADIO_TIMEAGO_W, IDC_RADIO_TIMEAGO_S,
      IDC_RADIO_TIMEAGO_D};
int ctrls_datetime_relative_ago_direction[] = {IDC_STATIC_DIRECTION, IDC_RADIO_AFTER, IDC_RADIO_BEFORE};
int ctrls_datetime_relative_selective[] = {IDC_STATIC_SELECTIVE, IDC_STATIC_YEAR, IDC_RADIO_THISYEAR, 
      IDC_RADIO_YEAR, IDC_EDIT_YEAR, IDC_STATIC_MONTH, IDC_RADIO_THISMONTH, IDC_RADIO_MONTH, IDC_BUTTON_MONTH,
      IDC_STATIC_DAY, IDC_RADIO_THISDAY, IDC_RADIO_DAY, IDC_EDIT_DAY, IDC_STATIC_TIME, IDC_RADIO_THISTIME, 
      IDC_RADIO_TIME, IDC_RADIO_NOON, IDC_RADIO_MIDNIGHT, IDC_BUTTON_NOW, IDC_BUTTON_PICK, IDC_DATETIMEPICKER2};
int ctrls_datetime_absolute[] = {IDC_STATIC_ABSOLUTE, IDC_DATETIMEPICKER, IDC_DATETIMEPICKER1, IDC_STATIC_RELATIVE};
int ctrls_string_ops[] = {IDC_RADIO_ISLIKE, IDC_RADIO_BEGINS, IDC_RADIO_ENDS, IDC_RADIO_CONTAINS};

int ctrls_string[] = {IDC_STATIC_STRING, IDC_EDIT_STRING};

void qe_showHideOperators(HWND hwndDlg) {
  if (!qe_curquery.Get() || !*qe_curquery.Get()) {
    ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_AND), SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_OR), SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg, ID_BUTTON_SENDTOQUERY), SW_SHOW);
  } else {
    ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_AND), SW_SHOW);
    ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_OR), SW_SHOW);
    ShowWindow(GetDlgItem(hwndDlg, ID_BUTTON_SENDTOQUERY), SW_HIDE);
  }
}

void qe_updateResultingDate(HWND hwndDlg) {
	if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT_DATETIME)) && !IsDlgButtonChecked(hwndDlg, IDC_RADIO_ISEMPTY)) {
		time_t t = NDE_Time_ApplyConversion(0, qe_curdate.Get(), 0);
		struct tm *ts = localtime(&t);
		wchar_t qe_tempstr[4096] = {0};
		if (ts)
			wsprintfW(qe_tempstr, L"%02d/%d/%04d %02d:%02d:%02d", ts->tm_mon+1, ts->tm_mday, ts->tm_year+1900, ts->tm_hour, ts->tm_min, ts->tm_sec);
		else
			wcsncpy(qe_tempstr, L"DATE_OUTOFRANGE", 4096);
		SetDlgItemTextW(hwndDlg, IDC_STATIC_CURDATETIME, qe_tempstr);
	} else {
		SetDlgItemTextW(hwndDlg, IDC_STATIC_CURDATETIME, L"N/A");
	}
}

void te_updateResultingDate(HWND hwndDlg) {
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELATIVE)) {
		time_t t = NDE_Time_ApplyConversion(0, qe_curorigin.Get(), 0);
		struct tm *ts = localtime(&t);
		wchar_t qe_tempstr[4096] = {0};
		if (ts)
			wsprintfW(qe_tempstr, L"%02d/%d/%04d %02d:%02d:%02d", ts->tm_mon+1, ts->tm_mday, ts->tm_year+1900, ts->tm_hour, ts->tm_min, ts->tm_sec);
		else
			wcsncpy(qe_tempstr, L"DATE_OUTOFRANGE", 4096);
		SetDlgItemTextW(hwndDlg, IDC_STATIC_QUERYTIME, qe_tempstr);
	} else {
		SetDlgItemTextW(hwndDlg, IDC_STATIC_QUERYTIME, L"N/A");
	}
}

void qe_enableDisable(HWND hwndDlg) {
  if (!*qe_field) return;
  nde_field_t field = NDE_Table_GetColumnByName(g_table, AutoChar(qe_field));
  if (!field) return;
  int type = NDE_ColumnField_GetDataType(field);

  qe_showHideOperators(hwndDlg);

  switch (type) {
    case FIELD_DATETIME: {
      qe_fallback(hwndDlg, IDC_RADIO_ISLIKE, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_BEGINS, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_ENDS, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_CONTAINS, IDC_RADIO_EQUAL);
      qe_doEnableDisable(hwndDlg, ctrls_string, sizeof(ctrls_string)/sizeof(int), 0);
      qe_doEnableDisable(hwndDlg, ctrls_string_ops, sizeof(ctrls_string_ops)/sizeof(int), 0);
	  SetDlgItemTextW(hwndDlg,IDC_STATIC_STRING,WASABI_API_LNGSTRINGW(IDS_COMPARE_TO_STRING));
      qe_doEnableDisable(hwndDlg, ctrls_datetime, sizeof(ctrls_datetime)/sizeof(int), 1);
	  SetDlgItemTextW(hwndDlg, IDC_RADIO_ABOVE, WASABI_API_LNGSTRINGW(IDS_AFTER));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOW, WASABI_API_LNGSTRINGW(IDS_BEFORE));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_ABOVEOREQUAL, WASABI_API_LNGSTRINGW(IDS_SINCE));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOWOREQUAL, WASABI_API_LNGSTRINGW(IDS_UNTIL));
      break;
    }
    case FIELD_LENGTH: {
      qe_fallback(hwndDlg, IDC_RADIO_ISLIKE, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_BEGINS, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_ENDS, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_CONTAINS, IDC_RADIO_EQUAL);
      qe_doEnableDisable(hwndDlg, ctrls_string, sizeof(ctrls_string)/sizeof(int), 1);
      qe_doEnableDisable(hwndDlg, ctrls_string_ops, sizeof(ctrls_string_ops)/sizeof(int), 0);
      SetDlgItemTextW(hwndDlg,IDC_STATIC_STRING,WASABI_API_LNGSTRINGW(IDS_COMPARE_TO_LENGTH));
      qe_doEnableDisable(hwndDlg, ctrls_datetime, sizeof(ctrls_datetime)/sizeof(int), 0);
      SetDlgItemTextW(hwndDlg, IDC_RADIO_ABOVE, WASABI_API_LNGSTRINGW(IDS_ABOVE));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOW, WASABI_API_LNGSTRINGW(IDS_BELOW));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_ABOVEOREQUAL, WASABI_API_LNGSTRINGW(IDS_ABOVE_OR_EQUAL));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOWOREQUAL, WASABI_API_LNGSTRINGW(IDS_BELOW_OR_EQUAL));
      break;
    }
		case FIELD_FILENAME:
    case FIELD_STRING: {
      qe_doEnableDisable(hwndDlg, ctrls_string, sizeof(ctrls_string)/sizeof(int), 1);
      qe_doEnableDisable(hwndDlg, ctrls_string_ops, sizeof(ctrls_string_ops)/sizeof(int), 1);
	  SetDlgItemTextW(hwndDlg,IDC_STATIC_STRING,WASABI_API_LNGSTRINGW(IDS_COMPARE_TO_STRING));
      qe_doEnableDisable(hwndDlg, ctrls_datetime, sizeof(ctrls_datetime)/sizeof(int), 0);
	  SetDlgItemTextW(hwndDlg, IDC_RADIO_ABOVE, WASABI_API_LNGSTRINGW(IDS_ABOVE));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOW, WASABI_API_LNGSTRINGW(IDS_BELOW));
      SetDlgItemTextW(hwndDlg,IDC_RADIO_ABOVEOREQUAL, WASABI_API_LNGSTRINGW(IDS_ABOVE_OR_EQUAL));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOWOREQUAL, WASABI_API_LNGSTRINGW(IDS_BELOW_OR_EQUAL));
      break;
    }
    case FIELD_INTEGER: {
      qe_fallback(hwndDlg, IDC_RADIO_ISLIKE, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_BEGINS, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_ENDS, IDC_RADIO_EQUAL);
      qe_fallback(hwndDlg, IDC_RADIO_CONTAINS, IDC_RADIO_EQUAL);
      SetDlgItemTextW(hwndDlg,IDC_STATIC_STRING,WASABI_API_LNGSTRINGW(IDS_COMPARE_TO_NUMBER));
      qe_doEnableDisable(hwndDlg, ctrls_string, sizeof(ctrls_string)/sizeof(int), 1);
      qe_doEnableDisable(hwndDlg, ctrls_string_ops, sizeof(ctrls_string_ops)/sizeof(int), 0);
      qe_doEnableDisable(hwndDlg, ctrls_datetime, sizeof(ctrls_datetime)/sizeof(int), 0);
      SetDlgItemTextW(hwndDlg, IDC_RADIO_ABOVE, WASABI_API_LNGSTRINGW(IDS_ABOVE));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOW, WASABI_API_LNGSTRINGW(IDS_BELOW));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_ABOVEOREQUAL, WASABI_API_LNGSTRINGW(IDS_ABOVE_OR_EQUAL));
      SetDlgItemTextW(hwndDlg, IDC_RADIO_BELOWOREQUAL, WASABI_API_LNGSTRINGW(IDS_BELOW_OR_EQUAL));
      break;
    }
  }

  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ISEMPTY)) {
    qe_doEnableDisable(hwndDlg, ctrls_string, sizeof(ctrls_string)/sizeof(int), 0);
    qe_doEnableDisable(hwndDlg, ctrls_datetime, sizeof(ctrls_datetime)/sizeof(int), 0);
    return;
  }
}

void te_enableDisable(HWND hwndDlg) {
  qe_doEnableDisable(hwndDlg, ctrls_datetime_relative, sizeof(ctrls_datetime_relative)/sizeof(int), IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELATIVE));
  qe_doEnableDisable(hwndDlg, ctrls_datetime_absolute, sizeof(ctrls_datetime_absolute)/sizeof(int), IsDlgButtonChecked(hwndDlg, IDC_CHECK_ABSOLUTE));
  qe_doEnableDisable(hwndDlg, ctrls_datetime_relative_ago, sizeof(ctrls_datetime_relative_ago)/sizeof(int), IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELATIVE) && IsDlgButtonChecked(hwndDlg, IDC_CHECK_TIMEAGO));
  qe_doEnableDisable(hwndDlg, ctrls_datetime_relative_ago_direction, sizeof(ctrls_datetime_relative_ago_direction)/sizeof(int), !IsDlgButtonChecked(hwndDlg, IDC_CHECK_ABSOLUTE) && IsDlgButtonChecked(hwndDlg, IDC_CHECK_SELECTIVE) && IsDlgButtonChecked(hwndDlg, IDC_CHECK_TIMEAGO));
  qe_doEnableDisable(hwndDlg, ctrls_datetime_relative_selective, sizeof(ctrls_datetime_relative_selective)/sizeof(int), IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELATIVE) && IsDlgButtonChecked(hwndDlg, IDC_CHECK_SELECTIVE));
  int origin = IsDlgButtonChecked(hwndDlg, IDC_CHECK_SELECTIVE) && IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELATIVE);
  qe_enableDisableItem(hwndDlg, IDC_EDIT_YEAR, origin && IsDlgButtonChecked(hwndDlg, IDC_RADIO_YEAR));
  qe_enableDisableItem(hwndDlg, IDC_BUTTON_MONTH, origin && IsDlgButtonChecked(hwndDlg, IDC_RADIO_MONTH));
  qe_enableDisableItem(hwndDlg, IDC_EDIT_DAY, origin && IsDlgButtonChecked(hwndDlg, IDC_RADIO_DAY));
  qe_enableDisableItem(hwndDlg, IDC_DATETIMEPICKER2, origin && IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIME));

  SetDlgItemTextW(hwndDlg, IDC_CHECK_TIMEAGO, WASABI_API_LNGSTRINGW((IsDlgButtonChecked(hwndDlg, IDC_CHECK_SELECTIVE) ? IDS_OFFSET_BY : IDS_TIME_AGO)));
  SetWindowTextW(GetDlgItem(hwndDlg, IDC_BUTTON_MONTH), monthtable[te_cur_selected_month]);
  
  InvalidateRect(GetDlgItem(hwndDlg, IDC_CHECK_ABSOLUTE), NULL, TRUE);
  InvalidateRect(GetDlgItem(hwndDlg, IDC_CHECK_RELATIVE), NULL, TRUE);
  InvalidateRect(GetDlgItem(hwndDlg, IDC_CHECK_TIMEAGO), NULL, TRUE);
  InvalidateRect(GetDlgItem(hwndDlg, IDC_CHECK_SELECTIVE), NULL, TRUE);
  InvalidateRect(GetDlgItem(hwndDlg, IDC_STATIC_DIRECTION), NULL, TRUE);
}

const wchar_t *te_getDateTime(HWND hwndDlg) {
  static GayStringW str;
	wchar_t qe_tempstr[4096] = {0};
  str.Set(L"");
  if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_ABSOLUTE)) {
    GayStringW sd;
    SYSTEMTIME st={0,0,0,0,0,0,0,0};
    DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER), &st);
    if (st.wYear != 0) {
      wsprintfW(qe_tempstr, L"%02d/%d/%02d", st.wMonth, st.wDay, st.wYear);
      sd.Append(qe_tempstr);
    }
    SYSTEMTIME stt={0,0,0,0,0,0,0,0};
    DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER1), &stt);
    if (stt.wYear != 0) {
      if (sd.Get() && *sd.Get()) sd.Append(L" ");
      wsprintfW(qe_tempstr, L"%02d:%02d:%02d", stt.wHour, stt.wMinute, stt.wSecond);
      sd.Append(qe_tempstr);
    }
    if (sd.Get() && *sd.Get()) {
      str.Set(sd.Get());
      return str.Get();
    }
  } else if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELATIVE)) {
    GayStringW sd;
    int gotshit = 0;
    int gotmonth = 0;
    int gotthismonth = 0;
    int gotday = 0;
    int lgotthistime = 0;
    int lgotthisday = 0;
    if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_TIMEAGO)) {
      int v = GetDlgItemInt(hwndDlg, IDC_EDIT_TIMEAGO, NULL, TRUE);
      wsprintfW(qe_tempstr, L"%d", v);
      sd.Append(qe_tempstr);
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIMEAGO_Y)) { sd.Append(L" year"); if (v > 1) sd.Append(L"s"); }
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIMEAGO_M)) { sd.Append(L" month"); if (v > 1) sd.Append(L"s"); }
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIMEAGO_W)) { sd.Append(L" week"); if (v > 1) sd.Append(L"s"); }
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIMEAGO_D)) { sd.Append(L" day"); if (v > 1) sd.Append(L"s"); }
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIMEAGO_H)) sd.Append(L" h"); 
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIMEAGO_MIN)) sd.Append(L" mn"); 
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIMEAGO_S)) sd.Append(L" s"); 
    }
    if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SELECTIVE)) {
		if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_TIMEAGO) && IsDlgButtonChecked(hwndDlg, IDC_RADIO_AFTER)) sd.Append(L" after");
      if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_TIMEAGO) && IsDlgButtonChecked(hwndDlg, IDC_RADIO_BEFORE)) sd.Append(L" before");
      if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISDAY) &&
          IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH)) {
        if (sd.Get() && *sd.Get()) sd.Append(L" ");
        int v = GetDlgItemInt(hwndDlg, IDC_EDIT_DAY, NULL, TRUE);
		wsprintfW(qe_tempstr, L"%s%d", !gotmonth ? L"the " : L"", v);
        int u = (v - ((int)((double)v/10.0)) * 10);
        if (v < 1 || v > 31) {
          sd.Append(L"DAY_OUTOFRANGE");
          gotshit = 1;
        } else {
          sd.Append(qe_tempstr);
          switch (u) {
			case 1: sd.Append(L"st"); gotshit = 1; break;
            case 2: sd.Append(L"nd"); gotshit = 1; break;
            case 3: sd.Append(L"rd"); gotshit = 1; break;
            default: sd.Append(L"th"); gotshit = 1; break;
          }
        }
        if (!gotthismonth && IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH)) {
		  sd.Append(L" of this month");
          gotthismonth = 1;
        }
        gotshit = 1;
        gotday = 1;
      }
      if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISYEAR) &&
          IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH) &&
          IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISDAY) &&
          IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISTIME)) {
          if (sd.Get() && *sd.Get()) sd.Append(L" ");
		  sd.Append(L"now");
          gotshit = 1;
      } else {
        if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISYEAR) &&
          IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH) &&
          IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISDAY)) {
          if (gotshit) sd.Append(L",");
          if (sd.Get() && *sd.Get()) sd.Append(L" ");
		  sd.Append(L"this date");
          gotshit = 1;
        } else {
          if (!gotthismonth && IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH) && !IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISYEAR)) {
            if (gotshit) sd.Append(L",");
            if (sd.Get() && *sd.Get()) sd.Append(L" ");
			sd.Append(L"this month");
            gotthismonth = 1;
            gotshit = 1;
          } 
          if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISDAY)) {
            if (gotshit) sd.Append(L",");
            if (sd.Get() && *sd.Get()) sd.Append(L" ");
			sd.Append(L"this day");
            lgotthisday = 1;
            gotshit = 1;
          }
          if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISTIME)) {
            if (gotshit) sd.Append(L",");
            if (sd.Get() && *sd.Get()) sd.Append(L" ");
			sd.Append(L"this time");
            lgotthistime = 1;
            lgotthisday = 0;
            gotshit = 1;
          }
        }
      }
      if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH)) {
        if (sd.Get() && *sd.Get()) sd.Append(L" ");
        if (lgotthistime) {
          if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISDAY))
			sd.Append(L"on ");
          else {
            sd.Append(L"in ");
          }
		} else if (lgotthisday) sd.Append(L"of ");
        switch (te_cur_selected_month) {
          case 0: sd.Append(L"Jan"); gotshit = 1; gotmonth = 1; break;
          case 1: sd.Append(L"Feb"); gotshit = 1; gotmonth = 1; break;
          case 2: sd.Append(L"Mar"); gotshit = 1; gotmonth = 1; break;
          case 3: sd.Append(L"Apr"); gotshit = 1; gotmonth = 1; break;
          case 4: sd.Append(L"May"); gotshit = 1; gotmonth = 1; break;
          case 5: sd.Append(L"Jun"); gotshit = 1; gotmonth = 1; break;
          case 6: sd.Append(L"Jul"); gotshit = 1; gotmonth = 1; break;
          case 7: sd.Append(L"Aug"); gotshit = 1; gotmonth = 1; break;
          case 8: sd.Append(L"Sep"); gotshit = 1; gotmonth = 1; break;
          case 9: sd.Append(L"Oct"); gotshit = 1; gotmonth = 1; break;
          case 10: sd.Append(L"Nov"); gotshit = 1; gotmonth = 1; break;
          case 11: sd.Append(L"Dec"); gotshit = 1; gotmonth = 1; break;
          default: sd.Append(L"MONTH_OUTOFRANGE"); gotshit = 1; gotmonth = 1; break;
        }
      }
      if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISDAY) &&
          !IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH)) {
        if (sd.Get() && *sd.Get()) sd.Append(L" ");
        int v = GetDlgItemInt(hwndDlg, IDC_EDIT_DAY, NULL, TRUE);
		wsprintfW(qe_tempstr, L"%s%d", !gotmonth ? L"the " : L"", v);
        int u = (v - ((int)((double)v/10.0)) * 10);
        if (v < 1 || v > 31) {
          sd.Append(L"DAY_OUTOFRANGE");
          gotshit = 1;
        } else {
          sd.Append(qe_tempstr);
          switch (u) {
            case 1: sd.Append(L"st"); gotshit = 1; break;
            case 2: sd.Append(L"nd"); gotshit = 1; break;
            case 3: sd.Append(L"rd"); gotshit = 1; break;
            default: sd.Append(L"th"); gotshit = 1; break;
          }
        }
        if (!gotthismonth && IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISMONTH)) {
		  sd.Append(L" of this month");
        }
        gotthismonth = 1;
        gotshit = 1;
        gotday = 1;
      }
      if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISYEAR)) {
        
        if ((gotshit || gotday) && !gotmonth) {
          if (sd.Get() && *sd.Get()) sd.Append(L" ");
		  sd.Append(L"in ");
        } else if (gotmonth && gotday) sd.Append(L", ");
        else if (sd.Get() && *sd.Get()) sd.Append(L" ");
        int v = GetDlgItemInt(hwndDlg, IDC_EDIT_YEAR, NULL, TRUE);
        if (v <= 69) v += 2000;
        else if (v > 69 && v < 100) v += 1900;
        if (v <= 1969 || v >= 2038) 
          wcsncpy(qe_tempstr, L"YEAR_OUTOFRANGE", 4096);
        else
          wsprintfW(qe_tempstr, L"%d", v);
        sd.Append(qe_tempstr);
        gotshit = 1;
      }
      if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISTIME)) {
        if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_NOON)) {
          if (sd.Get() && *sd.Get()) sd.Append(L" ");
		  if (gotshit) sd.Append(L"at ");
		  sd.Append(L"noon");
        } else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_MIDNIGHT)) {
          if (sd.Get() && *sd.Get()) sd.Append(L" ");
		  if (gotshit) sd.Append(L"at ");
		  sd.Append(L"midnight");
        } else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_TIME)) {
          if (sd.Get() && *sd.Get()) sd.Append(L" ");
          if (gotshit) sd.Append(L"at ");
          SYSTEMTIME stt={0,0,0,0,0,0,0,0};
          DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER2), &stt);
          wsprintfW(qe_tempstr, L"%02d:%02d:%02d", stt.wHour, stt.wMinute, stt.wSecond);
          sd.Append(qe_tempstr);
          gotshit = 1;
        }
      }
    } else {
      if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_TIMEAGO))
		sd.Append(L" ago");
    }
    qe_curorigin.Set(sd.Get());
    if (sd.Get() && *sd.Get()) {
      str.Set(sd.Get());
      return str.Get();
    }
  }
  return NULL;
}

void qe_updateQuery(HWND hwndDlg) {
  if (!*qe_field) return;
	wchar_t qe_tempstr[4096] = {0};
  nde_field_t field = NDE_Table_GetColumnByName(g_table, AutoChar(qe_field));
  if (!field) return;
  int type =NDE_ColumnField_GetDataType(field);

  *qe_tempstr = NULL;
  GayStringW str;

  if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_NOT))
    str.Append(L"!(");
  str.Append(qe_field);
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_EQUAL))
    str.Append(L" ==");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ABOVE))
    str.Append(L" >");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_BELOW))
    str.Append(L" <");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ABOVEOREQUAL))
    str.Append(L" >=");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_BELOWOREQUAL))
    str.Append(L" <=");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ISLIKE))
    str.Append(L" like");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ISEMPTY))
    str.Append(L" isempty");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_BEGINS))
    str.Append(L" begins");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ENDS))
    str.Append(L" ends");
  if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_CONTAINS))
    str.Append(L" has");

  if (!IsDlgButtonChecked(hwndDlg, IDC_RADIO_ISEMPTY)) {
    switch(type) {
      case FIELD_DATETIME: {
        str.Append(L" [");
        GetDlgItemTextW(hwndDlg, IDC_EDIT_DATETIME, qe_tempstr, 4096);
        qe_curdate.Set(qe_tempstr);
        str.Append(qe_curdate.Get());
        str.Append(L"]");
      }
      break;
      case FIELD_INTEGER: {
        wsprintfW(qe_tempstr, L" %d", GetDlgItemInt(hwndDlg, IDC_EDIT_STRING, NULL, 1));
        str.Append(qe_tempstr);
      }
      break;
			case FIELD_FILENAME:
      case FIELD_STRING: {
        GetDlgItemTextW(hwndDlg, IDC_EDIT_STRING, qe_tempstr, 4096);
        str.Append(L" \"");
        GayStringW escaped;
        queryStrEscape(qe_tempstr, escaped);
        str.Append(escaped.Get());
        str.Append(L"\"");
      }
      break;
      case FIELD_LENGTH: {
        GetDlgItemTextW(hwndDlg, IDC_EDIT_STRING, qe_tempstr, 4096);
        wchar_t *z = wcschr(qe_tempstr, L':');
        if (z) {
          wchar_t *zz = wcschr(z+1, L':');
          int a, b, c=0,v=0;
          a = myatoi(qe_tempstr, z-qe_tempstr);
          if (zz) { b = myatoi(z+1, zz-(z+1)); c = _wtoi(zz+1); v = a * 3600 + b * 60 + c; }
          else { b = _wtoi(z+1); v = a * 60 + b; }

          if (v < 60)
            wsprintfW(qe_tempstr,L"%d",v);
          else if (v < 60*60)
            wsprintfW(qe_tempstr,L"%d:%02d",v/60,v%60);
          else
            wsprintfW(qe_tempstr,L"%d:%02d:%02d",v/60/60,(v/60)%60,v%60);

          str.Append(qe_tempstr);
        } else {
          wsprintfW(qe_tempstr, L" %d", GetDlgItemInt(hwndDlg, IDC_EDIT_STRING, NULL, 1));
          str.Append(qe_tempstr);
        }
      }
      break;
    }
  }

  if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_NOT))
    str.Append(L")");

  qe_curexpr.Set(str.Get());
  SetDlgItemTextW(hwndDlg, IDC_EDIT_RESULT, str.Get());
  SetDlgItemTextW(hwndDlg, IDC_EDIT_QUERY, qe_curquery.Get());

  qe_updateResultingDate(hwndDlg);
}

void qe_update(HWND hwndDlg) {
  qe_enableDisable(hwndDlg);
  qe_updateQuery(hwndDlg);
}

void te_updateResult(HWND hwndDlg) {
  const wchar_t *s = te_getDateTime(hwndDlg);
  te_result.Set(s == NULL ? L"":s);
  SetDlgItemTextW(hwndDlg, IDC_EDIT_RESULT, te_result.Get());
}

void te_update(HWND hwndDlg) {
  te_enableDisable(hwndDlg);
  te_updateResult(hwndDlg);
}

void qe_pushExpression(HWND hwndDlg, const wchar_t *op) {
  GayStringW newq;
  if (op && qe_curquery.Get() && *qe_curquery.Get()) {
    newq.Append(L"(");
    newq.Append(qe_curquery.Get());
    newq.Append(L") ");
    newq.Append(op);
    newq.Append(L" (");
    newq.Append(qe_curexpr.Get());
    newq.Append(L")");
    qe_curquery.Set(newq.Get());
  } else {
    qe_curquery.Set(qe_curexpr.Get());
  }
  qe_update(hwndDlg);
}

SYSTEMTIME pickedtime;
SYSTEMTIME pickeddate;

INT_PTR CALLBACK pickDateTimeDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
  switch(uMsg) {
    case WM_INITDIALOG: {
      return TRUE;
    }
    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case IDOK: 
          memset(&pickedtime, 0, sizeof(pickedtime));
          memset(&pickeddate, 0, sizeof(pickeddate));
          DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER), &pickeddate);
          DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER1), &pickedtime);
          EndDialog(hwndDlg, IDOK); 
          break;
        case IDCANCEL: 
          EndDialog(hwndDlg, IDCANCEL); 
          break;
      }
    }
    break;
  }
  return FALSE;
}

INT_PTR CALLBACK editDateTimeDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	
  switch(uMsg) {
    case WM_INITDIALOG: {
      const wchar_t *p = te_ret.Get();
      while (p) {
				if (*p != L' ') break;
        p++;
      }
      int empty = (p && *p == 0);
      TimeParse tp;
			NDE_Time_ApplyConversion(0, te_ret.Get(), &tp);

      if (!empty) {
        CheckDlgButton(hwndDlg, IDC_RADIO_THISYEAR, tp.relative_year == -1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_YEAR, tp.relative_year != -1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_THISMONTH, tp.relative_month == -1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_MONTH, tp.relative_month != -1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_THISDAY, tp.relative_day == -1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_DAY, tp.relative_day != -1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_THISTIME, tp.relative_hour == -1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_NOON, (tp.relative_hour == 12 && tp.relative_min == 0 && tp.relative_sec == 0) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_MIDNIGHT, (tp.relative_hour == 0 && tp.relative_min == 0 && tp.relative_sec == 0) ? BST_CHECKED : BST_UNCHECKED);
        int timeyet = IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISTIME);
        timeyet |= IsDlgButtonChecked(hwndDlg, IDC_RADIO_NOON);
        timeyet |= IsDlgButtonChecked(hwndDlg, IDC_RADIO_MIDNIGHT);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIME, !timeyet ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_CHECK_RELATIVE, tp.is_relative ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_CHECK_ABSOLUTE, !tp.is_relative ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_BEFORE, tp.offset_whence == 1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_AFTER, tp.offset_whence == 0 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_CHECK_TIMEAGO, tp.offset_used == 1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_Y, tp.offset_what == 0 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_M, tp.offset_what == 1 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_W, tp.offset_what == 2 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_D, tp.offset_what == 3 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_H, tp.offset_what == 4 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_MIN, tp.offset_what == 5 ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_S, tp.offset_what == 6 ? BST_CHECKED : BST_UNCHECKED);
        if (tp.is_relative && (tp.relative_year != -1 || tp.relative_month != -1 || tp.relative_day != -1 || tp.relative_hour != -1 || tp.relative_min != -1 || tp.relative_sec != -1))
          CheckDlgButton(hwndDlg, IDC_CHECK_SELECTIVE, BST_CHECKED);
        else
          CheckDlgButton(hwndDlg, IDC_CHECK_SELECTIVE, BST_UNCHECKED);
      }
      wchar_t t[64] = {0};
      wsprintfW(t, L"%d", tp.offset_value);
      SetDlgItemTextW(hwndDlg, IDC_EDIT_TIMEAGO, t);
      SetDlgItemTextW(hwndDlg, IDC_EDIT_RESULT, te_ret.Get());
      {
		wchar_t qe_tempstr[4096] = {0};
        time_t now;
        time(&now);
        struct tm *ts = localtime(&now);
        wsprintfW(qe_tempstr, L"%d", tp.relative_year != -1 ? tp.relative_year : ts->tm_year+1900);
        SetDlgItemTextW(hwndDlg, IDC_EDIT_YEAR, qe_tempstr);
        wsprintfW(qe_tempstr, L"%d", tp.relative_day != -1 ? tp.relative_day : ts->tm_mday);
        SetDlgItemTextW(hwndDlg, IDC_EDIT_DAY, qe_tempstr);
        te_cur_selected_month = tp.relative_month != -1 ? tp.relative_month : ts->tm_mon;
      }
      if (!empty && !IsDlgButtonChecked(hwndDlg, IDC_RADIO_THISTIME) && !IsDlgButtonChecked(hwndDlg, IDC_RADIO_NOON) && !IsDlgButtonChecked(hwndDlg, IDC_RADIO_MIDNIGHT)) {
        SYSTEMTIME st={0};
        st.wHour = (WORD)tp.relative_hour;
        st.wMinute = (WORD)tp.relative_min;
        st.wSecond = (WORD)tp.relative_sec;
        DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER2), GDT_VALID, &st);
      }
      if (!empty && !tp.is_relative) {
        time_t o = tp.absolute_datetime;
        struct tm *t = localtime(&o);
        if (t) {
          if (!tp.absolute_hasdate) 
            DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER), GDT_NONE, NULL);
          else {
            SYSTEMTIME st={0};
            st.wYear = (WORD)t->tm_year;
            st.wDay = (WORD)t->tm_mday;
            st.wMonth = (WORD)t->tm_mon;
            DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER), GDT_VALID, &st);
          }
          if (!tp.absolute_hastime) 
            DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER1), GDT_NONE, NULL);
          else {
            SYSTEMTIME st2={0};
            GetSystemTime(&st2);
            st2.wHour = (WORD)t->tm_hour;
            st2.wMinute = (WORD)t->tm_min;
            st2.wSecond = (WORD)t->tm_sec;
            DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER1), GDT_VALID, &st2);
          }
        }
        CheckDlgButton(hwndDlg, IDC_CHECK_TIMEAGO, 1);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_W, 1);
        SetDlgItemText(hwndDlg, IDC_EDIT_TIMEAGO, L"1");
        CheckDlgButton(hwndDlg, IDC_RADIO_BEFORE, 1);
      } else if (!empty) {
        if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_SELECTIVE)) {
          CheckDlgButton(hwndDlg, IDC_RADIO_THISYEAR, 1);
          CheckDlgButton(hwndDlg, IDC_RADIO_THISMONTH, 1);
          CheckDlgButton(hwndDlg, IDC_RADIO_THISDAY, 1);
          CheckDlgButton(hwndDlg, IDC_RADIO_THISTIME, 1);
        }
      } else if (empty) {
        CheckDlgButton(hwndDlg, IDC_CHECK_TIMEAGO, 1);
        CheckDlgButton(hwndDlg, IDC_RADIO_TIMEAGO_W, 1);
        SetDlgItemText(hwndDlg, IDC_EDIT_TIMEAGO, L"1");
        CheckDlgButton(hwndDlg, IDC_RADIO_BEFORE, 1);
        CheckDlgButton(hwndDlg, IDC_RADIO_THISYEAR, 1);
        CheckDlgButton(hwndDlg, IDC_RADIO_THISMONTH, 1);
        CheckDlgButton(hwndDlg, IDC_RADIO_THISDAY, 1);
        CheckDlgButton(hwndDlg, IDC_RADIO_THISTIME, 1);
        CheckDlgButton(hwndDlg, IDC_CHECK_RELATIVE, 1);
      }
      te_enableDisable(hwndDlg);
      SetTimer(hwndDlg, 0, 1000, NULL);
    }
    return 1;
    case WM_COMMAND: {
      switch (LOWORD(wParam)) {
        case IDOK: EndDialog(hwndDlg, IDOK); break;
        case IDCANCEL: EndDialog(hwndDlg, IDCANCEL); break;
        case IDC_EDIT_TIMEAGO:
        case IDC_EDIT_YEAR:
        case IDC_EDIT_DAY:
          if (HIWORD(wParam) != EN_CHANGE) break;
        case IDC_CHECK_ABSOLUTE:
        case IDC_CHECK_RELATIVE:
        case IDC_CHECK_TIMEAGO:
        case IDC_CHECK_SELECTIVE:
        case IDC_RADIO_THISYEAR:
        case IDC_RADIO_THISMONTH:
        case IDC_RADIO_THISDAY:
        case IDC_RADIO_THISTIME:
        case IDC_RADIO_YEAR:
        case IDC_RADIO_MONTH:
        case IDC_RADIO_DAY:
        case IDC_RADIO_TIME:
        case IDC_RADIO_NOON:
        case IDC_RADIO_MIDNIGHT:
        case IDC_RADIO_EQUAL:
        case IDC_RADIO_ABOVE:
        case IDC_RADIO_ABOVEOREQUAL:
        case IDC_RADIO_BELOW:
        case IDC_RADIO_BELOWOREQUAL:
        case IDC_RADIO_ISLIKE:
        case IDC_RADIO_ISEMPTY:
        case IDC_RADIO_BEGINS:
        case IDC_RADIO_ENDS:
        case IDC_RADIO_CONTAINS:
        case IDC_CHECK_NOT:
        case IDC_RADIO_TIMEAGO_Y:
        case IDC_RADIO_TIMEAGO_M:
        case IDC_RADIO_TIMEAGO_D:
        case IDC_RADIO_TIMEAGO_H:
        case IDC_RADIO_TIMEAGO_MIN:
        case IDC_RADIO_TIMEAGO_S:
        case IDC_RADIO_TIMEAGO_W:
        case IDC_RADIO_AFTER:
        case IDC_RADIO_BEFORE:
          te_update(hwndDlg);
          return 0;
        case IDC_BUTTON_PICK: {
           INT_PTR r = WASABI_API_DIALOGBOXW(IDD_EDIT_QUERY_PICK, hwndDlg, pickDateTimeDialogProc);
           if (r == IDOK) {
             if (pickeddate.wYear != 0) {
               SetDlgItemInt(hwndDlg, IDC_EDIT_YEAR, pickeddate.wYear, 0);
               te_cur_selected_month = pickeddate.wMonth-1;
               SetDlgItemInt(hwndDlg, IDC_EDIT_DAY, pickeddate.wDay, 0);
               CheckDlgButton(hwndDlg, IDC_RADIO_THISYEAR, BST_UNCHECKED);
               CheckDlgButton(hwndDlg, IDC_RADIO_YEAR, BST_CHECKED);
               CheckDlgButton(hwndDlg, IDC_RADIO_THISMONTH, BST_UNCHECKED);
               CheckDlgButton(hwndDlg, IDC_RADIO_MONTH, BST_CHECKED);
               CheckDlgButton(hwndDlg, IDC_RADIO_THISDAY, BST_UNCHECKED);
               CheckDlgButton(hwndDlg, IDC_RADIO_DAY, BST_CHECKED);
             }
             if (pickedtime.wYear != 0) {
               DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER2), GDT_VALID, &pickedtime);
               CheckDlgButton(hwndDlg, IDC_RADIO_THISTIME, BST_UNCHECKED);
               CheckDlgButton(hwndDlg, IDC_RADIO_TIME, BST_CHECKED);
             }
             te_update(hwndDlg);
           }
         }
         break;
        case IDC_BUTTON_NOW:
          CheckDlgButton(hwndDlg, IDC_RADIO_THISYEAR, BST_CHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_YEAR, BST_UNCHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_THISMONTH, BST_CHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_MONTH, BST_UNCHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_THISDAY, BST_CHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_DAY, BST_UNCHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_THISTIME, BST_CHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_TIME, BST_UNCHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_NOON, BST_UNCHECKED);
          CheckDlgButton(hwndDlg, IDC_RADIO_MIDNIGHT, BST_UNCHECKED);
          te_update(hwndDlg);
          return 0;
        case IDC_BUTTON_MONTH: {
          HMENU menu = CreatePopupMenu();
          for (int i=0;i<12;i++)
            AppendMenuW(menu, MF_STRING|(te_cur_selected_month==i)?MF_CHECKED:MF_UNCHECKED, i+1, monthtable[i]);
          POINT pt;
          GetCursorPos(&pt);
          int r = TrackPopupMenuEx(menu, TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, hwndDlg, NULL);
		  if (r)
		  {
			te_cur_selected_month = r-1;
			te_update(hwndDlg);
		  }
          break;
        }
      }
    }
    case WM_TIMER: {
      te_updateResultingDate(hwndDlg);
      return 0;
    }
    case WM_NOTIFY: {
      int idCtrl = (int)wParam; 
      NMHDR *hdr = (NMHDR*)lParam;
      switch (idCtrl) {
        case IDC_DATETIMEPICKER:
        case IDC_DATETIMEPICKER1:
        case IDC_DATETIMEPICKER2:
          if (hdr->code == DTN_DATETIMECHANGE)
            te_update(hwndDlg);
          break;
      }
    }
    break;
  }
  return 0;
}

INT_PTR CALLBACK editQueryDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            qe_fillFieldsList( GetDlgItem( hwndDlg, IDC_LIST_FIELDS ) );
            SetDlgItemTextW( hwndDlg, IDC_EDIT_RESULT, qe_curquery.Get() );
            CheckDlgButton( hwndDlg, IDC_RADIO_EQUAL, MF_CHECKED );
            qe_update( hwndDlg );
            SetTimer( hwndDlg, 0, 1000, NULL );
            return 1;
        }
        case WM_TIMER:
        {
            qe_updateResultingDate( hwndDlg );
            return 0;
        }
        case WM_COMMAND:
        {
            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                    if ( !qe_curquery.Get() || !*qe_curquery.Get() && qe_curexpr.Get() && *qe_curexpr.Get() )
                    {
                        wchar_t titleStr[ 32 ] = { 0 };
                        int r = MessageBoxW( hwndDlg, WASABI_API_LNGSTRINGW( IDS_QUERY_FIELD_IS_EMPTY ),
                                             WASABI_API_LNGSTRINGW_BUF( IDS_EMPTY_QUERY, titleStr, 32 ), MB_YESNOCANCEL );
                        if ( r == IDYES )
                            qe_curquery.Set( qe_curexpr.Get() );
                        else if ( r == IDCANCEL ) break;
                    }
                    if ( qe_origquery.Get() && *qe_origquery.Get() && qe_curquery.Get() || *qe_curquery.Get() && qe_curexpr.Get() && *qe_curexpr.Get() )
                    {
                        if ( !wcscmp( qe_curquery.Get(), qe_origquery.Get() ) )
                        {
                            wchar_t titleStr[ 32 ] = { 0 };
                            int r = MessageBoxW( hwndDlg, WASABI_API_LNGSTRINGW( IDS_NO_CHANGES_MADE_TO_QUERY ),
                                                 WASABI_API_LNGSTRINGW_BUF( IDS_QUERY_NOT_CHANGED, titleStr, 32 ), MB_YESNOCANCEL );
                            if ( r == IDYES )
                                qe_curquery.Set( qe_curexpr.Get() );
                            else if ( r == IDCANCEL ) break;
                        }
                    }
                    EndDialog( hwndDlg, IDOK );
                    break;
                case IDCANCEL: EndDialog( hwndDlg, IDCANCEL ); break;

                case IDC_EDIT_STRING:
                case IDC_EDIT_DATETIME:
                    if ( HIWORD( wParam ) != EN_CHANGE ) break;
                case IDC_RADIO_EQUAL:
                case IDC_RADIO_ABOVE:
                case IDC_RADIO_ABOVEOREQUAL:
                case IDC_RADIO_BELOW:
                case IDC_RADIO_BELOWOREQUAL:
                case IDC_RADIO_ISLIKE:
                case IDC_RADIO_ISEMPTY:
                case IDC_RADIO_BEGINS:
                case IDC_RADIO_ENDS:
                case IDC_RADIO_CONTAINS:
                case IDC_CHECK_NOT:
                    qe_update( hwndDlg );
                    return 0;
                case IDC_LIST_FIELDS:
                    if ( HIWORD( wParam ) == CBN_SELCHANGE )
                    {
                        HWND wnd = GetDlgItem( hwndDlg, IDC_LIST_FIELDS );
                        int idx = ListBox_GetCurSel( wnd );
                        int len = ListBox_GetTextLenW( wnd, idx );
                        if ( qe_field != NULL ) free( qe_field );
                        qe_field = (wchar_t *)calloc( ( len + 1 ), sizeof( wchar_t ) );
                        ListBox_GetTextW( wnd, idx, qe_field );

                        qe_field[ len ] = 0;
                        qe_update( hwndDlg );
                    }
                    break;
                case IDC_BUTTON_AND:
                    qe_pushExpression( hwndDlg, L"AND" );
                    break;
                case IDC_BUTTON_OR:
                    qe_pushExpression( hwndDlg, L"OR" );
                    break;
                case ID_BUTTON_SENDTOQUERY:
                    qe_pushExpression( hwndDlg, NULL );
                    break;
                case IDC_BUTTON_EDITDATETIME:
                {
                    const wchar_t *res = editTime( hwndDlg, qe_curdate.Get() );
                    if ( res != NULL )
                    {
                        SetDlgItemTextW( hwndDlg, IDC_EDIT_DATETIME, res );
                        qe_curdate.Set( res );
                    }
                    qe_update( hwndDlg );
                    break;
                }
                case IDC_EDIT_QUERY:
                    if ( HIWORD( wParam ) == EN_CHANGE )
                    {
                        wchar_t qe_tempstr[ 4096 ] = { 0 };
                        GetDlgItemTextW( hwndDlg, IDC_EDIT_QUERY, qe_tempstr, 4096 );
                        qe_curquery.Set( qe_tempstr );
                        qe_showHideOperators( hwndDlg );
                    }
                    break;
            }
        }
        break;
    }
    return FALSE;
}