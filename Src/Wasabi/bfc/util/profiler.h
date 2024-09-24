#ifndef _PROFILER_H
#define _PROFILER_H

#include <bfc/wasabi_std.h>
#include <bfc/string/bfcstring.h>
#include <bfc/ptrlist.h>


#ifdef NO_PROFILING
#define PR_ENTER(msg) 
#define PR_LEAVE()
#else

#define _PR_ENTER(msg, line) { __Profiler __prx##line(msg) 
#define _PR_ENTER2(msg, msg2, line) { __Profiler __prx##line(msg, msg2) 
#define PR_ENTER(msg) _PR_ENTER(msg, line)
#define PR_ENTER2(msg, msg2) _PR_ENTER2(msg, msg2, line) 
#define PR_LEAVE() }

class __ProfilerEntry {
  public: 
    __ProfilerEntry(const char *txt) { text = txt; totaltime = 0; totaln = 0; subcount = 0; lastcps = -1;}
    virtual ~__ProfilerEntry() {}

    void add(float ms) { totaltime += ms; totaln++; 
                         if (subcount == 0) {
                           firstcall = Wasabi::Std::getTimeStampMS();
                         }
                         if (Wasabi::Std::getTimeStampMS() - firstcall < 1) {
                           subcount++; 
                         } else {
                           lastcps = subcount;
                           subcount = 0;
                         }
                       }
    float getAverage() { if (totaln == 0) return 0; return totaltime / (float)totaln; }
    float getTotal() { return totaltime; }
    const char *getText() { return text; }
    int getLastCPS() { return lastcps; }

  private:
    float totaltime;
    int totaln;
    stdtimevalms firstcall;
    int lastcps;
    int subcount;
    String text;
};

class __ProfilerEntrySort {
public:
  static int compareAttrib(const wchar_t *attrib, void *item) {
    return STRICMP((const char *)attrib, ((__ProfilerEntry*)item)->getText());
  }
  static int compareItem(void *i1, void *i2) {
    return STRICMP(((__ProfilerEntry*)i1)->getText(), ((__ProfilerEntry*)i2)->getText());
  }
};

extern COMEXP PtrListInsertSorted<__ProfilerEntry, __ProfilerEntrySort> __profiler_entries;
extern COMEXP int __profiler_indent;

class __ProfilerManager {
  public:
  static void log(const char *txt, float ms, float *total, float *average, int *lastcps) {
    int pos=-1;
    __ProfilerEntry *e = __profiler_entries.findItem((const wchar_t *)txt, &pos);
    if (pos < 0 || e == NULL) {
      e = new __ProfilerEntry(txt);
      __profiler_entries.addItem(e);
    }
    if (e != NULL) {
      e->add(ms);
      if (total != NULL) *total = e->getTotal();
      if (average != NULL) *average = e->getAverage();
      if (lastcps != NULL) *lastcps = e->getLastCPS();
    }
  }
};

#undef USE_TICK_COUNT

class __Profiler {
public:
  __Profiler(const char *text, const char *text2="") : str(text), str2(text2) {
    if (!str2.isempty()) str2 += " ";
#ifdef USE_TICK_COUNT
    ts1 = GetTickCount();
#else
    ts1 = Wasabi::Std::getTimeStampMS();
#endif
    __profiler_indent++;
  }
  ~__Profiler() {
    __profiler_indent--;
#ifdef USE_TICK_COUNT
    stdtimevalms ts2 = GetTickCount();
#else
    stdtimevalms ts2 = Wasabi::Std::getTimeStampMS();
#endif
    float ms = (float)((ts2 - ts1)
#ifndef USE_TICK_COUNT
*1000.0
#endif
);
    float total=0;
    float average=0;
    int lastcps=0;
    __ProfilerManager::log(str, ms, &total, &average, &lastcps);
    char buf[4096];
    if (lastcps >= 0) 
      sprintf(buf, "%*sProfiler: %s: %s%6.4f ms (total: %6.4f ms, average: %6.4f ms, calls per second : %d)\n", __profiler_indent*4, " ", str.getValue(), str2.getValue(), ms, total, average, lastcps);
    else
      sprintf(buf, "%*sProfiler: %s: %s%6.4f ms (total: %6.4f ms, average: %6.4f ms)\n", __profiler_indent*4, " ", str.getValue(), str2.getValue(), ms, total, average);
#ifdef _WIN32
    OutputDebugStringA(buf);
#else
#warning port me
#endif
  }
private:
  String str, str2;
  stdtimevalms ts1;
};

#endif//!NO_PROFILING

#endif
