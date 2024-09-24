#include <precomp.h>
#include <api/api.h>
#include <api/apiinit.h>
#include <api/service/svcmgr.h>
#include <api/timer/timerclient.h>
#include <api/timer/timermul.h>
#include <api/xml/xmlreader.h>
#include <bfc/string/bigstring.h>
#include <bfc/string/stringdict.h>

// This is the Wasabi Library Test Application

DECLARE_MODULE_SVCMGR

#define TIMER_TEST_DURATION (MAX_TIMER_DELAY/1000)*4 // We should at least use (MAX_TIMER_DELAY/1000)*2 here so as to test the low speed timer cycle at least twice

int failed = 0;

#if defined(WASABI_COMPILE_TIMERS) || defined(WASABI_COMPILE_WND) || defined(WASABI_COMPILE_TEXTMODE)

int exitpump = 0;

//-------------------------------------------------------------------------------------------

void doMessagePump() {
  exitpump = 0;
  // Despite appearances, this is portable
  MSG msg;
  while (!exitpump && GetMessage( &msg, NULL, 0, 0 ) ) {
#ifdef WASABI_COMPILE_WND
    TranslateMessage( &msg );
#endif
    DispatchMessage( &msg );
  }
}

//-------------------------------------------------------------------------------------------

void exitMessagePump() {
  exitpump = 1;
}

#endif

//-------------------------------------------------------------------------------------------
void fail(const char *module, const char *test) {
  failed++;
  printf("\n\n*** FAILED *** : %s (%s)\n\n", module, test);
  fflush(stdout);
}

//-------------------------------------------------------------------------------------------

#ifdef WASABI_COMPILE_TIMERS

// Multiplexed timers test

int timer[10];

class TestTimer : public TimerClientDI {
public:
  TestTimer() {
    for (int id = 0; id < 10; id++) {
      timer[id] = 0;
      timerclient_setTimer(id+1, id*100+100);
    }
  }
  virtual ~TestTimer() {
    for (int id = 0; id < 10; id++) {
      timerclient_killTimer(id+1);
    }
  }
  virtual void timerclient_timerCallback(int id) {
    if (id >= 1 && id <= 10) {
      timer[id-1]+=timerclient_getSkipped()+1;
      if (id == 1 || id == 10) {
        printf("\r");
        for (int i = 0; i < 10; i++) {
          if (i > 0) printf(" | ");
          printf("%d:%3d", i+1, timer[i]);
        }
        fflush(stdout);
      }
    if (id == 10 && timer[id-1] >= TIMER_TEST_DURATION)
      exitMessagePump();
    }
  }
};
#endif

//-------------------------------------------------------------------------------------------
#ifdef WASABI_COMPILE_XMLPARSER
// Xml parser test
enum XML_TEST_TAGS {
  XML_TEST_ROOT,
  XML_TEST_TEST1,
  XML_TEST_TEST2,
};

BEGIN_STRINGDICTIONARY(_XMLTESTTAGS);
SDI("WasabiTest", XML_TEST_ROOT);
SDI("Test1",      XML_TEST_TEST1);
SDI("Test2",      XML_TEST_TEST2);
END_STRINGDICTIONARY(_XMLTESTTAGS, xmltesttags);

class XmlTest : public XmlReaderCallbackI {
public:
  XmlTest() : m_failed(0), m_inroot(0), m_intest1(0), m_intest2(0) {
    BigString str;
    str += "buf:";
    str += "<WasabiTest>\n";
    str += " <Test1>success</Test1>\n";
    str += " <Test2 result=\"success\"/>\n";
    str += "</WasabiTest>\n";
    XmlReader::registerCallback("*", static_cast<api_xmlreadercallback*>(this));
    XmlReader::loadFile(str, NULL, static_cast<api_xmlreadercallback*>(this));
    XmlReader::unregisterCallback(static_cast<api_xmlreadercallback*>(this));
  }
  virtual ~XmlTest() {
  }
  virtual int xmlReaderDisplayErrors() { return 0; }
  virtual void xmlReaderOnError(const char *filename, int linenum, const char *incpath, int errcode, const char *errstr) {
    fail("XML TEST", "Parse error");
  }
  virtual void xmlReaderOnStartElementCallback(const char *xmlpath, const char *xmltag, api_xmlreaderparams *params) {
    switch (xmltesttags.getId(xmltag)) {
      case XML_TEST_ROOT:
        m_inroot++;
        printf("  <WasabiTest>\n");
        fflush(stdout);
        break;
      case XML_TEST_TEST1:
        m_intest1++;
        printf("    <Test1>\n");
        fflush(stdout);
        break;
      case XML_TEST_TEST2:
        m_intest2++;
        printf("    <Test2\n");
        int _failed = 0;
        for (int i=0;i<params->getNbItems();i++) {
          printf("      %s = \"%s\"\n", params->getItemName(i), params->getItemValue(i));
          if (i == 1) {
            if (!STRCASEEQLSAFE(params->getItemValue(i), "success")) _failed = 1;
            if (!STRCASEEQLSAFE(params->getItemName(i), "result")) _failed = 1;
          }
        }
        if (_failed) {
          m_failed = 1;
          fail("XML PARSER", "Not receiving the right params in Test2");
        }
        fflush(stdout);
        break;
    }
  }
  virtual void xmlReaderOnEndElementCallback(const char *xmlpath, const char *xmltag) {
    switch (xmltesttags.getId(xmltag)) {
      case XML_TEST_ROOT:
        m_inroot--;
        printf("  </WasabiTest>\n");
        fflush(stdout);
        break;
      case XML_TEST_TEST1:
        m_intest1--;
        printf("    </Test1>\n");
        fflush(stdout);
        break;
      case XML_TEST_TEST2:
        m_intest2--;
        printf("    />\n");
        fflush(stdout);
        break;
    }
  }
  virtual void xmlReaderOnCharacterDataCallback(const char *xmlpath, const char *xmltag, const char *str) {
    if (m_intest1) {
      printf("      Character Data reads \"%s\"\n", str);
      if (!STRCASEEQL(str, "success")) {
        m_failed = 1;
        fail("XML PARSER", "Not receiving the right character data in Test1");
      }
      fflush(stdout);
    }
  }
  int didFail() { return m_failed; }
private:
  int m_failed;
  int m_inroot;
  int m_intest1;
  int m_intest2;
};

#endif
//-------------------------------------------------------------------------------------------

// 03F73CE0-987D-46fd-B2E3-DBB364A47F54
static const GUID myappguid = { 0x3f73ce0, 0x987d, 0x46fd, { 0xb2, 0xe3, 0xdb, 0xb3, 0x64, 0xa4, 0x7f, 0x54 }};

int main(int argc, char **argv) {
  printf("-------------------------------------------------------------------------------\n");
  printf("Initializing Wasabi API.\n");
  printf("-------------------------------------------------------------------------------\n");
  fflush(stdout);

  ApiInit::init((HINSTANCE)0, myappguid, "", (HWND)NULL);

  printf("\n- PASSED -\n\n");
  fflush(stdout);

  // Rudimentary svcmgr test: enumerate services
  printf("-------------------------------------------------------------------------------\n");
  printf("Testing service manager.\n");
  printf("-------------------------------------------------------------------------------\n\n");
  int n=ServiceManager::getNumServicesByGuid();
  printf("Services running : %d\n\n", n);
  fflush(stdout);
  int i;
  for (i=0;i<n;i++) {
    waServiceFactory *factory = ServiceManager::enumService(i);
    char sguid[256];
    nsGUID::toChar(factory->getGuid(), sguid);
    printf("  %d : %s (%s)\n", i, factory->getServiceName(), sguid);
  }
  if (i > 0) {
  printf("\n- PASSED -\n\n");
  } else {
    fail("SVCMGR", "service enumerator returns no service present");
  }
  fflush(stdout);

#ifdef WASABI_COMPILE_TIMERS
  {
    DWORD ds = Std::getTickCount();
    // Multiplexed timers test
    printf("-------------------------------------------------------------------------------\n");
    printf("Testing timers for %d seconds.\n", TIMER_TEST_DURATION);
    printf("-------------------------------------------------------------------------------\n\n");
    fflush(stdout);
    TestTimer tt;
    doMessagePump();
    printf("\n");
    DWORD dt = Std::getTickCount() - ds;
    // let's assume this is a VERY VERY busy cpu, we're just trying to determine if the timers have been running...
    if (dt < (TIMER_TEST_DURATION-1)*1000 || dt > (TIMER_TEST_DURATION+1)*1000) {
      fail("TIMERS", "Test was out of range by more than 1s");
    } else {
      printf("\n- PASSED -\n\n");
    }
    fflush(stdout);
  }
#endif

#ifdef WASABI_COMPILE_XMLPARSER
  {
    // Multiplexed timers test
    printf("-------------------------------------------------------------------------------\n");
    printf("Testing XML parser.\n");
    printf("-------------------------------------------------------------------------------\n\n");
    fflush(stdout);
    XmlTest xt;
    if (!xt.didFail()) {
      printf("\n- PASSED -\n\n");
      fflush(stdout);
    } // else msg is already printed
  }
#endif

  printf("-------------------------------------------------------------------------------\n");
  printf("Shutting down.\n");
  printf("-------------------------------------------------------------------------------\n");

  ApiInit::shutdown();

  printf("\n- PASSED -\n\n");
  printf("===============================================================================\n");
  printf("Result : ");
  if (failed)
    printf("*** %d FAILURE%s ***\n", failed, failed > 1 ? "S" : "");
  else
    printf("- ALL PASSED -\n");
  printf("===============================================================================\n\n");

  return 0;
}

