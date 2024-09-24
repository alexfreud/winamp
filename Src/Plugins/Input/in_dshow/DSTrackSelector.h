#ifndef NULLSOFT_IN_DSHOW_DSTRACKSELECTOR_H
#define NULLSOFT_IN_DSHOW_DSTRACKSELECTOR_H

#include "../Winamp/wa_ipc.h"

class DSTrackSelector : public ITrackSelector {
public:
    virtual int getNumAudioTracks();
    virtual void enumAudioTrackName(int n, char *buf, int size);
    virtual int getCurAudioTrack();
    virtual int getNumVideoTracks();
    virtual void enumVideoTrackName(int n, char *buf, int size);
    virtual int getCurVideoTrack();	
    virtual void setAudioTrack(int n);
    virtual void setVideoTrack(int n);
};

#endif