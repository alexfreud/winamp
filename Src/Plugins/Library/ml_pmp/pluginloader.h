#ifndef __PLUGINLOADER_H_
#define __PLUGINLOADER_H_

#include <windows.h>
#include "..\..\General\gen_ml/ml.h"
#include "pmp.h"
#include "..\..\General\gen_ml/itemlist.h"

BOOL testForDevPlugins();
BOOL loadDevPlugins(int *count);
void unloadDevPlugins();
int wmDeviceChange(WPARAM wParam, LPARAM lParam);

#endif