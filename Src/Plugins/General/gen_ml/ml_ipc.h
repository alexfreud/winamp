#ifndef _WA_ML_IPC_H_
#define _WA_ML_IPC_H_

#include <windows.h>
#include <stddef.h>

//# IPC_LIBRARY_SENDTOMENU 
/*
  Basically for the sendto menu, do this:
    librarySendToMenuStruct s={0,};
    int IPC_LIBRARY_SENDTOMENU=SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)&"LibrarySendToMenu",IPC_REGISTER_WINAMP_IPCMESSAGE);
    if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)0,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
    {
      s.mode=1;
      s.hwnd=myWnd;
      s.data_type=mydatatype;
      s.build_hMenu=mysubmenu;
    }
    TrackPopupMenu();
    if (ret)
    {
      if (unrecognized ret)
      {
        if (s.mode == 2)
        {
					s.menu_id=ret;
          if (SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
          {
            // build my data.
            s.mode=3;
            s.data=my data;
            SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)
            // free my data
          }
        }
      }
    }
    if (s.mode) 
    {
      s.mode=4;
      SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU) // cleanup
    }
     
    ...

     WM_INITMENUPOPUP:
       if (wParam && (HMENU)wParam == s.build_hMenu && s.mode==1)
       {
         if (SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
           s.mode=2;
       }

  kinda complex and gross, yes?


  */
typedef struct { // always init this to all 0s

  int mode; // mode can be 0, to see if sendto is available. If sendto is available, we'll return 0xffffffff.
            // mode = 1 means we are building the menu. data_type should be set. on success, this will return 0xffffffff. 
            // mode = 2 means we are querying if our menu_id is handled by the sendto menu. returns 0xffffffff if it was.
            // mode = 3 means we are sending the data. return value is not important =)
            //        be sure to have set data_type, menu_id, and data, for this one.
            // mode = 4 means to cleanup this structure.

  // build parms
  HMENU build_hMenu; // set this to the HMENU
  int build_start_id; // override the start and endpoints of IDs it can use
  int build_end_id;   // or leave as 0s for defaults.

  // type used for build and send modes (ML_TYPE_*)
  int data_type;

  int menu_id;

  void *data;   // only used in mode = 3

  HWND hwnd; // parent for sendto

  // specify ctx[1]=1 to disable 'enqueue in winamp' on the menu
  // specify ctx[2]=(ML_TYPE_*)+1 as the originally intended data_type
  intptr_t ctx[32]; // internal winamp use
} librarySendToMenuStruct;

//IPC_GETMLWINDOW
//int IPC_GETMLWINDOW=SendMessage(hMainWindow,WM_WA_IPC,(WPARAM)&"LibraryGetWnd",IPC_REGISTER_WINAMP_IPCMESSAGE);
//
//then:
// to ensure library's db is loaded: if (IPC_GETMLWINDOW>65536) SendMessage(hMainWindow,WM_WA_IPC,-1,IPC_GETMLWINDOW);
// or to get the HWND of the library (for library APIs): HWND hwndlib = IPC_GETMLWINDOW>65536 ? SendMessage(hMainWindow,WM_WA_IPC,0,IPC_GETMLWINDOW) : 0;

#endif//_WA_ML_IPC_H_
