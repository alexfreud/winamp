#ifndef _ALPHAMGR_H
#define _ALPHAMGR_H

#include <api/timer/timerclient.h>

class Layout;

#define STATUS_UNKNOWN           -1
#define STATUS_OUT_OFF            0
#define STATUS_IN_ON              1
#define STATUS_OUT_FADINGOUT      2
#define STATUS_IN_FADINGON        3
#define STATUS_IN_OFF             4 // when no autoopacity

class AlphaMgrEntry {
  public:
    AlphaMgrEntry(Layout *l) : layout(l), status(STATUS_UNKNOWN), fade_val(-1), startalpha(-1), lasttime_in(0), next_in(-1) {}
    virtual ~AlphaMgrEntry() { }

    Layout *getLayout() { return layout; }
    int getStatus() { return status; }

    void onEnterLeave() { enterleave_time = Wasabi::Std::getTickCount(); }
    int getEnterLeaveTime() { return enterleave_time; }
    void setEnterLeaveTime(uint32_t t) { enterleave_time = t; }
    void setStatus(int s) { status = s; }
    int getStartAlpha() { return startalpha; }
    void setStartAlpha(int s) { startalpha = s; }
    uint32_t getLastTimeIn() { return lasttime_in; }
    void onLastIn() { lasttime_in = Wasabi::Std::getTickCount(); }
    void setNextIn(int i) { next_in = i; }
    int getNextIn() { return next_in; }

  private:
    Layout *layout;
    int status;
    int fade_val;
    uint32_t enterleave_time;
    int startalpha;
    uint32_t lasttime_in;
    int next_in;
};

class AlphaMgrEntryComparator {
public:
  static int compareItem(AlphaMgrEntry *p1, AlphaMgrEntry* p2) {
    return CMP3((void*)p1->getLayout(), (void *)p2->getLayout());
  }
  static int compareAttrib(const wchar_t *attrib, AlphaMgrEntry *item) {
    return CMP3((void *)attrib, (void *)item->getLayout());
  }
};

class AlphaMgr : public TimerClientDI {
  public:
    AlphaMgr();
    virtual ~AlphaMgr();

    void addLayout(Layout *l);
    void removeLayout(Layout *l);

    virtual void timerclient_timerCallback(int id);

    void updateTransparency(Layout *l);
    int getTransparency(Layout *l);
    int getGlobalAlpha();
    void setGlobalAlpha(int a);
    int isFocusInLayout(Layout *l);
    int isMouseInLayout(Layout *l);
    int isPointInLayout(Layout *l, int x, int y, api_region **rgn=NULL);
    int needForcedTransparencyFlag(Layout *l);
    int hasAutoOpacity(Layout *l);
    int hasAutoOpacityOnHover(Layout *l);
    int hasAutoOpacityOnFocus(Layout *l);
    
    void setAllLinked(int l) { alllinked = l; resetTimer(); updateAllTransparency(); }
    void setAutoOpacify(int l);
    int getAllLinked() { return alllinked; }
    int getAutoOpacify() { return (autoopacify && alllinked) ? autoopacify : 0; }
    void setExtendAutoOpacity(int n) { extend_px = n; }
    int getExtendAutoOpacity() { return extend_px; }
    int getBigCurTransparency();

    void setFadeInTime(int ms) { fadein_ms = MAX(ms, 1); }
    void setFadeOutTime(int ms) { fadeout_ms = MAX(ms, 1); }
    void setHoldTime(int ms) { holdtime_ms = ms; }
    void hoverCheck(Layout *l);
    int getAlpha(Layout *l);

  private:
    void updateAllTransparency();
    void updateInList(AlphaMgrEntry *e, int isin);
    void setBigStartAlpha(int a);
    int getBigStartAlpha() { return big_startalpha; }
    void onBigEnterLeave();
    uint32_t getBigEnterLeaveTime();
    void setBigStatus(int s);
    int getBigStatus() { return big_status; }
    void initStatus(AlphaMgrEntry *e, int applytransparency=0);
    int getAlpha(AlphaMgrEntry *e);
    int hasAutoOpacityOnHover(AlphaMgrEntry *e);
    int hasAutoOpacityOnFocus(AlphaMgrEntry *e);
    int hasAutoOpacity(AlphaMgrEntry *e);
    void checkTimer();
    void hoverCheck(AlphaMgrEntry *e, int applytransparency=1);
    void preHoverCheck(AlphaMgrEntry *e);
    int getCurve(AlphaMgrEntry *e);
    void doEndCheck(AlphaMgrEntry *e);
    void onBigLastIn() { big_lasttimein = Wasabi::Std::getTickCount(); }
    uint32_t getBigLastTimeIn() { return big_lasttimein; }
    int isFocusingExternalWindow();
    int isOverExternalWindow();
    int isOurExternalWindow(OSWINDOWHANDLE w);
    int isWasabiWindow(OSWINDOWHANDLE w);
    int isMenuWindow(OSWINDOWHANDLE w);
    void resetTimer();
  
    PtrListQuickSorted<AlphaMgrEntry, AlphaMgrEntryComparator> layouts;	
    PtrListQuickSortedByPtrVal<AlphaMgrEntry> in_layouts;	
    Layout *overlayout;
    int alllinked;
    int autoopacify;
    int global_alpha;
    int fast_timer_on;
    int big_status;
    int big_curtransparency;
    int big_startalpha;
    uint32_t big_enterleave_time;
    int big_lasttimein;
    int fadein_ms;
    int fadeout_ms;
    int holdtime_ms;
    int extend_px;
};

#endif
