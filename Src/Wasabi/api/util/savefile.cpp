#include "precomp.h"
//NONPORTABLE

#include <windows.h>
#include <commdlg.h>
#include "savefile.h"

#include "../bfc/basewnd.h"

#include "../studio/api.h"
#include "../studio/assert.h"

#include "../bfc/encodedstr.h"
#include "../studio/services/svc_stringconverter.h"

SaveFileWnd::SaveFileWnd(const char *ident) : identifier(ident),
  force_initial_dir(0) {}

void SaveFileWnd::setInitialDir(const char *dir, int force) {
  initial_dir = dir;
  force_initial_dir = force;
}

int SaveFileWnd::getSaveFile(api_window *parent, const char *ext, const char *suggext) {
  int retcode, failed = 0;
  if (ext == NULL) return 0;

  if (Std::encodingSupportedByOS(SvcStrCnv::UTF16)) {
    int ret = getSaveFileW(parent, ext, suggext);
    // If ret returns -1, the service is not available, we will call our own
    // OSNative translation failure routines below:
    if (ret != -1) {
      return ret;
    }
  }
  
  char savedir[WA_MAX_PATH];
  Std::getCurDir(savedir, WA_MAX_PATH);

  char filenamebuf[MAX_PATH]="";
  filename = NULL;

  OPENFILENAME ofn;
  ofn.lStructSize = sizeof ofn;
  ofn.hwndOwner = parent->gethWnd();
  ofn.hInstance = NULL;
  ofn.lpstrFilter = ext;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = filenamebuf;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;

  ofn.lpstrInitialDir = NULL;
  const char *initDir8 = NULL;

  // Figure out the initial directory in UTF8
  char dir[WA_MAX_PATH]="";
  String tname;
  if (identifier != NULL) {
    tname.printf("Recent directories/SaveFile/%s", identifier.getValue());
    if (force_initial_dir)
      initial_dir.strncpyTo(dir, sizeof(dir));
    else
      WASABI_API_CONFIG->getStringPublic(tname, dir, WA_MAX_PATH, initial_dir);
    if (*dir) initDir8 = dir;
  }

  // And then convert it when you're done to OSNATIVE.
  EncodedStr initDirOSenc;
  retcode = initDirOSenc.convertFromUTF8(SvcStrCnv::OSNATIVE, String(initDir8));
  if (retcode == SvcStrCnv::ERROR_UNAVAILABLE) {
    failed = 1;
    ofn.lpstrInitialDir = initDir8;
  } else { 
    ofn.lpstrInitialDir = static_cast<char *>(initDirOSenc.getEncodedBuffer());
  }

  ofn.lpstrTitle = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN|OFN_HIDEREADONLY;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lpstrDefExt = suggext;
  ofn.lCustData = 0;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;
  api->pushModalWnd();
  int ret = GetSaveFileName(&ofn);
  api->popModalWnd();

  if (failed) {
    if (ret)
      filename = filenamebuf;
  } else {
    // Okay, at this point we have the string in OSNATIVE format.
    // Now we downconvert everything to UTF8 and pass our information to the engine API's
    if (ret) {
      EncodedStr bufOSstr(SvcStrCnv::OSNATIVE, filenamebuf, STRLEN(filenamebuf)+1, 0/*no delete*/);
      bufOSstr.convertToUTF8(filename);
    }
  }

  // get new cur dir & save it off
  if ((identifier != NULL) && ret) {
    char newdir[WA_MAX_PATH];
    Std::getCurDir(newdir, WA_MAX_PATH);

    WASABI_API_CONFIG->setStringPublic(tname, newdir);
  }
  // put back old one
  Std::setCurDir(savedir);

  return ret;
}

int SaveFileWnd::getSaveFileW(api_window *parent, const char *ext, const char *suggext) {  

  // The ultimate top level retrohack. (sigh).
  // Test to see if the service is available.  If not, return -1.
  // The OSNATIVE codepath will include the proper failure handling.
  StringConverterEnum myServiceEnum(SvcStrCnv::UTF16);  // _ASSUME_ that there is a converter for U16 (we should, but still...)
  svc_stringConverter *myConv = myServiceEnum.getFirst();
  if (myConv != NULL) {
    myServiceEnum.release(myConv);
  } else {
    return -1;
  }

  char savedir[WA_MAX_PATH];
  Std::getCurDir(savedir, WA_MAX_PATH);

  WCHAR filenamebufW[MAX_PATH] = L"";
  filename = NULL;

  // convert multi const char* ext to U16 (Ascii7 assumptive)
  int thisz = 0, lastz = 0;
  const char *px;
  WCHAR *pr, *ext16 = static_cast<WCHAR *>(MALLOC(WA_MAX_PATH));
  for (px = ext, pr = ext16; *px | (lastz); px++, pr++) {
    lastz = (thisz)?1:0;
    *pr = (*px) & 0xFF;
    thisz = (*pr)?1:0;
  }
  *pr = *px; // the last 0
  
  EncodedStr suggext16;
  suggext16.convertFromUTF8(SvcStrCnv::UTF16, String(suggext));

  OPENFILENAMEW ofnW;
  ofnW.lStructSize = sizeof ofnW;
  ofnW.hwndOwner = parent->gethWnd();
  ofnW.hInstance = NULL;                                  
  ofnW.lpstrFilter = ext16;
  ofnW.lpstrCustomFilter = NULL;
  ofnW.nMaxCustFilter = 0;
  ofnW.nFilterIndex = 0;
  ofnW.lpstrFile = filenamebufW;
  ofnW.nMaxFile = MAX_PATH;
  ofnW.lpstrFileTitle = NULL;
  ofnW.nMaxFileTitle = 0;

  ofnW.lpstrInitialDir = NULL;

  char dir[WA_MAX_PATH]="";
  String tname;
  
  // Figure out the initial directory in UTF8
  const char *initDir8 = NULL;
  if (identifier != NULL) {
    tname.printf("Recent directories/SaveFile/%s", identifier.getValue());
    if (force_initial_dir)
      initial_dir.strncpyTo(dir, sizeof(dir));
    else
      WASABI_API_CONFIG->getStringPublic(tname, dir, WA_MAX_PATH, "");
    if (*dir) initDir8 = dir;
  }

  // And then convert it when you're done to UTF16.
  WCHAR *initDir16 = NULL;
  EncodedStr initDir16enc;
  if (initDir8 != NULL) {
    int written = initDir16enc.convertFromUTF8(SvcStrCnv::UTF16, String(initDir8));
    if (written > 0) {
       initDir16 = static_cast<WCHAR *>(initDir16enc.getEncodedBuffer());
    } else {
      return -1;
    }
  }

  // And then stuff it here.  Phew!
  ofnW.lpstrInitialDir = initDir16;

  ofnW.lpstrTitle = NULL;
  ofnW.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN|OFN_HIDEREADONLY;
  ofnW.nFileOffset = 0;
  ofnW.nFileExtension = 0;
  ofnW.lpstrDefExt = static_cast<WCHAR *>(suggext16.getEncodedBuffer());
  ofnW.lCustData = 0;
  ofnW.lpfnHook = NULL;
  ofnW.lpTemplateName = NULL;
  api->pushModalWnd();
  int ret = GetSaveFileNameW(&ofnW);
  api->popModalWnd();

  // Okay, at this point we have the string in UTF16 widechar format.
  // Now we downconvert everything to UTF8 and pass our information to the engine API's
  if (ret) {
    EncodedStr buf16str(SvcStrCnv::UTF16, filenamebufW, (WSTRLEN(filenamebufW)+1)*2, 0/*no delete*/);
    buf16str.convertToUTF8(filename);
  }

  // get new cur dir & save it off
  if ((identifier != NULL) && ret) {
    char newdir[WA_MAX_PATH];
    Std::getCurDir(newdir, WA_MAX_PATH);

    WASABI_API_CONFIG->setStringPublic(tname, newdir);
  }

  FREE(ext16);

  // put back old one
  Std::setCurDir(savedir);

  return ret;
}
