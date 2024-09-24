#include "api.h"
#include "main.h"
#include "resource.h"
#include <commctrl.h>
#include "../../winamp/wa_ipc.h"
#include <strsafe.h>

static BOOL CALLBACK infoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK tabProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
static void OnSelChanged(HWND hwndDlg);

INFOBOX   *infobox_list = NULL;

int  config_info_x = 0, config_info_y = 0;
BOOL config_track = FALSE;

// =====================================================================================
void infobox_delete(HWND hwnd)
// =====================================================================================
// This function is called with the handle of the infobox to destroy.  It unloads the 
// module if appropriate (care must be take not to unload a module which is in use!).
{
	INFOBOX *cruise, *old;

	old = cruise = infobox_list;

	while(cruise)
	{
		if(cruise->hwnd == hwnd)
		{
			if(cruise == infobox_list)
				infobox_list = cruise->next;
			else old->next = cruise->next;

			// Destroy the info box window, then unload the module, *if*
			// the module is not actively playing!

			info_killseeker(hwnd);
			DestroyWindow(hwnd);

			if (cruise->dlg.module!=mf && cruise->dlg.ownModule)
				Unimod_Free(cruise->dlg.module);

			free(cruise->dlg.suse);
			free(cruise);
			return;
		}
		old    = cruise;
		cruise = cruise->next;
	}
}


// =====================================================================================
MPLAYER *get_player(UNIMOD *othermf)
// =====================================================================================
// Checks the current module against the one given.  if they match the MP is returned,
// else it returns NULL.
{
	if (mf == othermf)
		return mp;
	return NULL;
}


// =====================================================================================
static void infoTabInit(HWND hwndDlg, UNIMOD *m, DLGHDR *pHdr) 
// =====================================================================================
{ 
	DWORD   dwDlgBase = GetDialogBaseUnits(); 
	int     cxMargin = LOWORD(dwDlgBase) / 4,
		cyMargin = HIWORD(dwDlgBase) / 8;
	TC_ITEM tie;
	int     tabCounter;

	// Add a tab for each of the three child dialog boxes. 
	// and lock the resources for the child frames that appear within.

	tie.mask   = TCIF_TEXT | TCIF_IMAGE; 
	tie.iImage = -1; 
	tabCounter = 0;

	if(m->numsmp)
	{
		tie.pszText = WASABI_API_LNGSTRING(IDS_SAMPLES);
		TabCtrl_InsertItem(pHdr->hwndTab, tabCounter, &tie); 
		pHdr->apRes[tabCounter] = WASABI_API_CREATEDIALOGPARAM(IDD_SAMPLES, hwndDlg, tabProc, IDD_SAMPLES);
		SendMessage(mikmod.hMainWindow,WM_WA_IPC,(WPARAM)pHdr->apRes[tabCounter],IPC_USE_UXTHEME_FUNC);
		SetWindowPos(pHdr->apRes[tabCounter], HWND_TOP, pHdr->left+1, pHdr->top+15, 0, 0, SWP_NOSIZE);
		ShowWindow(pHdr->apRes[tabCounter++], SW_HIDE);
	}

	if(m->numins)
	{
		tie.pszText = WASABI_API_LNGSTRING(IDS_INSTRUMENTS);
		TabCtrl_InsertItem(pHdr->hwndTab, tabCounter, &tie);
		pHdr->apRes[tabCounter] = WASABI_API_CREATEDIALOGPARAM(IDD_INSTRUMENTS, hwndDlg, tabProc, IDD_INSTRUMENTS);
		SendMessage(mikmod.hMainWindow,WM_WA_IPC,(WPARAM)pHdr->apRes[tabCounter],IPC_USE_UXTHEME_FUNC);
		SetWindowPos(pHdr->apRes[tabCounter], HWND_TOP, pHdr->left+1, pHdr->top+15, 0, 0, SWP_NOSIZE);
		ShowWindow(pHdr->apRes[tabCounter++], SW_HIDE);
	}

	if(m->comment && m->comment[0])
	{
		tie.pszText = WASABI_API_LNGSTRING(IDS_COMMENT);
		TabCtrl_InsertItem(pHdr->hwndTab, tabCounter, &tie);
		pHdr->apRes[tabCounter] = WASABI_API_CREATEDIALOGPARAM(IDD_COMMENT, hwndDlg, tabProc, CEMENT_BOX);
		SendMessage(mikmod.hMainWindow,WM_WA_IPC,(WPARAM)pHdr->apRes[tabCounter],IPC_USE_UXTHEME_FUNC);
		SetWindowPos(pHdr->apRes[tabCounter], HWND_TOP, pHdr->left+1, pHdr->top+15, 0, 0, SWP_NOSIZE);
		ShowWindow(pHdr->apRes[tabCounter++], SW_HIDE);
	}

	// Simulate selection of the LAST item
	TabCtrl_SetCurSel(pHdr->hwndTab, tabCounter-1);
	OnSelChanged(hwndDlg);
} 


// =====================================================================================
static BOOL CALLBACK tabProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
// =====================================================================================
// This is the callback procedure used by each of the three forms under the
// tab control on the Module info dialog box (sample, instrument, comment
// info forms).
{
	switch (uMsg)
	{   case WM_INITDIALOG:
	{
		HWND   hwndLB;
		DLGHDR *pHdr = (DLGHDR *)GetWindowLong(GetParent(hwndDlg), GWL_USERDATA);
		UNIMOD *m = pHdr->module;
		char   sbuf[10280] = {0};

		switch(lParam)
		{   case IDD_SAMPLES:
		{
			uint  x;

			hwndLB = GetDlgItem(hwndDlg, IDC_SAMPLIST);
			for (x=0; x<m->numsmp; x++)
			{
				StringCbPrintfA(sbuf, sizeof(sbuf), "%02d: %s",x+1, m->samples[x].samplename ? m->samples[x].samplename : "");
				SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM) sbuf);
			}
			SendMessage(hwndLB, LB_SETCURSEL, 0, 0);
			tabProc(hwndDlg, WM_COMMAND, (WPARAM)((LBN_SELCHANGE << 16) + IDC_SAMPLIST), (LPARAM)hwndLB);
		}
		return TRUE;

		case IDD_INSTRUMENTS:
			{
				uint  x;

				hwndLB = GetDlgItem(hwndDlg, IDC_INSTLIST);
				for (x=0; x<m->numins; x++)
				{
					StringCbPrintfA(sbuf, sizeof(sbuf), "%02d: %s",x+1, m->instruments[x].insname ? m->instruments[x].insname : "");
					SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM) sbuf);
				}
				SendMessage(hwndLB, LB_SETCURSEL, 0, 0);
				tabProc(hwndDlg, WM_COMMAND, (WPARAM)((LBN_SELCHANGE << 16) + IDC_INSTLIST), (LPARAM)hwndLB);
			}
			return TRUE;

		case CEMENT_BOX:
			if(m->comment && m->comment[0])
			{   
				uint  x,i;

				hwndLB = GetDlgItem(hwndDlg, CEMENT_BOX);
				// convert all CRs to CR/LF pairs.  That's the way the edit box likes them!

				for(x=0, i=0; x<strlen(m->comment) && i < sizeof(sbuf)-1; x++)
				{	
					sbuf[i++] = m->comment[x];
					if(m->comment[x]==0x0d && m->comment[x+1]!=0x0a)
						sbuf[i++] = 0x0a;
				}
				sbuf[i] = 0;

				SetWindowText(hwndLB, sbuf);
			}
			return TRUE;
		}
	}
	break;

	case WM_COMMAND:
		if(HIWORD(wParam) == LBN_SELCHANGE)
		{	// Processes the events for the sample and instrument list boxes, namely updating
			// the samp/inst info upon a WM_COMMAND issuing a listbox selection change.

			int   moo = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
			char  sbuf[1024] = {0}, st1[128] = {0}, st2[64] = {0}, st3[64] = {0};
			char tmp1[32] = {0}, tmp2[32] = {0}, tmp3[32] = {0};
			DLGHDR *pHdr = (DLGHDR *)GetWindowLong(GetParent(hwndDlg), GWL_USERDATA);
			UNIMOD *m = pHdr->module;

			switch (LOWORD(wParam))
			{   case IDC_INSTLIST:
			{	INSTRUMENT *inst = &m->instruments[moo];
			uint        x;
			size_t         cnt;
			char *sbuf_p;

			// --------------------
			// Part 1: General instrument header info
			//   default volume, auto-vibrato, fadeout (in that order).

			{
				StringCbPrintfA(sbuf, sizeof(sbuf), "%d%%\n%s\n%s", (inst->globvol * 400) / 256,
						WASABI_API_LNGSTRING_BUF((inst->vibdepth ? IDS_YES : IDS_NO),tmp1,sizeof(tmp1)/sizeof(*tmp1)),
						WASABI_API_LNGSTRING_BUF((inst->volfade ? IDS_YES : IDS_NO),tmp2,sizeof(tmp2)/sizeof(*tmp2)));
				SetWindowText(GetDlgItem(hwndDlg, IDC_INSTHEAD), sbuf);
			}
			//(inst->nnatype == NNA_CONTINUE) ? "Continue" : (inst->nnatype == NNA_OFF) ? "Off" : (inst->nnatype == NNA_FADE) ? "Fade" : "Cut");

			// --------------------
			// Part 2: The instrument envelope info (vol/pan/pitch)

			// Wow this is ugly, but it works:  Make a set of strings that have the
			// '(loop / sustain)' string.  Tricky, cuz the '/' is only added if it
			// is needed of course.

			if(inst->volflg & (EF_LOOP | EF_SUSTAIN))
			{
				StringCbPrintfA(st1, sizeof(st1), "(%s%s%s)",
						(inst->volflg & EF_LOOP) ? WASABI_API_LNGSTRING(IDS_LOOP) : "",
						((inst->volflg & EF_LOOP) && (inst->volflg & EF_SUSTAIN)) ? " / " : "",
						(inst->volflg & EF_SUSTAIN) ? WASABI_API_LNGSTRING_BUF(IDS_SUSTAIN,tmp1,sizeof(tmp1)/sizeof(*tmp1)) : "");
			} else st1[0] = 0;

			if(inst->panflg & (EF_LOOP | EF_SUSTAIN))
			{

				StringCbPrintfA(st2, sizeof(st2), "(%s%s%s)",
						(inst->panflg & EF_LOOP) ? WASABI_API_LNGSTRING(IDS_LOOP) : "",
						((inst->panflg & EF_LOOP) && (inst->panflg & EF_SUSTAIN)) ? " / " : "",
						(inst->panflg & EF_SUSTAIN) ? WASABI_API_LNGSTRING_BUF(IDS_SUSTAIN,tmp1,sizeof(tmp1)/sizeof(*tmp1)) : "");
			} else st2[0] = 0;

			if(inst->pitflg & (EF_LOOP | EF_SUSTAIN))
			{
				StringCchPrintfA(st3,sizeof(st3), "(%s%s%s)",
							(inst->pitflg & EF_LOOP) ? WASABI_API_LNGSTRING(IDS_LOOP) : "",
							((inst->pitflg & EF_LOOP) && (inst->pitflg & EF_SUSTAIN)) ? " / " : "",
							(inst->pitflg & EF_SUSTAIN) ? WASABI_API_LNGSTRING_BUF(IDS_SUSTAIN,tmp1,sizeof(tmp1)/sizeof(*tmp1)) : "");
			} else st3[0] = 0;

			{
				StringCbPrintfA(sbuf, sizeof(sbuf), "%s %s\n%s %s\n%s %s",
						WASABI_API_LNGSTRING_BUF(((inst->volflg & EF_ON) ? IDS_ON : IDS_OFF),tmp1,sizeof(tmp1)/sizeof(*tmp1)),
						st1[0] ? st1 : "",
						WASABI_API_LNGSTRING_BUF(((inst->panflg & EF_ON) ? IDS_ON : IDS_OFF),tmp2,sizeof(tmp2)/sizeof(*tmp2)),
						st2[0] ? st2 : "",
						WASABI_API_LNGSTRING_BUF(((inst->pitflg & EF_ON) ? IDS_ON : IDS_OFF),tmp3,sizeof(tmp3)/sizeof(*tmp3)),
						st3[0] ? st3 : "");

			}
			SetWindowText(GetDlgItem(hwndDlg, IDC_INSTENV), sbuf);

			// --------------------
			// Part 3: List of samples used by this instrument!
			// the trick here is that that we have to figure out what samples are used from the
			// sample index table in inst->samplenumber.

			memset(pHdr->suse,0,m->numsmp*sizeof(BOOL));
			for(x=0; x<120; x++)
				if(inst->samplenumber[x] != 65535)
					pHdr->suse[inst->samplenumber[x]] = 1;

			sbuf[0] = 0;  cnt = sizeof(sbuf)/sizeof(*sbuf);
			sbuf_p = sbuf;
			for (x=0; x<m->numsmp; x++)
			{
				if(pHdr->suse[x])
				{
					StringCbPrintfExA(sbuf_p, cnt, &sbuf_p, &cnt, 0, "%02d: %s\r\n",x+1, m->samples[x].samplename);
				}
			}
			if (cnt < sizeof(sbuf)/sizeof(*sbuf))
			{
				sbuf[sizeof(sbuf)/sizeof(*sbuf) - cnt - 2] = 0;  // cut off the final CR/LF set
			}
			SetWindowText(GetDlgItem(hwndDlg, TB_SAMPLELIST), sbuf);

			}
			break;

			case IDC_SAMPLIST:
				{	UNISAMPLE *samp = &m->samples[moo];
				EXTSAMPLE *es = NULL;

				if(m->extsamples) es = &m->extsamples[moo];

				// Display sampe header info...
				// Length, Format, Quality, Looping, Auto-vibrato, Volume, Panning (in that order).

				{
					char yn[64] = {0}, pp[64] = {0};
					StringCbPrintfA(sbuf, sizeof(sbuf), "%d %s\n%d %s\n%s %s\n%s\n%s\n%d\n%d",
							samp->length * (samp->format&SF_16BITS ? 2 : 1), WASABI_API_LNGSTRING_BUF(IDS_BYTES, tmp1, sizeof(tmp1)/sizeof(*tmp1)),
							samp->speed, WASABI_API_LNGSTRING_BUF((m->flags&UF_XMPERIODS ? IDS_FINETUNE : (samp->format&SF_SIGNED ? IDS_HZ_SIGNED : IDS_HZ_UNSIGNED)),tmp2,sizeof(tmp2)/sizeof(*tmp2)),
							samp->format & SF_16BITS ? "16" : "8", WASABI_API_LNGSTRING_BUF(IDS_BITS, tmp3, sizeof(tmp3)/sizeof(*tmp3)),
							WASABI_API_LNGSTRING_BUF((samp->flags&SL_LOOP ? ( samp->flags&SL_BIDI ? IDS_PING_PONG : (samp->flags&SL_REVERSE ? IDS_REVERSE : IDS_FORWARD )) : samp->flags&SL_SUSTAIN_LOOP ? ( samp->flags&SL_SUSTAIN_BIDI ? IDS_SUSTAIN_PING_PONG : IDS_SUSTAIN ) : IDS_NONE),pp,sizeof(pp)/sizeof(*pp)),
							WASABI_API_LNGSTRING_BUF(((es && es->vibdepth) ? IDS_YES : IDS_NO),yn,sizeof(yn)/sizeof(*yn)),
							samp->volume, samp->panning);
				}

				SetWindowText(GetDlgItem(hwndDlg, IDC_SAMPINFO), sbuf);
				}
				break;
			}
		}
		break;
	}
	return 0;
}


// =====================================================================================
static void OnSelChanged(HWND hwndDlg) 
// =====================================================================================
{ 
	DLGHDR *pHdr = (DLGHDR *) GetWindowLong(hwndDlg, GWL_USERDATA); 
	int iSel = TabCtrl_GetCurSel(pHdr->hwndTab); 

	if(pHdr->hwndDisplay)  ShowWindow(pHdr->hwndDisplay,SW_HIDE);
	ShowWindow(pHdr->apRes[iSel],SW_SHOW);
	pHdr->hwndDisplay = pHdr->apRes[iSel];

	// Note to self: Despite their inhernet use in interfaces, coding tab controls
	// apparently REALLY sucks, and it should never ever be done again by myself
	// or anyone else whom I respect as a sane individual and I would like to have
	// remain that way.  As for me, it is too late.  Bruhahahaha!K!J!lkjgkljASBfkJBdglkn.

} 

// =====================================================================================
static void CALLBACK UpdateInfoRight(HWND hwnd, UINT uMsg, UINT ident, DWORD systime)
// =====================================================================================
{
	char        str[256] = {0};
	DLGHDR     *pHdr = (DLGHDR *)GetWindowLong(hwnd, GWL_USERDATA);
	MPLAYER    *mp;

	// Player info update .. BPM, sngspeed, position, row, voices.
	// Only update if our mf struct is the same as the one currently loaded into the player.

	if ((mp = get_player(pHdr->module)) == NULL)
	{
		// clean up
		if (pHdr->inUse)
		{
			UNIMOD *m = pHdr->module;
			pHdr->inUse = FALSE;

			StringCbPrintfA(str, sizeof(str), WASABI_API_LNGSTRING(IDS_X_X_X_OF_X_NOT_PLAYING),
					 m->inittempo, m->initspeed, m->numpos);
			SetDlgItemText(hwnd, IDC_INFORIGHT, str);

			if (pHdr->seeker)
			{
				if (pHdr->seeker != mp)
					Player_Free(pHdr->seeker);
				pHdr->seeker = NULL;
			}
		}

		// "track song" mode
		if (mf && IsDlgButtonChecked(hwnd, IDC_TRACK)==BST_CHECKED)
		{
			SendMessage(hwnd, WM_USER+10, 0, 0);
			infoDlg(GetParent(hwnd), mf, GetActiveWindow()==hwnd, FALSE);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		}
	}
	else
	{
		MPLAYER *seeker;
		long     acv;

		if (!pHdr->inUse)
		{
			assert(pHdr->seeker == NULL);
			pHdr->inUse = TRUE;

			// create our new player instance specifically for seeking
			if (!(config_playflag & CPLAYFLG_SEEKBYORDERS))
			{
				if ((pHdr->seeker = Player_Dup(mp)) == NULL)
					return;

				// steal statelist from the original player
				// (this will not require special handling,
				// because of a smart allocation system)

				pHdr->seeker->statelist  = mp->statelist;
				pHdr->seeker->statecount = mp->statecount;
			}
			else pHdr->seeker = NULL;
		}

		// Seek to our new song time, using a bastardized version of Player_SetPosTime code:
		if (pHdr->seeker)
		{
			long curtime = mikmod.GetOutputTime() * 64;
			seeker = pHdr->seeker;

			if (seeker->statelist)
			{
				int   t = 0;

				while (t<seeker->statecount &&
					seeker->statelist[t].curtime &&
					curtime>=seeker->statelist[t].curtime)
					t++;

				if (t)
					Player_Restore(seeker, t - 1);
				else Player_Cleaner(seeker);
			}
			else Player_Cleaner(seeker);

			while(!seeker->ended && seeker->state.curtime<curtime)
				Player_PreProcessRow(seeker, NULL);
		}
		else seeker = mp;

		// Display all the goodie info we have collected:
		// ---------------------------------------------

		acv = Mikmod_GetActiveVoices(mp->vs->md);
		if (acv > pHdr->maxv) pHdr->maxv = acv;

		StringCbPrintfA(str, sizeof(str), WASABI_API_LNGSTRING(IDS_X_X_X_OF_X_X_OF_X_X_OF_X),
				 seeker->state.bpm, seeker->state.sngspd, seeker->state.sngpos,
				 seeker->mf->numpos, seeker->mf->positions[seeker->state.sngpos],
				 seeker->state.patpos, seeker->state.numrow, acv, pHdr->maxv);
		SetWindowText(GetDlgItem(hwnd,IDC_INFORIGHT), str);
	}
}

// =====================================================================================
void infoDlg(HWND hwnd, UNIMOD *m, BOOL activate, BOOL primiary)
// =====================================================================================
{
	INFOBOX *box;
	HWND     dialog, hwndPrev;
	char     str[256] = {0};

	if (!m) return;

	//
	//  create local dataplace
	//

	box = (INFOBOX*)calloc(1, sizeof(INFOBOX));
	if (!box) return;

	box->dlg.left   = 7;
	box->dlg.top    = 168;
	box->dlg.module = m;
	box->dlg.ownModule = primiary;

	box->next       = infobox_list;
	infobox_list    = box;

	//
	//  create dialog
	//

	hwndPrev  = GetActiveWindow();
	box->hwnd = dialog = WASABI_API_CREATEDIALOG(IDD_ID3EDIT, hwnd, infoProc);
	box->dlg.hwndTab = GetDlgItem(dialog, IDC_TAB);

	SetWindowLong(dialog, GWL_USERDATA, (LONG)&box->dlg);
	SetDlgItemText(dialog, IDC_ID3_FN, m->filename);

	// IDC_INFOLEFT contains static module information:
	// File Size, Length (in mins), channels, samples, instruments.

	StringCbPrintfA(str, sizeof(str), WASABI_API_LNGSTRING(IDS_X_BTYES_X_OF_X_MINUTES),
			 m->filesize, m->songlen/60000,(m->songlen%60000)/1000, m->numchn, m->numsmp, m->numins);
	SetDlgItemText(dialog, IDC_INFOLEFT, str);
	SetDlgItemText(dialog, IDC_TITLE, m->songname);
	SetDlgItemText(dialog, IDC_TYPE,  m->modtype);

	// IDC_INFORIGHT - contains player information

	StringCbPrintfA(str, sizeof(str), WASABI_API_LNGSTRING(IDS_X_X_X_OF_X_NOT_PLAYING),
			 m->inittempo, m->initspeed, m->numpos);
	SetDlgItemText(dialog, IDC_INFORIGHT, str);

	// pHdr->suse is a samples-used block, allocated if this module uses
	// instruments, and used to display the sampels that each inst uses

	if (m->numins)
		box->dlg.suse = (BOOL*)calloc(m->numsmp, sizeof(BOOL));

	CheckDlgButton(dialog, IDC_TRACK, config_track ? BST_CHECKED : BST_UNCHECKED);
	infoTabInit(dialog, m, &box->dlg);
	SetTimer(dialog, 1, 50, UpdateInfoRight);

	ShowWindow(dialog, SW_SHOW);
	if (!activate) SetActiveWindow(hwndPrev);               // do not steal focus
}

// =====================================================================================
void info_killseeker(HWND hwnd)
// =====================================================================================
{
	DLGHDR *pHdr = (DLGHDR *)GetWindowLong(hwnd, GWL_USERDATA);

	if (pHdr->seeker)
	{
		assert(pHdr->inUse);

		if (pHdr->seeker != mp)
			Player_Free(pHdr->seeker);
		pHdr->seeker = NULL;
	}

	pHdr->inUse = FALSE;
}


// =====================================================================================
static BOOL CALLBACK infoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
// =====================================================================================
{
	UNIMOD   *m = NULL;

	switch (uMsg)
	{   
	case WM_INITDIALOG:
		{
			RECT rect, wrect;

			if (GetWindowRect(mikmod.hMainWindow, &wrect) && GetWindowRect(hwndDlg, &rect))
			{
				wrect.left += config_info_x;
				wrect.top  += config_info_y;

				if (wrect.left>=0 && wrect.top>=0 &&
					wrect.left<GetSystemMetrics(SM_CXFULLSCREEN)-16 &&
					wrect.top<GetSystemMetrics(SM_CYFULLSCREEN)-16)
					MoveWindow(hwndDlg, wrect.left, wrect.top, rect.right-rect.left, rect.bottom-rect.top, FALSE);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;
		case IDC_TRACK:
			config_track = IsDlgButtonChecked(hwndDlg, IDC_TRACK) == BST_CHECKED;
			break;
		}
		break;

	case WM_USER + 10:
	case WM_CLOSE:
		// save offset
		{
			RECT rect, wrect;

			if (GetWindowRect(mikmod.hMainWindow, &wrect) && GetWindowRect(hwndDlg, &rect))
			{
				config_info_x = rect.left - wrect.left;
				config_info_y = rect.top - wrect.top;
			}
		}
		config_track = IsDlgButtonChecked(hwndDlg, IDC_TRACK) == BST_CHECKED;

		// clean up
		if (uMsg != WM_CLOSE)
			break;

		KillTimer(hwndDlg, 1);
		infobox_delete(hwndDlg);
		config_write();
		break;

	case WM_NOTIFY:
		{
			NMHDR *notice = (NMHDR*)lParam;
			switch(notice->code)
			{
			case TCN_SELCHANGE:
				OnSelChanged(hwndDlg);
				break;
			}
		}
		return TRUE;
	}
	return 0;
}
