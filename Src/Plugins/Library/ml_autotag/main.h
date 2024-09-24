#ifndef NULLSOFT_ML_AUTOTAG_MAIN_H
#define NULLSOFT_ML_AUTOTAG_MAIN_H

#include <windows.h>
#include <shlwapi.h>
#include "../../General/gen_ml/ml.h"
#include "../nu/AutoWide.h"
#include "../nu/listview.h"

#include "resource.h"

#include <api/service/waServiceFactory.h>

#include "../gracenote/api_gracenote.h"
extern api_gracenote * gracenoteApi;
#define AGAVE_API_GRACENOTE gracenoteApi

#include "../Winamp/api_decodefile.h"
extern api_decodefile *decodeApi;
#define AGAVE_API_DECODE decodeApi

#include "../ml_local/api_mldb.h"
extern api_mldb *mldbApi;
#define AGAVE_API_MLDB mldbApi

#include "../Agave/Language/api_language.h"

extern winampMediaLibraryPlugin plugin;
INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

LRESULT SetFileInfo(const wchar_t *filename, const wchar_t *metadata, const wchar_t *data);
void WriteFileInfo(const wchar_t *file);
int GetFileInfo(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len);

#endif