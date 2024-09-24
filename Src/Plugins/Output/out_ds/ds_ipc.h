#ifndef DS_IPC_H
#define DS_IPC_H

#define DS_IPC_CLASSNAME "DSound_IPC"

#define WM_DS_IPC WM_USER

#define DS_IPC_CB_CFGREFRESH          0   // trap this to detect when apply/ok was pressed in config
#define DS_IPC_CB_ONSHUTDOWN          1   // trap this to detect when out_ds is going away (ie: another output plugin was selected, or winamp is exiting)

#define DS_IPC_SET_CROSSFADE          100 // send this with wParam = 0/1 to change fade on end setting
#define DS_IPC_GET_CROSSFADE          101 // returns fade on end on/off 
#define DS_IPC_SET_CROSSFADE_TIME     102 // send this with wParam = fade on end time in ms 
#define DS_IPC_GET_CROSSFADE_TIME     103 // returns fade on end time in ms

#endif
