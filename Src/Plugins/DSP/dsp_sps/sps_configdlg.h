#ifndef SPS_CONFIGDLG_IMPL

BOOL CALLBACK SPS_configWindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

#else

#ifdef SPS_CONFIGDLG_HIDEABLE_EDITOR
static void showHideSliders(HWND hwndDlg, SPSEffectContext *ctx)
{
	int x;
	x=(SPS_CONFIGDLG_HIDEABLE_EDITOR || ctx->curpreset.slider_labels[0][0][0])?SW_SHOWNA:SW_HIDE;
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER1),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER1_LABEL1),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER1_LABEL2),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER1_LABEL3),x);
	x=(SPS_CONFIGDLG_HIDEABLE_EDITOR || ctx->curpreset.slider_labels[1][0][0])?SW_SHOWNA:SW_HIDE;
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER2),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER2_LABEL1),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER2_LABEL2),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER2_LABEL3),x);
	x=(SPS_CONFIGDLG_HIDEABLE_EDITOR || ctx->curpreset.slider_labels[2][0][0])?SW_SHOWNA:SW_HIDE;
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER3),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER3_LABEL1),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER3_LABEL2),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER3_LABEL3),x);
	x=(SPS_CONFIGDLG_HIDEABLE_EDITOR || ctx->curpreset.slider_labels[3][0][0])?SW_SHOWNA:SW_HIDE;
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER4),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER4_LABEL1),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER4_LABEL2),x);
	ShowWindow(GetDlgItem(hwndDlg,IDC_SLIDER4_LABEL3),x);
}

static void showHideEditor(HWND hwndDlg, int isInit)
{
	int en=0;
	static int lw;
	RECT r;
	GetWindowRect(hwndDlg,&r);
	if (isInit)
	{
		lw=r.right-r.left;
	}
	if (!SPS_CONFIGDLG_HIDEABLE_EDITOR)
	{
		SetDlgItemText(hwndDlg,IDC_EDIT,WASABI_API_LNGSTRING(IDS_SHOW_EDITOR));
		RECT r2;
		GetWindowRect(GetDlgItem(hwndDlg,IDC_SAVE),&r2);
		SetWindowPos(hwndDlg,NULL,0,0,r2.left-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
	}
	else 
	{
		if (!isInit)
		{
			SetWindowPos(hwndDlg,NULL,0,0,lw,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
		}
		SetDlgItemText(hwndDlg,IDC_EDIT,WASABI_API_LNGSTRING(IDS_HIDE_EDITOR));
		en=1;
	}

	UINT tab[] = {
		IDC_SLIDER1_LABEL1,
		IDC_SLIDER1_LABEL2,
		IDC_SLIDER1_LABEL3,
		IDC_SLIDER2_LABEL1,
		IDC_SLIDER2_LABEL2,
		IDC_SLIDER2_LABEL3,
		IDC_SLIDER3_LABEL1,
		IDC_SLIDER3_LABEL2,
		IDC_SLIDER3_LABEL3,
		IDC_SLIDER4_LABEL1,
		IDC_SLIDER4_LABEL2,
		IDC_SLIDER4_LABEL3,
	};

	for (int x = 0; x < sizeof(tab)/sizeof(UINT); x ++)
	{
		EnableWindow(GetDlgItem(hwndDlg,tab[x]),en);
    }

	UINT tab2[]={
		IDC_SAVE,
		IDC_SHOWHELP,
		IDC_INIT,
		IDC_ONSLIDERCHANGE,
		IDC_PERSAMPLE,
	};

	for (int x = 0; x < sizeof(tab2)/sizeof(UINT); x++)
	{
	    ShowWindow(GetDlgItem(hwndDlg,tab2[x]),en?SW_SHOWNA:SW_HIDE);
	}
}

#endif

char* BuildFilterString(void)
{
	static char filterStr[MAX_PATH] = {0};
	if(!filterStr[0])
	{
		char* temp = filterStr;
		//"SPS presets\0*.sps\0All files\0*.*\0"
		WASABI_API_LNGSTRING_BUF(IDS_SPS_PRESETS,filterStr,128);
		temp += lstrlen(filterStr)+1;
		lstrcpyn(temp, "*.sps", MAX_PATH);
		temp = temp + lstrlen(temp) + 1;
		lstrcpyn(temp, WASABI_API_LNGSTRING(IDS_ALL_FILES), 128);
		temp = temp + lstrlen(temp) + 1;
		lstrcpyn(temp, "*.*", MAX_PATH);
		*(temp = temp + lstrlen(temp) + 1) = 0;
	}
	return filterStr;
}

static void updatePresetText(HWND hwndDlg, SPSEffectContext *ctx)
{
	char *p=strrchr(ctx->curpreset_name,'\\');
	if (!p) p=ctx->curpreset_name;
	else p++;
	char *p2=strrchr(p,'.');
	if (p2) *p2=0;
	SetDlgItemText(hwndDlg,IDC_PRESET,p);
	if (p2) *p2='.';
}

static void dosavePreset(HWND hwndDlg, SPSEffectContext *ctx)
{
	char temp[2048] = {0};
	OPENFILENAME l={sizeof(l),0};
	char buf1[2048],buf2[2048];
	GetCurrentDirectory(sizeof(buf2),buf2);
	strcpy(buf1,g_path);
	l.hwndOwner = hwndDlg;
	l.lpstrFilter = BuildFilterString();
	l.lpstrFile = temp;
	strcpy(temp,ctx->curpreset_name);
	l.nMaxFile = 2048-1;
	l.lpstrTitle = WASABI_API_LNGSTRING(IDS_SAVE_PRESET);
	l.lpstrDefExt = "SPS";
	l.lpstrInitialDir = buf1;
	l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_OVERWRITEPROMPT; 	        
	if (GetSaveFileName(&l))
	{
		strcpy(ctx->curpreset_name,temp);
		SPS_save_preset(ctx,ctx->curpreset_name,"SPS PRESET");
		updatePresetText(hwndDlg,ctx);
	} 
	SetCurrentDirectory(buf2);
}

static void presetToDialog(HWND hwndDlg, SPSEffectContext *ctx)
{
	SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,1000-ctx->curpreset.sliderpos[0]);
	SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETPOS,1,1000-ctx->curpreset.sliderpos[1]);
	SendDlgItemMessage(hwndDlg,IDC_SLIDER3,TBM_SETPOS,1,1000-ctx->curpreset.sliderpos[2]);
	SendDlgItemMessage(hwndDlg,IDC_SLIDER4,TBM_SETPOS,1,1000-ctx->curpreset.sliderpos[3]);

	SetDlgItemText(hwndDlg,IDC_SLIDER1_LABEL1,ctx->curpreset.slider_labels[0][0]);
	SetDlgItemText(hwndDlg,IDC_SLIDER1_LABEL2,ctx->curpreset.slider_labels[0][1]);
	SetDlgItemText(hwndDlg,IDC_SLIDER1_LABEL3,ctx->curpreset.slider_labels[0][2]);
	SetDlgItemText(hwndDlg,IDC_SLIDER2_LABEL1,ctx->curpreset.slider_labels[1][0]);
	SetDlgItemText(hwndDlg,IDC_SLIDER2_LABEL2,ctx->curpreset.slider_labels[1][1]);
	SetDlgItemText(hwndDlg,IDC_SLIDER2_LABEL3,ctx->curpreset.slider_labels[1][2]);
	SetDlgItemText(hwndDlg,IDC_SLIDER3_LABEL1,ctx->curpreset.slider_labels[2][0]);
	SetDlgItemText(hwndDlg,IDC_SLIDER3_LABEL2,ctx->curpreset.slider_labels[2][1]);
	SetDlgItemText(hwndDlg,IDC_SLIDER3_LABEL3,ctx->curpreset.slider_labels[2][2]);
	SetDlgItemText(hwndDlg,IDC_SLIDER4_LABEL1,ctx->curpreset.slider_labels[3][0]);
	SetDlgItemText(hwndDlg,IDC_SLIDER4_LABEL2,ctx->curpreset.slider_labels[3][1]);
	SetDlgItemText(hwndDlg,IDC_SLIDER4_LABEL3,ctx->curpreset.slider_labels[3][2]);

	SetDlgItemText(hwndDlg,IDC_INIT,ctx->curpreset.code_text[0]);
	SetDlgItemText(hwndDlg,IDC_PERSAMPLE,ctx->curpreset.code_text[1]);
	SetDlgItemText(hwndDlg,IDC_ONSLIDERCHANGE,ctx->curpreset.code_text[2]);

	updatePresetText(hwndDlg,ctx);
#ifdef SPS_CONFIGDLG_HIDEABLE_EDITOR
	showHideSliders(hwndDlg,ctx);
#endif
}

static int m_help_lastpage=4;
static char *m_localtext;
static void _dosetsel(HWND hwndDlg)
{
	HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
	int sel=TabCtrl_GetCurSel(tabwnd);
	char *text="";

	m_help_lastpage=sel;

	text=SPSHELP_gethelptext(sel);

	if (!text && sel == 4 && m_localtext) text=m_localtext;

	SetDlgItemText(hwndDlg,IDC_EDIT1,text);
}

static BOOL CALLBACK evalHelpDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{	
			TCITEM item;
			HWND tabwnd=GetDlgItem(hwndDlg,IDC_TAB1);
			helpWnd=hwndDlg;
			item.mask=TCIF_TEXT;
			item.pszText=WASABI_API_LNGSTRING(IDS_GENERAL);
			TabCtrl_InsertItem(tabwnd,0,&item);
			item.pszText=WASABI_API_LNGSTRING(IDS_OPERATORS);
			TabCtrl_InsertItem(tabwnd,1,&item);
			item.pszText=WASABI_API_LNGSTRING(IDS_FUNCTIONS);
			TabCtrl_InsertItem(tabwnd,2,&item);
			item.pszText=WASABI_API_LNGSTRING(IDS_CONSTANTS);
			TabCtrl_InsertItem(tabwnd,3,&item);
			// fucko: context specific stuff
			m_localtext=0;
			if (lParam)
			{
				item.pszText=(char *)lParam;
				m_localtext=item.pszText + strlen(item.pszText)+1;
				TabCtrl_InsertItem(tabwnd,4,&item);
			}
			else if (m_help_lastpage > 3) m_help_lastpage=0;

			TabCtrl_SetCurSel(tabwnd,m_help_lastpage);
			_dosetsel(hwndDlg);
		}
		return 0;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwndDlg,1);
				helpWnd = 0;
			}
		return 0;

		case WM_NOTIFY:
		{
			LPNMHDR p=(LPNMHDR) lParam;
			if (p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE) _dosetsel(hwndDlg);
		}
		return 0;
	}
	return 0;
}

BOOL CALLBACK SPS_configWindowProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	SPSEffectContext *ctx;

	if ( uMsg == WM_INITDIALOG )
		SetWindowLong( hwndDlg, DWL_USER, lParam );

	ctx = (SPSEffectContext *) GetWindowLong( hwndDlg, DWL_USER );

	if ( ctx )
	{
		switch ( uMsg )
		{
			case WM_INITDIALOG:
				SetWindowText( hwndDlg, hdr.description );

				SendDlgItemMessage( hwndDlg, IDC_SLIDER1, TBM_SETTICFREQ, 100, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER2, TBM_SETTICFREQ, 100, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER3, TBM_SETTICFREQ, 100, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER4, TBM_SETTICFREQ, 100, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER1, TBM_SETRANGEMIN, 0, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER1, TBM_SETRANGEMAX, 0, 1000 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER2, TBM_SETRANGEMIN, 0, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER2, TBM_SETRANGEMAX, 0, 1000 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER3, TBM_SETRANGEMIN, 0, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER3, TBM_SETRANGEMAX, 0, 1000 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER4, TBM_SETRANGEMIN, 0, 0 );
				SendDlgItemMessage( hwndDlg, IDC_SLIDER4, TBM_SETRANGEMAX, 0, 1000 );

#ifdef SPS_CONFIGDLG_HIDEABLE_EDITOR
				showHideEditor( hwndDlg, TRUE );
				showHideSliders( hwndDlg, ctx );
#else
				ShowWindow( GetDlgItem( hwndDlg, IDC_EDIT ), SW_HIDE );
#endif
				if ( !ctx->bypass )
					CheckDlgButton( hwndDlg, IDC_BYPASS, BST_CHECKED );

				presetToDialog( hwndDlg, ctx );
				return 0;

			case WM_USER + 0x80:
				ShowWindow( hwndDlg, SW_SHOW );
				SetForegroundWindow( hwndDlg );
				return 0;

			case WM_COMMAND:
			{
				int w = 0;
				switch ( LOWORD( wParam ) )
				{
					case IDC_SLIDER1_LABEL1: w++;
					case IDC_SLIDER1_LABEL2: w++;
					case IDC_SLIDER1_LABEL3: w++;
					case IDC_SLIDER2_LABEL1: w++;
					case IDC_SLIDER2_LABEL2: w++;
					case IDC_SLIDER2_LABEL3: w++;
					case IDC_SLIDER3_LABEL1: w++;
					case IDC_SLIDER3_LABEL2: w++;
					case IDC_SLIDER3_LABEL3: w++;
					case IDC_SLIDER4_LABEL1: w++;
					case IDC_SLIDER4_LABEL2: w++;
					case IDC_SLIDER4_LABEL3: w++;
						if ( HIWORD( wParam ) == EN_CHANGE )
						{
							w = 12 - w;
							GetDlgItemText( hwndDlg, LOWORD( wParam ), ctx->curpreset.slider_labels[ w / 3 ][ w % 3 ], MAX_LABEL_LEN );
						}
						break;
#ifdef SPS_CONFIGDLG_HIDEABLE_EDITOR
					case IDC_EDIT:
						SPS_CONFIGDLG_HIDEABLE_EDITOR = !SPS_CONFIGDLG_HIDEABLE_EDITOR;
						showHideEditor( hwndDlg, FALSE );
						showHideSliders( hwndDlg, ctx );
						break;
#endif
					case IDC_BYPASS:
						ctx->bypass = !IsDlgButtonChecked( hwndDlg, IDC_BYPASS );
						break;

					case IDC_LOAD:
					{
						SendMessage( mod.hwndParent, WM_WA_IPC, 0, IPC_PUSH_DISABLE_EXIT );
						char temp[ 2048 ] = { 0 };
						OPENFILENAME l = { sizeof( l ),0 };
						char buf1[ 2048 ], buf2[ 2048 ];
						GetCurrentDirectory( sizeof( buf2 ), buf2 );
						strcpy( buf1, g_path );
						l.lpstrInitialDir = buf1;
						l.hwndOwner = hwndDlg;
						l.lpstrFilter = BuildFilterString();
						l.lpstrFile = temp;
						l.nMaxFile = 2048 - 1;
						l.lpstrTitle = WASABI_API_LNGSTRING( IDS_LOAD_PRESET );
						l.lpstrDefExt = "SPS";
						l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
						if ( GetOpenFileName( &l ) )
						{
							SPS_load_preset( ctx, temp, "SPS PRESET" );
							presetToDialog( hwndDlg, ctx );
						}
						SetCurrentDirectory( buf2 );
						SendMessage( mod.hwndParent, WM_WA_IPC, 0, IPC_POP_DISABLE_EXIT );
					}
					break;

					case IDC_NEW:
						char title[ 32 ];
						if ( MessageBox( hwndDlg, WASABI_API_LNGSTRING( IDS_CLEAR_CURRENT_SETTINGS ),
										 WASABI_API_LNGSTRING_BUF( IDS_CONFIRMATION, title, 32 ), MB_YESNO ) == IDYES )
						{
							EnterCriticalSection( &ctx->code_cs );
							memset( &ctx->curpreset, 0, sizeof( ctx->curpreset ) );
							ctx->code_needrecompile = 7;
							memset( &ctx->triggers, 0, sizeof( ctx->triggers ) );
							ctx->curpreset_name[ 0 ] = 0;
							LeaveCriticalSection( &ctx->code_cs );

							presetToDialog( hwndDlg, ctx );
						}
						break;

					case IDC_SAVE:
						SendMessage( mod.hwndParent, WM_WA_IPC, 0, IPC_PUSH_DISABLE_EXIT );
						dosavePreset( hwndDlg, ctx );
						SendMessage( mod.hwndParent, WM_WA_IPC, 0, IPC_POP_DISABLE_EXIT );
						break;

					case IDC_INIT:
						if ( HIWORD( wParam ) == EN_CHANGE )
						{
							KillTimer( hwndDlg, 100 );
							SetTimer( hwndDlg, 100, 250, NULL );
						}
						break;

					case IDC_PERSAMPLE:
						if ( HIWORD( wParam ) == EN_CHANGE )
						{
							KillTimer( hwndDlg, 101 );
							SetTimer( hwndDlg, 101, 250, NULL );
						}
						break;

					case IDC_ONSLIDERCHANGE:
						if ( HIWORD( wParam ) == EN_CHANGE )
						{
							KillTimer( hwndDlg, 102 );
							SetTimer( hwndDlg, 102, 250, NULL );
						}
						break;

					case IDC_SHOWHELP:
						WASABI_API_DIALOGBOX( IDD_EVAL_HELP, hwndDlg, evalHelpDlgProc );
						break;

					case IDC_TRIGGER1:
						ctx->triggers[ 0 ]++;
						break;

					case IDC_TRIGGER2:
						ctx->triggers[ 1 ]++;
						break;

					case IDC_TRIGGER3:
						ctx->triggers[ 2 ]++;
						break;

					case IDC_TRIGGER4:
						ctx->triggers[ 3 ]++;
						break;
				}

				return 0;
			}

			case WM_TIMER:
				if ( wParam == 100 || wParam == 101 || wParam == 102 )
				{
					KillTimer( hwndDlg, wParam );
					EnterCriticalSection( &ctx->code_cs );
					GetDlgItemText( hwndDlg, wParam == 100 ? IDC_INIT : ( wParam == 101 ? IDC_PERSAMPLE : IDC_ONSLIDERCHANGE ), ctx->curpreset.code_text[ wParam - 100 ], MAX_CODE_LEN );
					ctx->code_needrecompile |= 1 << ( wParam - 100 );
					LeaveCriticalSection( &ctx->code_cs );
				}
				return 0;

			case WM_CLOSE:
#ifdef SPS_CONFIGDLG_ON_WM_CLOSE
				SPS_CONFIGDLG_ON_WM_CLOSE
#endif
					return 0;

			case WM_VSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage( swnd, TBM_GETPOS, 0, 0 );
				if ( swnd == GetDlgItem( hwndDlg, IDC_SLIDER1 ) )
				{
					ctx->curpreset.sliderpos[ 0 ] = 1000 - t;
					ctx->sliderchange = 1;
				}

				if ( swnd == GetDlgItem( hwndDlg, IDC_SLIDER2 ) )
				{
					ctx->curpreset.sliderpos[ 1 ] = 1000 - t;
					ctx->sliderchange = 1;
				}

				if ( swnd == GetDlgItem( hwndDlg, IDC_SLIDER3 ) )
				{
					ctx->curpreset.sliderpos[ 2 ] = 1000 - t;
					ctx->sliderchange = 1;
				}

				if ( swnd == GetDlgItem( hwndDlg, IDC_SLIDER4 ) )
				{
					ctx->curpreset.sliderpos[ 3 ] = 1000 - t;
					ctx->sliderchange = 1;
				}
			}
			break;
		}
		return 0;
	}
}

#endif