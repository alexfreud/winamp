#include "Main.h"

/*
static HDDEDATA CALLBACK GroupDDECallback (UINT uiType, UINT uiFmt, HANDLE hConv,
      HSZ sz1, HSZ sz2, HDDEDATA hData, LONG lData1, LONG lData2) {
   return ((HDDEDATA) NULL);
}
static BOOL CreateGroup(HWND hwnd);
static BOOL AddProgramItems (HWND hwnd, LPSTR szDummy);

static CONVCONTEXT   CCFilter = { sizeof (CONVCONTEXT), 0, 0, 0, 0L, 0L };
static LONG lIdInst;
static LONG lIdInst2;
*/

//CreateShortCut(hwnd,"D:\\WINNT\\Profiles\\Administrator\\Application Data\\Microsoft\\Internet Explorer\\Quick Launch\\Winamp.lnk","C:\\winamp\\winamp.exe");
//CreateShortCut(hwnd,"D:\\WINNT\\Profiles\\Administrator\\Desktop\\Winamp.lnk","C:\\winamp\\winamp.exe");


#if 0
void dde_adddesktop(HWND hwnd)
{
	HKEY mp3Key;
	char name[MAX_PATH]="";
	if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",&mp3Key) == ERROR_SUCCESS) 
	{
		int s=sizeof(name);
		if (RegQueryValueEx(mp3Key,"Desktop",0,NULL,name,&s) == ERROR_SUCCESS)
		{
		}
		RegCloseKey(mp3Key);
	}
	if (lstrlen(name))
	{
		char exe[MAX_PATH];
		GetModuleFileName(NULL,exe,sizeof(exe));
		lstrcat(name,"\\WINAMP.LNK");
		CreateShortCut(hwnd,name,exe,NULL,0);
	}
}


void dde_addquicklaunch(HWND hwnd)
{
	HKEY mp3Key;
	char name[MAX_PATH]="";
	if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",&mp3Key) == ERROR_SUCCESS) 
	{
		int s=sizeof(name);
		if (RegQueryValueEx(mp3Key,"AppData",0,NULL,name,&s) == ERROR_SUCCESS)
		{
		}
		RegCloseKey(mp3Key);
	}
	if (lstrlen(name))
	{
		char exe[MAX_PATH];
		GetModuleFileName(NULL,exe,sizeof(exe));
		lstrcat(name,"\\Microsoft\\Internet Explorer\\Quick Launch\\WINAMP.LNK");
		CreateShortCut(hwnd,name,exe,NULL,0);
	}
}

int dde_isquicklaunchavailable(void)
{
	HKEY mp3Key;
	char name[MAX_PATH]="";
	if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",&mp3Key) == ERROR_SUCCESS) 
	{
		int s=sizeof(name);
		if (RegQueryValueEx(mp3Key,"AppData",0,NULL,name,&s) == ERROR_SUCCESS)
		{
		}
		RegCloseKey(mp3Key);
	}
	if (lstrlen(name))
	{
		HANDLE h;
		WIN32_FIND_DATA fd;
		lstrcat(name,"\\Microsoft\\Internet Explorer\\Quick Launch");
		h=FindFirstFile(name,&fd);
		if (h != INVALID_HANDLE_VALUE)
		{
			FindClose(h);
			if (fd.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY ) return 1;

		} 
	} 
	return 0;
}


#if 1
int dde_addstart(HWND hwnd)
{
	HKEY mp3Key;
	char startMenu[MAX_PATH]="";
	if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",&mp3Key) == ERROR_SUCCESS) 
	{
		int s=sizeof(startMenu)/sizeof(startMenu[0]);
		if (RegQueryValueEx(mp3Key, "Programs", 0, NULL, startMenu, &s) == ERROR_SUCCESS)
		{
      char exe[MAX_PATH], shortcut[MAX_PATH], target[MAX_PATH];
	    GetModuleFileName(NULL,exe,sizeof(exe));

			// create Winamp folder
			PathAppend(startMenu, "Winamp");
      CreateDirectory(startMenu, NULL);

			// winamp.exe
			PathCombine(shortcut, startMenu, "Winamp.lnk");
	    CreateShortCut(hwnd, shortcut, exe, NULL, 0);

			// we don't need the executable filename anymore, but we need its path
			PathRemoveFileSpec(exe);
			
			// whatsnew.txt
			PathCombine(target, exe, "whatsnew.txt");
			PathCombine(shortcut, startMenu, "What's new.lnk");
	    CreateShortCut(hwnd, startMenu, target, NULL, 0);
			
			// uninstwa.exe
			PathCombine(shortcut, startMenu, "Uninstall Winamp.lnk");
      PathAppend(target, exe, "uninstwa.exe");
			CreateShortCut(hwnd, shortcut, target, NULL, 0);
		}
		RegCloseKey(mp3Key);
	}
  return 0;
}


#else

int dde_addstart(HWND hwnd)
{
  CreateGroup(hwnd);
  AddProgramItems (hwnd, NULL);
  return 0;
}

void dde_delstart(void)
{
   HDDEDATA   hData;
   LPSTR      szText;
   LPSTR      szCommand;
   HCONV      hConv;
   HSZ        szProgMan;
   LONG       lResult;
   szText = VirtualAlloc (NULL, 64, MEM_COMMIT, PAGE_READWRITE);
   szCommand = VirtualAlloc (NULL, 128, MEM_COMMIT, PAGE_READWRITE);
   if (szText) {
      if (DdeInitialize(&lIdInst, (PFNCALLBACK) GroupDDECallback,
            (DWORD) APPCMD_CLIENTONLY, 0L)) {
         VirtualFree (szText, 128, MEM_DECOMMIT);
         return;
      }
      szProgMan = DdeCreateStringHandle (lIdInst, "PROGMAN", CP_WINANSI);
      if (szProgMan) {
         hConv = DdeConnect (lIdInst, szProgMan, szProgMan, &CCFilter);
		 lstrcpy(szText,app_name);
         wsprintf (szCommand, "[DeleteGroup(%s)]", szText);
         hData = DdeCreateDataHandle (lIdInst, szCommand,
                  lstrlen (szCommand) + 1, 0, (HSZ) NULL, CF_TEXT, 0L);
         if (!DdeClientTransaction ((LPBYTE) hData, 0xFFFFFFFF, hConv,
                (HSZ) NULL, 0, XTYP_EXECUTE, 10000, &lResult)) {
               lResult = DdeGetLastError (lIdInst);
            }

         DdeFreeStringHandle (lIdInst, szProgMan);
         DdeDisconnect (hConv);
      } else {
         lResult = DdeGetLastError (lIdInst);
      }
      VirtualFree (szText, 64, MEM_DECOMMIT);
      VirtualFree (szCommand, 64, MEM_DECOMMIT);
      DdeUninitialize (lIdInst);
      lIdInst = 0L;
      return;
   } else {
   }
}

static BOOL CreateGroup(HWND hwnd) {
   HDDEDATA   hData;
   LPSTR      szText;
   LPSTR      szCommand;
   HCONV      hConv;
   HSZ        szProgMan;
   LONG       lResult;
   szText = VirtualAlloc (NULL, 64, MEM_COMMIT, PAGE_READWRITE);
   szCommand = VirtualAlloc (NULL, 128, MEM_COMMIT, PAGE_READWRITE);
   if (szText) {
      if (DdeInitialize(&lIdInst, (PFNCALLBACK) GroupDDECallback,
            (DWORD) APPCMD_CLIENTONLY, 0L)) {
//         MessageBox (hwnd, "DDEML Initialization Failure", "Error", MB_OK);
         VirtualFree (szText, 128, MEM_DECOMMIT);
         return (FALSE);
      }
      szProgMan = DdeCreateStringHandle (lIdInst, "PROGMAN", CP_WINANSI);
      if (szProgMan) {
         hConv = DdeConnect (lIdInst, szProgMan, szProgMan, &CCFilter);
		 lstrcpy(szText,app_name);
         wsprintf (szCommand, "[CreateGroup(%s)]", szText);
         hData = DdeCreateDataHandle (lIdInst, szCommand,
                  lstrlen (szCommand) + 1, 0, (HSZ) NULL, CF_TEXT, 0L);
         if (!DdeClientTransaction ((LPBYTE) hData, 0xFFFFFFFF, hConv,
                (HSZ) NULL, 0, XTYP_EXECUTE, 10000, &lResult)) {
               lResult = DdeGetLastError (lIdInst);
//               MessageBox (hwnd, "DdeClientTransaction Failed", "Error",
  //                MB_OK);
            }

         DdeFreeStringHandle (lIdInst, szProgMan);
         DdeDisconnect (hConv);
      } else {
         lResult = DdeGetLastError (lIdInst);
      }
      VirtualFree (szText, 64, MEM_DECOMMIT);
      VirtualFree (szCommand, 64, MEM_DECOMMIT);
      DdeUninitialize (lIdInst);
      lIdInst = 0L;
      return (TRUE);
   } else {
//      MessageBox (hwnd, "Memory Allocation failure", "Error", MB_OK);
   }
   return (FALSE);
}


static BOOL AddProgramItems (HWND hwnd, LPSTR szDummy) {
   HDDEDATA  hData;
   HCONV     hConv;
   HSZ       szProgMan;
   int       lSelCount;
   LONG      lResult;
   LPLONG    lpSelection;
   LPSTR     szProgName;
   LPSTR     szExePath;
   LPSTR     szExecuteString;
   int       iIndex;
   int       iGroupCount;

   iGroupCount = 2;
   lSelCount = 3;

   lpSelection = VirtualAlloc (NULL, lSelCount * sizeof (int), MEM_COMMIT,PAGE_READWRITE);
   if (lpSelection) {
      if (DdeInitialize (&lIdInst2, (PFNCALLBACK) GroupDDECallback,
            (DWORD) APPCMD_CLIENTONLY, 0L)) {
         VirtualFree (lpSelection, lSelCount * sizeof (int), MEM_DECOMMIT);
         return (FALSE);
      }

      szProgMan = DdeCreateStringHandle (lIdInst2, "PROGMAN", CP_WINANSI);
      hConv = DdeConnect (lIdInst2, szProgMan, szProgMan, &CCFilter);
      DdeFreeStringHandle (lIdInst2, szProgMan);
      szProgName = VirtualAlloc (NULL, MAX_PATH * 2, MEM_COMMIT, PAGE_READWRITE);
      szExePath = szProgName + MAX_PATH;

      szExecuteString = VirtualAlloc (NULL, MAX_PATH * 4, MEM_COMMIT,
            PAGE_READWRITE);

      for (iIndex = 0; iIndex < lSelCount; iIndex++) {
		 {
			 static char *paths[4] = {
					"Winamp.exe",
					"WhatsNew.txt",
					"Winamp.exe"
			 };
			 static char *names[4] = 
			 {
				 "Winamp",
				 "What's New",
				 "Uninstall Winamp",

			 };
			 lstrcpy(szProgName,names[iIndex]);
			 {
				 szExePath[0] = '\"';
				 GetModuleFileName(NULL,szExePath+1,MAX_PATH);
				 scanstr_back(szExePath,"\\",szExePath-1)[1]=0;
				 lstrcat(szExePath,paths[iIndex]);
				 lstrcat(szExePath,"\"");
				 if (iIndex==2) lstrcat(szExePath," /UNINSTALL");
			 }
		 }

// Create the command string to add the item.
         wsprintf (szExecuteString, "[AddItem(%s,%s)]", szExePath,
               (LPARAM) szProgName);

// Create a DDEML Data handle for the command string.
         hData = DdeCreateDataHandle (lIdInst2, szExecuteString,
               lstrlen (szExecuteString) + 1, 0, (HSZ) NULL, CF_TEXT, 0L);

// Send the command over to the program manager.
         if (!DdeClientTransaction ((LPBYTE) hData, 0xFFFFFFFF,
               hConv, (HSZ) NULL, 0, XTYP_EXECUTE, 1000, &lResult)) {
            lResult = DdeGetLastError (lIdInst2);
         }/*endIf*/
      }/*endFor*/

// Release the memory allocated for path and name retrieval.
      VirtualFree (szProgName, MAX_PATH * 2, MEM_DECOMMIT);

// Release the command line memory.
      VirtualFree (szExecuteString, MAX_PATH * 4, MEM_DECOMMIT);

// Disoconnect the DDEML Conversation
      DdeDisconnect (hConv);

// Release the memory allocate for the list selections.
      VirtualFree (lpSelection, lSelCount * sizeof (int), MEM_DECOMMIT);
   }/*endIf*/

// Clear the selection in the lists.

// Uninitialize the local conversation.
   DdeUninitialize (lIdInst2);
   
   lIdInst2 = 0L;
   return (TRUE);
}/* end AddProgramItems */
#endif

#endif