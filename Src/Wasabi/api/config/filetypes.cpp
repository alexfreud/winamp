#include "precomp.h"
#include "filetypes.h"
#include "api.h"
#include "main.h"
#include "core.h"

#include <bfc/attribs/attribs.h>
#include <bfc/attribs/attrcb.h>

#include "../common/locales.h"
#include "../bfc/paramparser.h"

// {DB26AA7F-0CF4-4e48-8AC8-49F9B9855A98}
static const GUID winampa_guid = 
{ 0xdb26aa7f, 0xcf4, 0x4e48, { 0x8a, 0xc8, 0x49, 0xf9, 0xb9, 0x85, 0x5a, 0x98 } };

class Filetypes;

class ExtensionAttrCallback : public AttrCallback {
public:
  ExtensionAttrCallback(const char *extname, Filetypes *ft) : ext(extname), filetypes(ft) {}
  virtual void onValueChange(Attribute *attr) {
    Filetypes::registerExtension(ext, attr->getValueAsInt());
    api->cmd_sendCommand(winampa_guid,"extchange",0,0,attr,sizeof(attr)); // CT> notifies winampa.wac of the change
                                                                          // (dunno if there is a better method)
    filetypes->updateKeepers();
  }
private:
  String ext;
  Filetypes *filetypes;
};

class KeepersCB : public AttrCallback {
public:
  KeepersCB(CfgItemI *_par) : par(_par) { }
  virtual void onValueChange(Attribute *attr) {
    //CT> This is REAL SLOW, for each attribute modified, you reregister them all, this needs to have
    //    an old list and just register/unregister what's new
    char bufero[WA_MAX_PATH]="";
    char data[WA_MAX_PATH]="";
    attr->getData(bufero, WA_MAX_PATH-1);
    ParamParser pp(bufero);
    const char *ext=api->core_getSupportedExtensions();
    const char *p=ext;
    while(*p!=0 && *(p+1)!=0) {
      if(*p) {
        int v = !!pp.hasString(p);
        int r = par->cfgitem_getData(StringPrintf("extension/%s", p), data, WA_MAX_PATH-1); 
        if (r == 0 || (!!ATOI(data) != v)) {
          par->cfgitem_setData(StringPrintf("extension/%s", p), StringPrintf(v));
        }
      }
      while(*p!=0) p++;
      p++;
    }
  }
private:
  CfgItemI *par;
};

#ifdef WIN32
static void DCM_cb(int v) {
  if (v) {
    HKEY mp3Key;
    char programname[MAX_PATH];
    GetModuleFileName(Main::gethInstance(),programname,sizeof(programname));
    if (RegCreateKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Directory\\shell\\Winamp3.Play",&mp3Key) == ERROR_SUCCESS) {
      const char *str=_("&Play in Winamp");
      RegSetValueEx(mp3Key, NULL,0,REG_SZ,(unsigned char*)str,strlen(str) + 1);
      RegCloseKey(mp3Key);
    }
    if (RegCreateKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Directory\\shell\\Winamp3.Play\\command",&mp3Key) == ERROR_SUCCESS) {
      StringPrintf mstr("\"%s\" \"%%1\"", programname);
      const char *blah = mstr;
      const unsigned char *microsoft_sucks_ass = (const unsigned char*)blah;
      RegSetValueEx(mp3Key, NULL,0,REG_SZ,microsoft_sucks_ass,mstr.len() + 1);
      RegCloseKey(mp3Key);
    }

    if (RegCreateKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Directory\\shell\\Winamp3.Enqueue",&mp3Key) == ERROR_SUCCESS) {
      const char *str=_("&Enqueue in Winamp");
      RegSetValueEx(mp3Key, NULL,0,REG_SZ,(unsigned char*)str,strlen(str) + 1);
      RegCloseKey(mp3Key);
    }
    if (RegCreateKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Directory\\shell\\Winamp3.Enqueue\\command",&mp3Key) == ERROR_SUCCESS) {
      StringPrintf mstr("\"%s\" /ADD \"%%1\"", programname);
      const char *blah = mstr;
      const unsigned char *microsoft_sucks_ass = (const unsigned char*)blah;
      RegSetValueEx(mp3Key, NULL,0,REG_SZ,microsoft_sucks_ass,mstr.len() + 1);
      RegCloseKey(mp3Key);
    }
  } else {
    Filetypes::myRegDeleteKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Directory\\shell\\Winamp3.Play");
    Filetypes::myRegDeleteKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Directory\\shell\\Winamp3.Enqueue");
  }
}
#endif

// {C30C97E6-77E6-4da4-9F4F-C7F848F8F641}
const GUID filetypes_guid = 
{ 0xc30c97e6, 0x77e6, 0x4da4, { 0x9f, 0x4f, 0xc7, 0xf8, 0x48, 0xf8, 0xf6, 0x41 } };

Filetypes::Filetypes() : CfgItemI("Filetypes", filetypes_guid) { }

void Filetypes::registerAttributes() {
  const char *ext=api->core_getSupportedExtensions();
  if (ext == NULL) return; //BU

  _bool *wantExtensions;
  registerAttribute(wantExtensions = new _bool("Register associations on Winamp startup", TRUE));

  const char *p=ext;
  while(*p!=0 && *(p+1)!=0) {
    if(*p) {
      _bool *extenabled;
      registerAttribute(extenabled = new _bool(StringPrintf("extension/%s", p), FALSE), new ExtensionAttrCallback(p, this));
      if (*wantExtensions) registerExtension(p, *extenabled); 
    } while(*p!=0) p++;
    p++;
  }

  registerAttribute(new _string("keepers"), new KeepersCB(this));

#ifdef WIN32
  registerAttribute(new _bool("Directory context menus", TRUE), new int_attrCB(DCM_cb));
#endif

  // Always steal the ".wal" extension.
  CfgItem *cift=WASABI_API_CONFIG->config_getCfgItemByGuid(filetypes_guid);
  if (cift) {
    cift->setDataAsInt("extension/wal",1);
  }

  updateKeepers(); 
}

void Filetypes::updateKeepers() {
  static int reentry=0;
  if (reentry) return;
  const char *ext=api->core_getSupportedExtensions();
  if (ext == NULL) return; //BU

  reentry = 1;
  const char *p=ext;
  String kstr;
  while(*p!=0 && *(p+1)!=0) {
    if(*p) {
      _bool *extenabled = static_cast<_bool *>(getAttributeByName(StringPrintf("extension/%s", p)));
      if (*extenabled) {
        if (!kstr.isempty()) kstr += ";";
        kstr += p;
      }
    } while(*p!=0) p++;
    p++;
  }
  _string *k = static_cast<_string *>(getAttributeByName("keepers"));
  k->setValue(kstr);
  reentry = 0;
}

LONG Filetypes::myRegDeleteKeyEx(HKEY thiskey, LPCTSTR lpSubKey)
{
#ifdef WIN32
	HKEY key;
	int retval=RegOpenKey(thiskey,lpSubKey,&key);
	if (retval==ERROR_SUCCESS)
	{
		char buffer[1024];
		while (RegEnumKey(key,0,buffer,1024)==ERROR_SUCCESS)
      if ((retval=myRegDeleteKeyEx(key,buffer)) != ERROR_SUCCESS) break;
		RegCloseKey(key);
		retval=RegDeleteKey(thiskey,lpSubKey);
	}
	return retval;
#else
  DebugString( "portme -- Filetypes::myRegDeleteKeyEx\n" );
        return 0;
#endif
}

int Filetypes::isRegistered(const char *ext) {
  char b[256];
  int rval=0;
  unsigned long vt,s=sizeof(b);
  HKEY key;
  SPRINTF(b,".%s",ext);
#ifdef WIN32
  if (RegOpenKey(HKEY_CLASSES_ROOT,b,&key) != ERROR_SUCCESS) return 0;
  if (RegQueryValueEx(key,NULL,0,&vt,b,&s) == ERROR_SUCCESS)	{
    if (vt != REG_SZ || (strcmp(b,"Winamp3.File") && strcmp(b,"Winamp3.PlayList") && strcmp(b,"Winamp3.SkinZip"))) rval=0;
    else rval=1;
  } else rval=0;
  RegCloseKey(key);
#else
DebugString( "portme -- FileTypes::isRegistered\n" );
#endif
  return rval;
}

void Filetypes::regmimetype(const char *mtype, const char *programname, const char *ext, int nsonly) {
#ifdef WIN32
  HKEY mp3Key;
  if (!nsonly) {
    char s[MAX_PATH];
    // Changed these to create rather than just open the mimetypes in the database.
    if (RegCreateKey(HKEY_CLASSES_ROOT,ext,&mp3Key) == ERROR_SUCCESS) {
      RegSetValueEx(mp3Key,"Content Type",0,REG_SZ,mtype,STRLEN(mtype) + 1);
      RegCloseKey(mp3Key);  
    }
    wsprintf(s,"MIME\\Database\\Content Type\\%s",mtype);
    if (RegCreateKey(HKEY_CLASSES_ROOT,s,&mp3Key) == ERROR_SUCCESS) {
      RegDeleteValue(mp3Key,"CLSID");
      RegSetValueEx(mp3Key,"Extension",0,REG_SZ,ext,strlen(ext) + 1);
      RegCloseKey(mp3Key);  
    }
  }
  if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Netscape\\Netscape Navigator\\Viewers",&mp3Key) == ERROR_SUCCESS) {
    int x;
    RegSetValueEx(mp3Key, mtype,0,REG_SZ,programname,strlen(programname) + 1);
    for (x = 0; x < 999; x ++) {
      char st[100];
      DWORD vt;
      DWORD s=128;
      char b[128];
      wsprintf(st,"TYPE%d",x);
	    if (RegQueryValueEx(mp3Key,st,0,&vt,b,&s) == ERROR_SUCCESS) {
        if (!strcmp(b,mtype)) break;
      } else {
        RegSetValueEx(mp3Key,st,0,REG_SZ,mtype,strlen(mtype)+1);
        break;
      }
    }
    RegCloseKey(mp3Key);  
  }
#else
DebugString( "portme -- Filetypes::regmimetype\n" );
#endif
}

void Filetypes::registerExtension(const char *ext, int reg) {
#ifdef WIN32
  createWinampTypes();

  char b[128];
  HKEY mp3Key;
  char *which_str="Winamp3.File";
	
  if (!_stricmp(ext,"m3u") || !_stricmp(ext,"pls")) which_str="Winamp3.PlayList";
  if (!_stricmp(ext,"wsz") || !_stricmp(ext,"wpz") || !_stricmp(ext,"wal"))
    which_str="Winamp3.SkinZip";
	wsprintf(b,".%s",ext);
  if (reg && !_stricmp(ext,"pls")) {
    char programname[MAX_PATH];
    GetModuleFileName(Main::gethInstance(),programname,sizeof(programname));
    regmimetype("audio/x-scpls", programname,".pls",1);
    regmimetype("audio/scpls", programname,".pls",1);
  }
  if (reg && !_stricmp(ext,"wma")) {
    char programname[MAX_PATH];
    GetModuleFileName(Main::gethInstance(),programname,sizeof(programname));
    regmimetype("audio/x-ms-wma", programname,".wma",1);
    regmimetype("application/x-msdownload", programname,".wma",1);
  }
  if (reg && !_stricmp(ext,"m3u")) {
    char programname[MAX_PATH];
    GetModuleFileName(Main::gethInstance(),programname,sizeof(programname));
    regmimetype("audio/x-mpegurl", programname,".m3u",1);
    regmimetype("audio/mpegurl", programname,".m3u",1);
  }
  if (reg && !_stricmp(ext,"mp3")) {
    char programname[MAX_PATH];
    GetModuleFileName(Main::gethInstance(),programname,sizeof(programname));
    regmimetype("audio/x-mpeg", programname,".mp3",1);
    regmimetype("audio/x-mp3", programname,".mp3",1);
    regmimetype("audio/x-mpg", programname,".mp3",1);
    regmimetype("audio/mp3", programname,".mp3",1);
    regmimetype("audio/mpg", programname,".mp3",1);
    regmimetype("audio/mpeg", programname,".mp3",1);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,b,&mp3Key) == ERROR_SUCCESS) {
    if (reg) {
      unsigned long s=sizeof(b);
      if (RegQueryValueEx(mp3Key,NULL,0,NULL,b,&s) == ERROR_SUCCESS) {
        if (strcmp(b,which_str)) {
          RegSetValueEx(mp3Key,"Winamp_Back",0,REG_SZ,b,strlen(b)+1);
          RegSetValueEx(mp3Key,NULL,0,REG_SZ,which_str,strlen(which_str)+1);
        }
      } else RegSetValueEx(mp3Key,NULL,0,REG_SZ,which_str,strlen(which_str)+1);
    } else {
      unsigned long s=sizeof(b);
      if (RegQueryValueEx(mp3Key,NULL,0,NULL,b,&s) == ERROR_SUCCESS) {
        if (!strcmp(b,which_str)) {
          s=sizeof(b);
          if (RegQueryValueEx(mp3Key,"Winamp_Back",0,NULL,b,&s) == ERROR_SUCCESS) {
            if (RegSetValueEx(mp3Key, NULL,0,REG_SZ,b,strlen(b)+1) == ERROR_SUCCESS)
              RegDeleteValue(mp3Key,"Winamp_Back");
          } else {
            RegDeleteValue(mp3Key,NULL);
            RegCloseKey(mp3Key);
            mp3Key=NULL;
            wsprintf(b,".%s",ext);
            myRegDeleteKeyEx(HKEY_CLASSES_ROOT,b);
          }
        } 
      }
    }
    if (mp3Key) RegCloseKey(mp3Key);
  }
#else
DebugString( "portme -- Filetypes::registerExtensions\n" );
#endif
}

void Filetypes::createWinampTypes() {
#ifdef WIN32
  HKEY mp3Key;
  char programname[MAX_PATH];
  char str[MAX_PATH+32];
  char buf[128]="Winamp3.File";
  char buf2[128]="Winamp3.PlayList";
  char buf3[128]="Winamp3.SkinZip";
  if (!GetModuleFileName(Main::gethInstance(),programname,sizeof(programname))) return;

  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File",&mp3Key) == ERROR_SUCCESS) {
	  strcpy(str,"Winamp media file");
	  RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    RegCloseKey(mp3Key);
  }

  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\DefaultIcon",&mp3Key) == ERROR_SUCCESS) {
	  wsprintf(str,"%s,%d",programname,whichicon);
	  RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
	  RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell",&mp3Key) == ERROR_SUCCESS) {
    if (addtolist) 
		  RegSetValueEx(mp3Key, NULL,0,REG_SZ,"Enqueue",8);
	  else
  		RegSetValueEx(mp3Key, NULL,0,REG_SZ,"Play",5);
    RegCloseKey(mp3Key);
  }

  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\Play",&mp3Key) == ERROR_SUCCESS) {
    const char *str=_("&Play in Winamp");
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\Play\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" \"%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\open",&mp3Key) == ERROR_SUCCESS) {
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,"",1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\open\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" \"%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }

  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\Enqueue",&mp3Key) == ERROR_SUCCESS) {
    const char *str=_("&Enqueue in Winamp");
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\Enqueue\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" /ADD \"%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }

  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\ListBookmark",&mp3Key) == ERROR_SUCCESS) {
    const char *str=_("Add to Winamp's &Bookmark list");
	  RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.File\\shell\\ListBookmark\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" /BOOKMARK \"%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }

  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.Playlist",&mp3Key) == ERROR_SUCCESS) {
    strcpy(str,"Winamp playlist file");
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    str[0]=0;
    str[1]=0;
    str[2]=1;
    str[3]=0;
    RegSetValueEx(mp3Key, "EditFlags",0,REG_BINARY,str,4);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\DefaultIcon",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"%s,%d",programname,whichicon2);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell",&mp3Key) == ERROR_SUCCESS) {
    if (addtolist) 
      RegSetValueEx(mp3Key, NULL,0,REG_SZ,"Enqueue",8);
    else
      RegSetValueEx(mp3Key, NULL,0,REG_SZ,"Play",5);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\Play",&mp3Key) == ERROR_SUCCESS) {
    const char *str=_("&Play in Winamp");
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\Play\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" \"%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\open",&mp3Key) == ERROR_SUCCESS) {
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,"",1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\open\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" \"%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\Enqueue",&mp3Key) == ERROR_SUCCESS) {
    const char *str=_("&Enqueue in Winamp");
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\Enqueue\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" /ADD \"%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\ListBookmark",&mp3Key) == ERROR_SUCCESS) {
    const char *str=_("Add to Winamp's &Bookmark list");
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.PlayList\\shell\\ListBookmark\\command",&mp3Key) == ERROR_SUCCESS) {
	  wsprintf(str,"\"%s\" /BOOKMARK \"%%1\"",programname);
	  RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
	  RegCloseKey(mp3Key);  
  }

  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.SkinZip",&mp3Key) == ERROR_SUCCESS) {
    strcpy(str,"Winamp3 skin file");
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    str[0]=0;
    str[1]=0;
    str[2]=1;
    str[3]=0;
    RegSetValueEx(mp3Key, "EditFlags",0,REG_BINARY,str,4);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.SkinZip\\DefaultIcon",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"%s,%d",programname,whichicon);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str)+1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.SkinZip\\shell",&mp3Key) == ERROR_SUCCESS) {
		RegSetValueEx(mp3Key, NULL,0,REG_SZ,"Install and switch to",8);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.SkinZip\\shell\\install",&mp3Key) == ERROR_SUCCESS) {
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,"",1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.SkinZip\\shell\\install\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" \"/installskin=%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.SkinZip\\shell\\open",&mp3Key) == ERROR_SUCCESS) {
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,"",1);
    RegCloseKey(mp3Key);
  }
  if (RegCreateKey(HKEY_CLASSES_ROOT,"Winamp3.SkinZip\\shell\\open\\command",&mp3Key) == ERROR_SUCCESS) {
    wsprintf(str,"\"%s\" \"/installskin=%%1\"",programname);
    RegSetValueEx(mp3Key, NULL,0,REG_SZ,str,strlen(str) + 1);
    RegCloseKey(mp3Key);  
  }

  // Register the mimetypes to act like .wal files?

//  regmimetype("interface/x-winamp-skin", programname,".wsz",0);
  regmimetype("interface/x-winamp-skin", programname,".wal",0);
//  regmimetype("interface/x-winamp3-skin", programname,".wsz",0);
  regmimetype("interface/x-winamp3-skin", programname,".wal",0);
//  regmimetype("application/x-winamp-plugin", programname,"wpz",0);
#else
DebugString( "portme -- Filetypes::createWinampTypes\n" );
#endif
}

int Filetypes::isCdPlayer() {
	int r=0;
#ifdef WIN32
  unsigned long s;
	HKEY mp3Key;
	char buf[MAX_PATH],buf2[MAX_PATH]="\"";
  if (!GetModuleFileName(Main::gethInstance(),buf2+1,sizeof(buf2)-8)) return 0;
	if (RegOpenKey(HKEY_CLASSES_ROOT,"AudioCD\\shell\\play\\command",&mp3Key) != ERROR_SUCCESS) return 0;
	strcat(buf2,"\" /CDA:%1");
	s=sizeof(buf);
	if (RegQueryValueEx(mp3Key,NULL,0,NULL,buf,&s) == ERROR_SUCCESS)
	{
		if (!lstrcmpi(buf,buf2)) r=1;
	}
	RegCloseKey(mp3Key);
#else
DebugString( "portme -- Filetypes::isCdPlayer\n" );
#endif
	return r;
}

void Filetypes::registerCdPlayer(int reg) {
#ifdef WIN32
	char b[MAX_PATH];
	char buf2[MAX_PATH]="\"";
	HKEY mp3Key;
  if (!GetModuleFileName(Main::gethInstance(),buf2+1,sizeof(buf2)-8)) return;
	strcat(buf2,"\" /CDA:%1");
	if (RegOpenKey(HKEY_CLASSES_ROOT,"AudioCD\\shell\\play\\command",&mp3Key) == ERROR_SUCCESS) 
	{
		if (reg)
		{
			unsigned long s=sizeof(b);
			if (RegQueryValueEx(mp3Key,NULL,0,NULL,b,&s) == ERROR_SUCCESS)
			{
 				if (_stricmp(b,buf2))
				{
          char buf3[MAX_PATH];
          unsigned long st=sizeof(buf3);
					if (RegQueryValueEx(mp3Key,"Winamp_Back",0,NULL,buf3,&st) != ERROR_SUCCESS ||
              _stricmp(buf3,b))
					{
  					RegSetValueEx(mp3Key,"Winamp_Back",0,REG_SZ,b,strlen(b)+1);
					} 
					RegSetValueEx(mp3Key,NULL,0,REG_SZ,buf2,strlen(buf2)+1);
				}
			} else RegSetValueEx(mp3Key,NULL,0,REG_SZ,buf2,strlen(buf2)+1);
		}
		else
		{
			unsigned long s=sizeof(b);
			if (RegQueryValueEx(mp3Key,NULL,0,NULL,b,&s) == ERROR_SUCCESS)
			{
				if (!strcmp(b,buf2))
				{
					s=sizeof(b);
					if (RegQueryValueEx(mp3Key,"Winamp_Back",0,NULL,b,&s) == ERROR_SUCCESS)
					{
            if (!_stricmp(b,buf2)) b[0]=0;
						if (RegSetValueEx(mp3Key, NULL,0,REG_SZ,b,strlen(b)+1) == ERROR_SUCCESS)
							RegDeleteValue(mp3Key,"Winamp_Back");
					} 
          else 
          {
            buf2[0]=0;
            RegSetValueEx(mp3Key,NULL,0,REG_SZ,buf2,strlen(buf2)+1);
          }
				} 
			}
		}
		RegCloseKey(mp3Key);
	}
#else
DebugString( "portme -- Filetypes::isCdPlayer\n" );
#endif
}

int Filetypes::whichicon=1;
int Filetypes::whichicon2=1;
int Filetypes::addtolist=0;
