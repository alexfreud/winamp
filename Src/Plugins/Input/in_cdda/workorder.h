#ifndef NULLSOFT_IN_CDDA_WORKORDER_H
#define NULLSOFT_IN_CDDA_WORKORDER_H

#include "../gracenote/CDDBPluginWorkOrderManager.h"

extern CDDBModuleWorkOrderManagerInterface *workorder;
void OpenMusicIDWorkOrder();
void ShutdownMusicIDWorkOrder();

#endif