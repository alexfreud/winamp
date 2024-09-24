/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////
//
// CPlugin class implementation
//
#include <windows.h>
#include <windowsx.h>
#include "plugin.h"
#include "npupp.h"
#include "nsError.h"
#include <malloc.h>

static NPIdentifier sGetVersion_id;
//static NPObject *sWindowObj;

#define BUFFER_LEN 1024

// Helper class that can be used to map calls to the NPObject hooks
// into virtual methods on instances of classes that derive from this
// class.
class ScriptablePluginObjectBase : public NPObject
{
public:
  ScriptablePluginObjectBase(NPP npp)
    : mNpp(npp)
  {
  }

  virtual ~ScriptablePluginObjectBase()
  {
  }

  // Virtual NPObject hooks called through this base class. Override
  // as you see fit.
  virtual void Invalidate();
  virtual bool HasMethod(NPIdentifier name);
  virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                      uint32_t argCount, NPVariant *result);
  virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                             NPVariant *result);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
  virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
  virtual bool RemoveProperty(NPIdentifier name);

public:
  static void _Deallocate(NPObject *npobj);
  static void _Invalidate(NPObject *npobj);
  static bool _HasMethod(NPObject *npobj, NPIdentifier name);
  static bool _Invoke(NPObject *npobj, NPIdentifier name,
                      const NPVariant *args, uint32_t argCount,
                      NPVariant *result);
  static bool _InvokeDefault(NPObject *npobj, const NPVariant *args,
                             uint32_t argCount, NPVariant *result);
  static bool _HasProperty(NPObject * npobj, NPIdentifier name);
  static bool _GetProperty(NPObject *npobj, NPIdentifier name,
                           NPVariant *result);
  static bool _SetProperty(NPObject *npobj, NPIdentifier name,
                           const NPVariant *value);
  static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);

protected:
  NPP mNpp;
};

#define DECLARE_NPOBJECT_CLASS_WITH_BASE(_class, ctor)                        \
static NPClass s##_class##_NPClass = {                                        \
  NP_CLASS_STRUCT_VERSION,                                               \
  ctor,                                                                       \
  ScriptablePluginObjectBase::_Deallocate,                                    \
  ScriptablePluginObjectBase::_Invalidate,                                    \
  ScriptablePluginObjectBase::_HasMethod,                                     \
  ScriptablePluginObjectBase::_Invoke,                                        \
  ScriptablePluginObjectBase::_InvokeDefault,                                 \
  ScriptablePluginObjectBase::_HasProperty,                                   \
  ScriptablePluginObjectBase::_GetProperty,                                   \
  ScriptablePluginObjectBase::_SetProperty,                                   \
  ScriptablePluginObjectBase::_RemoveProperty,                                \
}

#define GET_NPOBJECT_CLASS(_class) &s##_class##_NPClass

void
ScriptablePluginObjectBase::Invalidate()
{
}

bool
ScriptablePluginObjectBase::HasMethod(NPIdentifier name)
{
  return false;
}

bool
ScriptablePluginObjectBase::Invoke(NPIdentifier name, const NPVariant *args,
                                   uint32_t argCount, NPVariant *result)
{
  return false;
}

bool
ScriptablePluginObjectBase::InvokeDefault(const NPVariant *args,
                                          uint32_t argCount, NPVariant *result)
{
  return false;
}

bool
ScriptablePluginObjectBase::HasProperty(NPIdentifier name)
{
  return false;
}

bool
ScriptablePluginObjectBase::GetProperty(NPIdentifier name, NPVariant *result)
{
  return false;
}

bool
ScriptablePluginObjectBase::SetProperty(NPIdentifier name,
                                        const NPVariant *value)
{
  return false;
}

bool
ScriptablePluginObjectBase::RemoveProperty(NPIdentifier name)
{
  return false;
}

// static
void
ScriptablePluginObjectBase::_Deallocate(NPObject *npobj)
{
  // Call the virtual destructor.
  delete (ScriptablePluginObjectBase *)npobj;
}

// static
void
ScriptablePluginObjectBase::_Invalidate(NPObject *npobj)
{
  ((ScriptablePluginObjectBase *)npobj)->Invalidate();
}

// static
bool
ScriptablePluginObjectBase::_HasMethod(NPObject *npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->HasMethod(name);
}

// static
bool
ScriptablePluginObjectBase::_Invoke(NPObject *npobj, NPIdentifier name,
                                    const NPVariant *args, uint32_t argCount,
                                    NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->Invoke(name, args, argCount,
                                                       result);
}

// static
bool
ScriptablePluginObjectBase::_InvokeDefault(NPObject *npobj,
                                           const NPVariant *args,
                                           uint32_t argCount,
                                           NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->InvokeDefault(args, argCount,
                                                              result);
}

// static
bool
ScriptablePluginObjectBase::_HasProperty(NPObject * npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->HasProperty(name);
}

// static
bool
ScriptablePluginObjectBase::_GetProperty(NPObject *npobj, NPIdentifier name,
                                         NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->GetProperty(name, result);
}

// static
bool
ScriptablePluginObjectBase::_SetProperty(NPObject *npobj, NPIdentifier name,
                                         const NPVariant *value)
{
  return ((ScriptablePluginObjectBase *)npobj)->SetProperty(name, value);
}

// static
bool
ScriptablePluginObjectBase::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->RemoveProperty(name);
}

class ScriptablePluginObject : public ScriptablePluginObjectBase
{
public:
  ScriptablePluginObject(NPP npp)
    : ScriptablePluginObjectBase(npp)
  {
  }

  virtual bool HasMethod(NPIdentifier name);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
  virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                      uint32_t argCount, NPVariant *result);
  virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                             NPVariant *result);
};

static NPObject *
AllocateScriptablePluginObject(NPP npp, NPClass *aClass)
{
  return new ScriptablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject,
                                 AllocateScriptablePluginObject);

bool
ScriptablePluginObject::HasMethod(NPIdentifier name)
{
  return name == sGetVersion_id;
}

bool
ScriptablePluginObject::HasProperty(NPIdentifier name)
{
  return PR_FALSE;
}

bool
ScriptablePluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
  VOID_TO_NPVARIANT(*result);
    
  return PR_FALSE;
}

bool
ScriptablePluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
  VOID_TO_NPVARIANT(*result);

  if (name == sGetVersion_id) {
	char csVersion[BUFFER_LEN];
	memset(&csVersion[0], '\0', BUFFER_LEN);

	DWORD BufferSize = BUFFER_LEN;
    DWORD cbData;
	bool keyFound = false;

	wchar_t exeName[] = L"\\winamp.exe";
    wchar_t fileName[BUFFER_LEN]; 
	memset(&fileName[0],'\0',BUFFER_LEN);
    wchar_t fileNameTemp[BUFFER_LEN]; 

	HKEY hKey;
	cbData = BUFFER_LEN;

	// first check the protocol handler registry key, we're looking for
	// the winamp:// protocol handler. If we find this, then this is the
	// "right" exe for winamp we need to get the version number on
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("winamp\\shell\\open\\command"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		if ( RegQueryValueEx( hKey,
								 TEXT(""),
								 NULL,
								 NULL,
								 (LPBYTE) fileNameTemp,
								 &cbData ) != ERROR_SUCCESS) {
				return PR_FALSE;
		}

		RegCloseKey (hKey);
		if (wcsstr(fileNameTemp,L"winamp.exe")) {
			int indexOfFirstQuote = wcscspn(fileNameTemp, L"\"");
			int indexOfSecondQuote = wcscspn(&fileNameTemp[indexOfFirstQuote+1], L"\"");
			if (indexOfFirstQuote >= 0) {
				keyFound = true;
				wcsncpy(fileName,&fileNameTemp[indexOfFirstQuote+1], indexOfSecondQuote);
			} 
		} else {
			// some other app (itunes ??) controlling the winamp:// protocol
			// return error
			return PR_FALSE;
		}
	}

	if (!keyFound) {
		// See if the reg key exists
		if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Winamp"), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
			return PR_FALSE;
		}

		cbData = BUFFER_LEN;
		if ( RegQueryValueEx( hKey,
								 TEXT(""),
								 NULL,
								 NULL,
								 (LPBYTE) fileName,
								 &cbData ) != ERROR_SUCCESS) {
			return PR_FALSE;
		}
		RegCloseKey (hKey);
		keyFound = true;
		wcscat(fileName,exeName);
	}

	if (!keyFound) {
		return PR_FALSE;
	}

	static TCHAR sBackSlash[] = {'\\','\0'};
	DWORD dwVersionDataLen = GetFileVersionInfoSize(fileName, NULL);

	if (dwVersionDataLen) {
		char* fvBuf = (char *)alloca(dwVersionDataLen);
		if (GetFileVersionInfo(fileName, 0, dwVersionDataLen, fvBuf)) {
			
			LPVOID pVal;
			UINT nValLen;
			if (VerQueryValue(fvBuf, sBackSlash, &pVal, &nValLen)) {
				if (nValLen == sizeof(VS_FIXEDFILEINFO)) {
					VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)pVal;
					//sprintf(csVersion, "%d.%d.%d.%d",
						//HIWORD(pFixedFileInfo->dwFileVersionMS), LOWORD(pFixedFileInfo->dwFileVersionMS),
						//HIWORD(pFixedFileInfo->dwFileVersionLS), LOWORD(pFixedFileInfo->dwFileVersionLS));
					sprintf(csVersion, "%d.%d%d",
						HIWORD(pFixedFileInfo->dwFileVersionMS), LOWORD(pFixedFileInfo->dwFileVersionMS),
						HIWORD(pFixedFileInfo->dwFileVersionLS));
				}
			}
		}
	}

	size_t versionLen = (uint32)strlen(&csVersion[0]) + 1;
	char *targetResult = (char *) NPN_MemAlloc(versionLen);

	if (targetResult != NULL )
		memcpy(targetResult, csVersion, versionLen);
	else
		return PR_FALSE;

    STRINGZ_TO_NPVARIANT(targetResult, *result);
    return PR_TRUE;
  }

  return PR_FALSE;
}

bool
ScriptablePluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
	VOID_TO_NPVARIANT(*result);

	return PR_FALSE;
}

CPlugin::CPlugin(NPP pNPInstance) :
  m_pNPInstance(pNPInstance),
  m_bInitialized(FALSE),
  m_pScriptableObject(NULL)
{
  //NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &sWindowObj);

  sGetVersion_id = NPN_GetStringIdentifier("getVersion");
}

CPlugin::~CPlugin()
{
  //if (sWindowObj)
  //  NPN_ReleaseObject(sWindowObj);
  if (m_pScriptableObject)
    NPN_ReleaseObject(m_pScriptableObject);

  //sWindowObj = 0;
}

NPBool CPlugin::init(NPWindow* pNPWindow)
{
  if(pNPWindow == NULL)
    return FALSE;

  m_Window = pNPWindow;

  m_bInitialized = TRUE;
  return TRUE;
}

void CPlugin::shut()
{
  m_bInitialized = FALSE;
}

NPBool CPlugin::isInitialized()
{
  return m_bInitialized;
}


int16 CPlugin::handleEvent(void* event)
{
  return 0;
}

NPObject *
CPlugin::GetScriptableObject()
{
  if (!m_pScriptableObject) {
    m_pScriptableObject =
      NPN_CreateObject(m_pNPInstance,
                       GET_NPOBJECT_CLASS(ScriptablePluginObject));
  }

  if (m_pScriptableObject) {
    NPN_RetainObject(m_pScriptableObject);
  }

  return m_pScriptableObject;
}