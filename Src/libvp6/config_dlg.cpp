/****************************************************************************
*
*   Module Title :     vfw_config_dlg.c
*
*   Description  :     Configuration Parameters dialog module.
*
****************************************************************************/						

/****************************************************************************
*  Header Files
****************************************************************************/
#include <windows.h>
#include <stdio.h>  
#include <commctrl.h>
#include "type_aliases.h"
#include "vp60_comp_interface.h"
#include "resource.h"		// Must be the version resident in the VCAP dll directory!!!
#include "vpvfwver.h"
#include "vp6vfw.h"
#include "vp60_comp_interface.h"
#include <commdlg.h> 
//#include <cderr.h>
extern HINSTANCE hInstance;

void BuildVersionInfo(char *modname,char *VersionInfo,int *vers)
{

	// ************************************************************
	// The next bit of code reads version information from resource
	VersionInfo[0]=0;
    char  szFullPath[256]; 
    DWORD dwVerHnd; 
    DWORD dwVerInfoSize; 
    GetModuleFileName(/*GetModuleHandle(modname)*/hInstance, szFullPath, sizeof(szFullPath)); 
    dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd); 

    if (dwVerInfoSize) 
    { 
        // If we were able to get the information, process it: 
        HANDLE  hMem; 
        LPVOID  lpvMem; 
 
        hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize); 
        lpvMem = GlobalLock(hMem); 
        GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem); 

        UINT  cchVer = 0; 
        LPSTR lszVer = NULL; 
        strcat(VersionInfo, "\r");
		VerQueryValue(lpvMem,TEXT("\\StringFileInfo\\040904E4\\FileDescription"), (void **) &lszVer, &cchVer);  	
		strcat(VersionInfo,lszVer);
        strcat(VersionInfo, "\r\r");
		VerQueryValue(lpvMem,TEXT("\\StringFileInfo\\040904E4\\LegalCopyright"), (void **) &lszVer, &cchVer);  	
		strcat(VersionInfo,lszVer);
        strcat(VersionInfo, "\r");
        strcat(VersionInfo, " Version ");
		VerQueryValue(lpvMem,TEXT("\\StringFileInfo\\040904E4\\ProductVersion"), (void **) &lszVer, &cchVer);  	
		strcat(VersionInfo,lszVer);

		GlobalUnlock(hMem); 
        GlobalFree(hMem); 
	}	
}

BOOL FAR PASCAL Advanced_ParamsDlgProc( HWND hWndDlg,UINT Message,WPARAM wParam,LPARAM lParam );
BOOL FAR PASCAL General_ParamsDlgProc( HWND hWndDlg,UINT Message,WPARAM wParam,LPARAM lParam );
BOOL FAR PASCAL Settings_ParamsDlgProc( HWND hWndDlg,UINT Message,WPARAM wParam,LPARAM lParam );
BOOL FAR PASCAL Main_ParamsDlgProc(   HWND   hWndDlg,
                                        UINT   Message,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    COMP_CONFIG_VP6 *compConfig = (COMP_CONFIG_VP6 *)GetWindowLong(hWndDlg,GWL_USERDATA);

    switch(Message)
    {      
    case WM_NOTIFY:
        {
            switch(wParam)
            {
            case IDC_TAB1:
                {
                    NMHDR *msg = (NMHDR *) lParam;

                    switch(msg->code)
                    {
                    case TCN_SELCHANGE:
                        {
                            HWND hwndTab = GetDlgItem(hWndDlg, IDC_TAB1);
                            int whichTab = TabCtrl_GetCurSel(hwndTab);
                            switch(whichTab)
                            {
                            case 2:
                                {
                                    if(compConfig->PlaceHolder)
                                        DestroyWindow((HWND) compConfig->PlaceHolder);
                                    compConfig->PlaceHolder = (INT32) CreateDialogParam(hInstance,"SETTINGS",hWndDlg,Settings_ParamsDlgProc ,(LPARAM) compConfig);
                                    ShowWindow((HWND) compConfig->PlaceHolder,SW_SHOW);
                                    return TRUE;
                                }
                            case 0:
                                {
                                    if(compConfig->PlaceHolder)
                                        DestroyWindow((HWND) compConfig->PlaceHolder);
                                    compConfig->PlaceHolder = (INT32) CreateDialogParam(hInstance,"GENERAL",hWndDlg,General_ParamsDlgProc ,(LPARAM) compConfig);
                                    ShowWindow((HWND) compConfig->PlaceHolder,SW_SHOW);
                                    return TRUE;
                                }
                            case 1:
                                {

                                    if(compConfig->PlaceHolder)
                                        DestroyWindow((HWND) compConfig->PlaceHolder);
                                    compConfig->PlaceHolder = (INT32) CreateDialogParam(hInstance,"ADVANCED",hWndDlg,Advanced_ParamsDlgProc ,(LPARAM) compConfig);
                                    ShowWindow((HWND) compConfig->PlaceHolder,SW_SHOW);
                                    return TRUE;
                                }
                            }

                            break;
                        }
                    }
                    break;
                }
            }
            return (FALSE);
        }
    case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC) wParam;
            HWND hwndStatic = (HWND) lParam;
            
            if ( hwndStatic == GetDlgItem ( hWndDlg, IDC_TITLE ) || 
                 hwndStatic == GetDlgItem ( hWndDlg, IDC_FRAME ) )
            {
                return (LRESULT) GetStockObject(WHITE_BRUSH); ;
            }
        }
        break;
    case WM_INITDIALOG:
        {

            HWND hwndTab = GetDlgItem(hWndDlg, IDC_TAB1);
            TC_ITEM tie;
            
            SetWindowLong(hWndDlg, GWL_USERDATA, (unsigned long)lParam);  
            compConfig = (COMP_CONFIG_VP6 *) lParam;

            tie.mask = TCIF_TEXT | TCIF_STATE | TCIF_IMAGE;
            tie.iImage = -1;
            
            tie.pszText = "General";
            if (TabCtrl_InsertItem(hwndTab, 0, &tie) == -1) 
                return NULL;
            
            tie.pszText = "Advanced";
            if (TabCtrl_InsertItem(hwndTab, 1, &tie) == -1) 
                return NULL;

            tie.pszText = "Settings";
            if (TabCtrl_InsertItem(hwndTab, 2, &tie) == -1) 
                return NULL;

            char VersionString[2048]={0};
            int vers;
            BuildVersionInfo("VP6VFW.DLL",VersionString,&vers);
            SetDlgItemText( hWndDlg, IDC_TITLE, VersionString);


            compConfig->PlaceHolder = (INT32) CreateDialogParam(hInstance,"GENERAL",hWndDlg,General_ParamsDlgProc ,(LPARAM) compConfig);
            ShowWindow((HWND) compConfig->PlaceHolder,SW_SHOW);

            
            return (TRUE);
        }
        
        
    case WM_CLOSE:          /* Close the dialog. */
        /* Closing the Dialog behaves the same as Cancel    */
        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
        return (TRUE);
        
    case WM_COMMAND:       /* A control has been activated. */
        switch(LOWORD(wParam))
        {
            /* OK leaves the current settings in force */
        case IDOK:
            EndDialog(hWndDlg, IDOK);
            break; 
            
        case IDCANCEL:
            EndDialog(hWndDlg, IDCANCEL);
            break;
            
        default:
            return (FALSE);
            
        }
        return (FALSE);
        
    default:
        return (FALSE);
            
    } /* End of Main Dialog case statement. */

    return FALSE;
}     /* End of WndProc */



BOOL FAR PASCAL General_ParamsDlgProc(   HWND   hWndDlg,
                                        UINT   Message,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    COMP_CONFIG_VP6 *compConfig = (COMP_CONFIG_VP6 *)GetWindowLong(hWndDlg,GWL_USERDATA);

    switch(Message)
    {      
    case WM_SETFOCUS:
        Message+=0;
        break;
    case WM_INITDIALOG:
        {
            SetWindowLong(hWndDlg, GWL_USERDATA, (unsigned long)lParam);  
            compConfig = (COMP_CONFIG_VP6 *) lParam;
            
            // fill mode box
            SendDlgItemMessage(hWndDlg, IDD_MODE, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) "Realtime / Live Encoding"); 
            SendDlgItemMessage(hWndDlg, IDD_MODE, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) "Good Quality Fast Encoding"); 
            SendDlgItemMessage(hWndDlg, IDD_MODE, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) "One Pass - Best Quality"); 
            SendDlgItemMessage(hWndDlg, IDD_MODE, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) "Two Pass - First Pass" ); 
            SendDlgItemMessage(hWndDlg, IDD_MODE, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) "Two Pass - Second Pass - Good Quality"); 
            SendDlgItemMessage(hWndDlg, IDD_MODE, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) "Two Pass - Second Pass - Best Quality"); 

            // set mode
            SendDlgItemMessage(hWndDlg, IDD_MODE, CB_SETCURSEL, (LPARAM) compConfig->Mode,0); 

            // set end usage
	        switch(compConfig->EndUsage)
	        {
            case 0:
                CheckDlgButton( hWndDlg, IDC_ENDUSAGE1, 1);
                CheckDlgButton( hWndDlg, IDC_ENDUSAGE2, 0);
                break;
	        default:
                CheckDlgButton( hWndDlg, IDC_ENDUSAGE2, 1);
                CheckDlgButton( hWndDlg, IDC_ENDUSAGE1, 0);
                break;
	        }

            // set material 
	        switch(compConfig->Interlaced)
	        {
	        case 1:
                CheckDlgButton( hWndDlg, IDC_MATERIAL1, 1);
                CheckDlgButton( hWndDlg, IDC_MATERIAL2, 0);
                break;
            default:
                CheckDlgButton( hWndDlg, IDC_MATERIAL2, 1);
                CheckDlgButton( hWndDlg, IDC_MATERIAL1, 0);
                break;
	        }

            // setup noise reduction slider
            SendDlgItemMessage(hWndDlg, IDC_NOISEREDUCTION_SLIDER, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 6));  

            // set noise reduction
            SetDlgItemInt( hWndDlg, IDC_NOISEREDUCTION, compConfig->NoiseSensitivity, FALSE );
            SendDlgItemMessage(hWndDlg, IDC_NOISEREDUCTION_SLIDER, TBM_SETPOS, (WPARAM) TRUE,(LPARAM) compConfig->NoiseSensitivity); 
            SendDlgItemMessage(hWndDlg, IDC_NOISEREDUCTION_SLIDER, TBM_SETTICFREQ, (WPARAM) 1,(LPARAM) 1); 

            // set auto keyframe 
            CheckDlgButton( hWndDlg, IDC_AUTOKEYFRAME_CHECK, (compConfig->AutoKeyFrameEnabled) ? 1 : 0 );

            // set max frames btw keys
            SetDlgItemInt( hWndDlg, IDC_MAXFRAMESBTWKEYS, compConfig->ForceKeyFrameEvery, FALSE );

            return (TRUE);
        }
    case WM_HSCROLL:
        {
            // change edit box to match slider
            SetDlgItemInt( hWndDlg, IDC_NOISEREDUCTION, SendDlgItemMessage(hWndDlg, IDC_NOISEREDUCTION_SLIDER, TBM_GETPOS, 0,0), FALSE );
            return (TRUE);
        }
        
    case WM_DESTROY: //case WM_CLOSE:          /* Close the dialog. */
        
        // save everything to our structure
        compConfig->AutoKeyFrameEnabled = SendDlgItemMessage(hWndDlg, IDC_AUTOKEYFRAME_CHECK, BM_GETCHECK, 0, 0);
        compConfig->EndUsage = (END_USAGE) SendDlgItemMessage(hWndDlg, IDC_ENDUSAGE2, BM_GETCHECK, 0, 0);
        compConfig->Interlaced = SendDlgItemMessage(hWndDlg, IDC_MATERIAL1, BM_GETCHECK, 0, 0);
        compConfig->ForceKeyFrameEvery = GetDlgItemInt(hWndDlg, IDC_MAXFRAMESBTWKEYS, NULL, FALSE );
        compConfig->Mode = (MODE) SendDlgItemMessage(hWndDlg, IDD_MODE, CB_GETCURSEL, 0, 0); 
        compConfig->NoiseSensitivity = GetDlgItemInt(hWndDlg, IDC_NOISEREDUCTION, NULL, FALSE );
        return (TRUE);
        
    case WM_COMMAND:       /* A control has been activated. */
        {
            switch(HIWORD(wParam))
            {
            case EN_KILLFOCUS:
                {
                    // error checking
                    int  value = GetDlgItemInt(hWndDlg, LOWORD(wParam), NULL, FALSE );

                    switch(LOWORD(wParam))
                    {
                    case IDC_NOISEREDUCTION:
                        {
                            if(value < 0) value = 0;
                            if(value > 6) value = 6;

                            SendDlgItemMessage(hWndDlg, IDC_NOISEREDUCTION_SLIDER, TBM_SETPOS, (WPARAM) TRUE, value  );
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                        break;
                    case IDC_MAXFRAMESBTWKEYS:
                        {
                            if(value < 0) value = 0;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    default:
                        return (FALSE);
                    }
            
                }
            }
            return (FALSE);
        }
        

    default:
        return (FALSE);
            
    } /* End of Main Dialog case statement. */

    return FALSE;
}     /* End of WndProc */




BOOL FAR PASCAL Advanced_ParamsDlgProc(   HWND   hWndDlg,
                                        UINT   Message,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    COMP_CONFIG_VP6 *compConfig = (COMP_CONFIG_VP6 *)GetWindowLong(hWndDlg,GWL_USERDATA);
    switch(Message)
    {      
    case WM_INITDIALOG:
        {
            
            SetWindowLong(hWndDlg, GWL_USERDATA, (unsigned long)lParam);  
            compConfig = (COMP_CONFIG_VP6 *) lParam;

            if ( compConfig->EndUsage == 1) 
            {
                EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_MAXBITRATE), 0);
                EnableWindow(GetDlgItem(hWndDlg, STREAMING_PARAMETERS), 0);
                EnableWindow(GetDlgItem(hWndDlg, PEAK_BITRATE), 0);
                EnableWindow(GetDlgItem(hWndDlg, PREBUFFER), 0);
                EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_PREBUFFER), 0);
                EnableWindow(GetDlgItem(hWndDlg, OPTIMAL_BUFFER), 0);
                EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_OPTIMAL), 0);
                EnableWindow(GetDlgItem(hWndDlg, MAX_BUFFER), 0);
                EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_MAXBUFFER), 0);
            }
            if(compConfig->Mode < 4)
            {
                EnableWindow(GetDlgItem(hWndDlg, TWO_PASS_SECTION_DATARATE), 0);
                EnableWindow(GetDlgItem(hWndDlg, VARIABILITY), 0);
                EnableWindow(GetDlgItem(hWndDlg, IDC_DATARATEVARIABILITY), 0);
                EnableWindow(GetDlgItem(hWndDlg, MIN_SECTION), 0);
                EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_MINBANDWIDTH), 0);
                EnableWindow(GetDlgItem(hWndDlg, MAX_SECTION), 0);
                EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_MAXBANDWIDTH), 0);
            }

            // set buffer stats
            SetDlgItemInt( hWndDlg, IDC_EDIT_PREBUFFER, compConfig->StartingBufferLevel, FALSE );
            SetDlgItemInt( hWndDlg, IDC_EDIT_OPTIMAL, compConfig->OptimalBufferLevel, FALSE );
            SetDlgItemInt( hWndDlg, IDC_EDIT_MAXBUFFER, compConfig->MaximumBufferSize, FALSE );

            // setup vbr slider
            SendDlgItemMessage(hWndDlg, IDC_DATARATEVARIABILITY_SLIDER, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));  
            SendDlgItemMessage(hWndDlg, IDC_DATARATEVARIABILITY_SLIDER, TBM_SETTICFREQ, (WPARAM) 10,(LPARAM) 10); 

            // set vbr settings
            SendDlgItemMessage(hWndDlg, IDC_DATARATEVARIABILITY_SLIDER, TBM_SETPOS, (WPARAM) TRUE,(LPARAM) compConfig->TwoPassVBRBias); 
            SetDlgItemInt( hWndDlg, IDC_DATARATEVARIABILITY, compConfig->TwoPassVBRBias, FALSE );
            SetDlgItemInt( hWndDlg, IDC_EDIT_MINBANDWIDTH, compConfig->TwoPassVBRMinSection, FALSE );
            SetDlgItemInt( hWndDlg, IDC_EDIT_MAXBANDWIDTH, compConfig->TwoPassVBRMaxSection, FALSE );
            SetDlgItemInt( hWndDlg, IDC_EDIT_MAXBITRATE, compConfig->MaxAllowedDatarate, FALSE );



            // datarate control options
            SetDlgItemInt( hWndDlg, IDC_EDIT_UNDERSHOOT, compConfig->UnderShootPct, FALSE );

            // set adjust quantizer control 
            CheckDlgButton( hWndDlg, IDC_ADJUSTQ_CHECK, (compConfig->FixedQ) ? 0 : 1 );
            SetDlgItemInt( hWndDlg, IDC_EDIT_MINQUALITY, compConfig->BestAllowedQ, FALSE );
            SetDlgItemInt( hWndDlg, IDC_EDIT_MAXQUALITY, compConfig->Quality, FALSE );

            // allowing dropped frames 
            CheckDlgButton( hWndDlg, IDC_ALLOW_DROPPED_FRAMES_CHECK, (compConfig->AllowDF) ? 1 : 0 );
            SetDlgItemInt( hWndDlg, IDC_EDIT_TEMPORAL_DOWN, compConfig->DropFramesWaterMark, FALSE );

            // allowing spatial resampling
            CheckDlgButton( hWndDlg, IDC_SCALE_CHECK, (compConfig->AllowSpatialResampling) ? 1 : 0 );
            SetDlgItemInt( hWndDlg, IDC_EDIT_SPATIAL_DOWN, compConfig->ResampleDownWaterMark, FALSE );
            SetDlgItemInt( hWndDlg, IDC_EDIT_SPATIAL_UP, compConfig->ResampleUpWaterMark, FALSE );
            
            
            return (TRUE);
        }
        
        
    case WM_DESTROY: //case WM_CLOSE:          /* Close the dialog. */
        
        // save everything to our structure
        compConfig->StartingBufferLevel = GetDlgItemInt(hWndDlg, IDC_EDIT_PREBUFFER, NULL, FALSE );
        compConfig->OptimalBufferLevel = GetDlgItemInt(hWndDlg, IDC_EDIT_OPTIMAL, NULL, FALSE );
        compConfig->MaximumBufferSize = GetDlgItemInt(hWndDlg, IDC_EDIT_MAXBUFFER, NULL, FALSE );
        compConfig->TwoPassVBRBias = GetDlgItemInt(hWndDlg, IDC_DATARATEVARIABILITY, NULL, FALSE );
        compConfig->TwoPassVBRMinSection = GetDlgItemInt(hWndDlg, IDC_EDIT_MINBANDWIDTH, NULL, FALSE );
        compConfig->TwoPassVBRMaxSection = GetDlgItemInt(hWndDlg, IDC_EDIT_MAXBANDWIDTH, NULL, FALSE );
        compConfig->MaxAllowedDatarate = GetDlgItemInt(hWndDlg, IDC_EDIT_MAXBITRATE, NULL, FALSE );
        compConfig->UnderShootPct = GetDlgItemInt(hWndDlg, IDC_EDIT_UNDERSHOOT, NULL, FALSE );
        compConfig->FixedQ = !SendDlgItemMessage(hWndDlg, IDC_ADJUSTQ_CHECK, BM_GETCHECK, 0, 0);
        compConfig->BestAllowedQ = GetDlgItemInt(hWndDlg, IDC_EDIT_MINQUALITY, NULL, FALSE );
        compConfig->Quality = GetDlgItemInt(hWndDlg, IDC_EDIT_MAXQUALITY, NULL, FALSE );
        compConfig->AllowDF = SendDlgItemMessage(hWndDlg, IDC_ALLOW_DROPPED_FRAMES_CHECK, BM_GETCHECK, 0, 0);
        compConfig->DropFramesWaterMark = GetDlgItemInt(hWndDlg, IDC_EDIT_TEMPORAL_DOWN, NULL, FALSE );
        compConfig->AllowSpatialResampling = SendDlgItemMessage(hWndDlg, IDC_SCALE_CHECK, BM_GETCHECK, 0, 0);
        compConfig->ResampleDownWaterMark = GetDlgItemInt(hWndDlg, IDC_EDIT_SPATIAL_DOWN, NULL, FALSE );
        compConfig->ResampleUpWaterMark = GetDlgItemInt(hWndDlg, IDC_EDIT_SPATIAL_UP, NULL, FALSE );

        return (TRUE);

    case WM_HSCROLL:
        {
            // change edit box to match slider
            SetDlgItemInt( hWndDlg, IDC_DATARATEVARIABILITY, SendDlgItemMessage(hWndDlg, IDC_DATARATEVARIABILITY_SLIDER, TBM_GETPOS, 0,0), FALSE );
            return (TRUE);
        }
        
    case WM_COMMAND:       /* A control has been activated. */
        {
            switch(HIWORD(wParam))
            {
            case EN_KILLFOCUS:
                {
                    // error checking
                    int  value = GetDlgItemInt(hWndDlg, LOWORD(wParam), NULL, FALSE );

                    switch(LOWORD(wParam))
                    {
                    case IDC_DATARATEVARIABILITY:
                        {
                            if(value < 0) value = 0;
                            if(value > 100 ) value = 100;
                            SendDlgItemMessage(hWndDlg, IDC_DATARATEVARIABILITY_SLIDER, TBM_SETPOS, (WPARAM) TRUE, value  );
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }

        // save everything to our structure
                    case IDC_EDIT_PREBUFFER:
                        {
                            if(value < 0) value = 0;
                            if(value > 30 ) value = 30;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    case IDC_EDIT_OPTIMAL:
                        {
                            if(value < 0) value = 0;
                            if(value > 30 ) value = 30;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    case IDC_EDIT_MAXBUFFER:
                        {
                            if(value < 0) value = 0;
                            if(value > 30 ) value = 30;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }

                    case IDC_EDIT_MINBANDWIDTH:
                        {
                            if(value < 0) value = 0;
                            if(value > 100 ) value = 100;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    case IDC_EDIT_MAXBANDWIDTH:
                        {
                            if(value < 100) value = 100;
                            if(value > 1000 ) value = 1000;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    case IDC_EDIT_UNDERSHOOT:
                        {
                            if(value < 50) value = 50;
                            if(value > 100 ) value = 100;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }

                    case IDC_EDIT_MINQUALITY:
                        {
                            if(value < 0) value = 0;
                            if(value > 63 ) value = 63;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    case IDC_EDIT_MAXQUALITY:
                        {
                            if(value < 0) value = 0;
                            if(value > 63 ) value = 63;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }

                    case IDC_EDIT_TEMPORAL_DOWN :
                        {
                            if(value < 0) value = 0;
                            if(value > 100 ) value = 100;
                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    case IDC_EDIT_SPATIAL_DOWN :
                        {
                            if(value < 0) value = 0;
                            if(value > 100 ) value = 100;

                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }
                    case IDC_EDIT_SPATIAL_UP :
                        {
                            if(value < 0) value = 0;
                            if(value > 100 ) value = 100;

                            SetDlgItemInt( hWndDlg, LOWORD(wParam), value, FALSE );
                            break;
                        }

                    }
                }
             default:
                return (FALSE);
            }
            
            return (FALSE);
        }
        
    default:
        return (FALSE);
            
    } /* End of Main Dialog case statement. */

    return FALSE;
}     /* End of WndProc */



BOOL FAR PASCAL Settings_ParamsDlgProc(   HWND   hWndDlg,
                                        UINT   Message,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    COMP_CONFIG_VP6 *compConfig = (COMP_CONFIG_VP6 *)GetWindowLong(hWndDlg,GWL_USERDATA);

    switch(Message)
    {      
    case WM_INITDIALOG:
        {
            SetWindowLong(hWndDlg, GWL_USERDATA, (unsigned long)lParam);  
            compConfig = (COMP_CONFIG_VP6 *) lParam;

            if(!memcmp(compConfig->SettingsFile,compConfig->RootDirectory,strlen(compConfig->RootDirectory)))
            {
                strcpy(compConfig->SettingsFile,compConfig->SettingsFile+strlen(compConfig->RootDirectory));
            }
            if(!memcmp(compConfig->SettingsFile+strlen(compConfig->SettingsFile)-4,".vps",4))
            {
                compConfig->SettingsFile[strlen(compConfig->SettingsFile)-4]=0;
            }
            SetDlgItemText(hWndDlg,IDC_FIRSTPASS,compConfig->FirstPassFile);
            SetDlgItemText(hWndDlg,IDC_SETTINGSFILE,compConfig->SettingsFile);

        	WIN32_FIND_DATA wfd;

            char FileFilter[512];
            strncpy(FileFilter,compConfig->RootDirectory,512);
            strcat(FileFilter,"\\*.vps");
	        HANDLE ffh = FindFirstFile(FileFilter,&wfd);
	        if( ffh !=INVALID_HANDLE_VALUE)
	        {
		        do
		        {
			        wfd.cFileName[strlen(wfd.cFileName)-4]=0;
                    SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_ADDSTRING, 0, (LPARAM) (LPCSTR) wfd.cFileName); 
		        }
		        while ( FindNextFile(ffh,&wfd));
		        FindClose(ffh);
        	}

                        
            return (TRUE);
        }
        
    case WM_DESTROY: //case WM_CLOSE:          /* Close the dialog. */
        
        // save everything to our structure
        GetDlgItemText(hWndDlg,IDC_FIRSTPASS,compConfig->FirstPassFile,512);
        GetDlgItemText(hWndDlg,IDC_SETTINGSFILE,compConfig->SettingsFile,512);
        if(compConfig->SettingsFile[1] != ':' && compConfig->SettingsFile[1] != '\\')
        {
            char tmp[512];
            strncpy(tmp,compConfig->SettingsFile,512);
            strncpy(compConfig->SettingsFile,compConfig->RootDirectory,512);
            strcat(compConfig->SettingsFile,tmp);
        }
        if(compConfig->SettingsFile[strlen(compConfig->SettingsFile)-4] != '.' )
        {
            strcat(compConfig->SettingsFile,".vps");
        }

        return (TRUE);
        
    case WM_COMMAND:       /* A control has been activated. */
        {
            switch(HIWORD(wParam))
            {
            case LBN_SELCHANGE :
                {
                    int curSel =SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_GETCURSEL, 0, 0); 
                    SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_GETTEXT, curSel, (DWORD) compConfig->SettingsFile);
                    SetDlgItemText(hWndDlg,IDC_SETTINGSFILE,compConfig->SettingsFile);
                    if(compConfig->SettingsFile[1] != ':' && compConfig->SettingsFile[1] != '\\')
                    {
                        char tmp[512];
                        strncpy(tmp,compConfig->SettingsFile,512);
                        strncpy(compConfig->SettingsFile,compConfig->RootDirectory,512);
                        strcat(compConfig->SettingsFile,"\\");
                        strcat(compConfig->SettingsFile,tmp);
                    }
                    if(compConfig->SettingsFile[strlen(compConfig->SettingsFile)-4] != '.' )
                    {
                        strcat(compConfig->SettingsFile,".vps");
                    }
                    FILE *f = fopen(compConfig->SettingsFile,"rb");
                    if(f)
                    {
                        char tmp[512];
                        HWND still = (HWND) compConfig->PlaceHolder;
                        strncpy(tmp,compConfig->RootDirectory,512);
                        fread(compConfig,sizeof COMP_CONFIG_VP6,1,f);
                        strncpy(compConfig->RootDirectory,tmp,512);
                        fclose(f);
                        compConfig->PlaceHolder = (INT32) still;
                    }
                }

            case BN_CLICKED:
                switch(LOWORD(wParam))
                {
                    case ID_SAVE:
                        {
                            FILE *f;

                            GetDlgItemText(hWndDlg,IDC_SETTINGSFILE,compConfig->SettingsFile,512);
                            GetDlgItemText(hWndDlg,IDC_FIRSTPASS,compConfig->FirstPassFile,512);
            
                            if(compConfig->SettingsFile[1] != ':' && compConfig->SettingsFile[1] != '\\')
                            {
                                char tmp[512];
                                strncpy(tmp,compConfig->SettingsFile,512);
                                strncpy(compConfig->SettingsFile,compConfig->RootDirectory,512);
                                strcat(compConfig->SettingsFile,"\\");
                                strcat(compConfig->SettingsFile,tmp);
                            }
                            if(compConfig->SettingsFile[strlen(compConfig->SettingsFile)-4] != '.' )
                            {
                                strcat(compConfig->SettingsFile,".vps");
                            }

                            f=fopen(compConfig->SettingsFile,"wb");
                            if(f)
                            {
                                fwrite(compConfig,10+sizeof COMP_CONFIG_VP6,1,f);
                                fclose(f);
                            }
                            int curSel = SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_FINDSTRING, 0, (LPARAM) (LPCSTR) compConfig->SettingsFile ); 
                            if( curSel == LB_ERR) 
                            {
       	                        WIN32_FIND_DATA wfd;

                                int curSel = SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_RESETCONTENT, 0, 0 ); 
                                char FileFilter[512];
                                strncpy(FileFilter,compConfig->RootDirectory,512);
                                strcat(FileFilter,"\\*.vps");
	                            HANDLE ffh = FindFirstFile(FileFilter,&wfd);
	                            if( ffh !=INVALID_HANDLE_VALUE)
	                            {
		                            do
		                            {
			                            wfd.cFileName[strlen(wfd.cFileName)-4]=0;
                                        SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_ADDSTRING, 0, (LPARAM) (LPCSTR) wfd.cFileName); 
		                            }
		                            while ( FindNextFile(ffh,&wfd));
		                            FindClose(ffh);
        	                    }
                            }
                             

                            return TRUE;
                        }; 
                    case ID_DELETE:
                        {
                            int curSel =SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_GETCURSEL, 0, 0); 
                            SendDlgItemMessage(hWndDlg, IDC_SETTINGS_LIST, LB_DELETESTRING, curSel, 0); 


                            DeleteFile(compConfig->SettingsFile);

                            return TRUE;
                        };
                    case IDC_LOAD_FIRSTPASS:
                        {
                            static TCHAR szFilterLoad[] = TEXT("VP First Pass \0*.vpf\0\0"); 
                            OPENFILENAME ofn; 
                            TCHAR szTitle[64]; 
                            TCHAR szT[256];  
                            ofn.lStructSize= sizeof(ofn); 
                            ofn.hInstance= 0; 
                            ofn.lpstrFilter= szFilterLoad; 
                            ofn.lpstrCustomFilter= NULL; 
                            ofn.nMaxCustFilter= 0; 
                            ofn.nFilterIndex= 0; 
                            ofn.lpstrFileTitle= szTitle; 
                            ofn.nMaxFileTitle= sizeof(szTitle); 
                            ofn.lpstrInitialDir= compConfig->RootDirectory; 
                            ofn.lpstrTitle= NULL; 
                            ofn.nFileOffset= 0; 
                            ofn.nFileExtension= 0; 
                            ofn.lpstrDefExt= "vps"; 
                            ofn.lCustData= 0L; 
                            ofn.lpfnHook= NULL; 
                            ofn.lpTemplateName= NULL; 
                            ofn.hwndOwner= hWndDlg; 
                            ofn.lpstrFile= szT; 
                            ofn.nMaxFile= sizeof(szT); 
                            ofn.Flags= 0; 
                            szTitle[0] = TEXT('\0'); 
                            szT[0] = TEXT('\0'); 
                            if(GetOpenFileName(&ofn))
                            {
                                strncpy(compConfig->FirstPassFile,ofn.lpstrFile,512);
                                SetDlgItemText(hWndDlg,IDC_FIRSTPASS,compConfig->FirstPassFile);
                            }
                            

                            return TRUE;
                        }
                    case IDC_LOAD_SETTINGS:
                        {
                            static TCHAR szFilterLoad[] = TEXT("VP Setting Files\0*.vps\0\0"); 
                            OPENFILENAME ofn; 
                            TCHAR szTitle[64]; 
                            TCHAR szT[256];  
                            ofn.lStructSize= sizeof(ofn); 
                            ofn.hInstance= 0; 
                            ofn.lpstrFilter= szFilterLoad; 
                            ofn.lpstrCustomFilter= NULL; 
                            ofn.nMaxCustFilter= 0; 
                            ofn.nFilterIndex= 0; 
                            ofn.lpstrFileTitle= szTitle; 
                            ofn.nMaxFileTitle= sizeof(szTitle); 
                            ofn.lpstrInitialDir= compConfig->RootDirectory; 
                            ofn.lpstrTitle= NULL; 
                            ofn.nFileOffset= 0; 
                            ofn.nFileExtension= 0; 
                            ofn.lpstrDefExt= "vps"; 
                            ofn.lCustData= 0L; 
                            ofn.lpfnHook= NULL; 
                            ofn.lpTemplateName= NULL; 
                            ofn.hwndOwner= hWndDlg; 
                            ofn.lpstrFile= szT; 
                            ofn.nMaxFile= sizeof(szT); 
                            ofn.Flags= 0; 
                            szTitle[0] = TEXT('\0'); 
                            szT[0] = TEXT('\0'); 
                            if(GetOpenFileName(&ofn))
                            {
                                
                                strncpy(compConfig->SettingsFile,ofn.lpstrFile,512);

                                FILE *f = fopen(compConfig->SettingsFile,"rb");
                                if(f)
                                {
                                    HWND still = (HWND) compConfig->PlaceHolder;
                                    fread(compConfig,sizeof COMP_CONFIG_VP6,1,f);
                                    fclose(f);
                                    compConfig->PlaceHolder = (INT32) still;
                                }
                                if(!memcmp(compConfig->SettingsFile,compConfig->RootDirectory,strlen(compConfig->RootDirectory)))
                                {
                                    strcpy(compConfig->SettingsFile,compConfig->SettingsFile+strlen(compConfig->RootDirectory));
                                }
                                if(!memcmp(compConfig->SettingsFile+strlen(compConfig->SettingsFile)-4,".vps",4))
                                {
                                    compConfig->SettingsFile[strlen(compConfig->SettingsFile)-4]=0;
                                }
                                SetDlgItemText(hWndDlg,IDC_SETTINGSFILE,compConfig->SettingsFile);
                            }
                            

                            return TRUE;
                        }
                }
                break;
                default:
                    break;
            }

        }


    default:
        return (FALSE);
            
    } /* End of Main Dialog case statement. */

    return FALSE;
}     /* End of WndProc */
