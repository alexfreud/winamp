#include "precomp.h"
#include "sharedminibrowser.h"
#include "wnds/skinwnd.h"
#include "../studio/api.h"
#include "../common/mainminibrowser.h"

void SharedMiniBrowser::navigateUrl(const char *url) {
  if (!m_monitor) {
    m_monitor = new SkinMonitor();
  }

  if (!MainMiniBrowser::getScriptObject()) {

    if (!m_inserted) {
      String xml = "buf:\n";
      xml += "<WinampAbstractionLayer>\n";
      xml += "  <groupdef id=\"addon.shared.minibrowser\" name=\"MiniBrowser\">\n";
      xml += "    <browser mainmb=\"1\" x=\"0\" y=\"0\" w=\"0\" h=\"0\" relatw=\"1\" relath=\"1\" />\n";
      xml += "  </groupdef>\n";
      xml += "</WinampAbstractionLayer>\n";
      WASABI_API_SKIN->loadSkinFile(xml);
      m_inserted = 1;
    }

    SkinWnd("addon.shared.minibrowser", WASABISTDCONTAINER_RESIZABLE_NOSTATUS);
    ASSERTPR(MainMiniBrowser::getScriptObject() != NULL, "Something is really wrong with wasabi");
  }

  MainMiniBrowser::navigateUrl(url);
  MainMiniBrowser::popMb();
}

void SharedMiniBrowser::shutdown() {
  if (m_monitor) delete m_monitor;
  m_monitor = NULL;
}

int SharedMiniBrowser::m_inserted = 0;
SkinMonitor *SharedMiniBrowser::m_monitor = NULL;