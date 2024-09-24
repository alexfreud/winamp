#include <precomp.h>

#include "coreactions.h"

#include <api/config/items/cfgitem.h>
#include <api/core/buttons.h>
#include <api/api.h>
#include <api/core/corehandle.h>
#include <api/service/svcs/svc_player.h>
#include <api/locales/xlatstr.h>

CoreActions::CoreActions() {
  registerAction("prev", ACTION_PREV);
  registerAction("play", ACTION_PLAY);
  registerAction("pause", ACTION_PAUSE);
  registerAction("stop", ACTION_STOP);
  registerAction("next", ACTION_NEXT);
  registerAction("eject", ACTION_EJECT);
  registerAction("eject_url", ACTION_EJECTURL);
  registerAction("eject_dir", ACTION_EJECTDIR);
  registerAction("seek", ACTION_SEEK);
  registerAction("volume", ACTION_VOLUME);
  registerAction("pan", ACTION_PAN);
  registerAction("volume_up", ACTION_VOLUME_UP);
  registerAction("volume_down", ACTION_VOLUME_DOWN);
  registerAction("rewind_5s", ACTION_REWIND_5S);
  registerAction("ffwd_5s", ACTION_FFWD_5S);
  registerAction("toggle_repeat", ACTION_TOGGLE_REPEAT);
  registerAction("toggle_shuffle", ACTION_TOGGLE_SHUFFLE);
  registerAction("toggle_crossfader", ACTION_TOGGLE_CROSSFADER);
  registerAction("mute", ACTION_MUTE);
  registerAction("eq_preamp", ACTION_EQ_PREAMP);
  registerAction("eq_band", ACTION_EQ_BAND);
  registerAction("eq_auto", ACTION_EQ_AUTO);
  registerAction("eq_reset", ACTION_EQ_RESET);
  registerAction("toggle_repeat", ACTION_TOGGLE_REPEAT);
  registerAction("toggle_shuffle", ACTION_TOGGLE_SHUFFLE);
  registerAction("toggle_crossfader", ACTION_TOGGLE_CROSSFADER);
  registerAction("eq_toggle", ACTION_EQ_TOGGLE);
  for (int i=0;i<4;i++)
    registerAction(StringPrintf("play_cd%d", i+1), ACTION_PLAY_CD+i);
}

CoreActions::~CoreActions() {
}

int CoreActions::onActionId(int pvtid, const char *action, const char *param/* =NULL */, int p1/* =0 */, int p2/* =0 */, void *data/* =NULL */, int datalen/* =0 */, api_window *source/* =NULL */) {
  int d = ATOI(param);
  CoreHandle ch("main");
  switch(pvtid) {
    case ACTION_PREV: { if (d==0) ch.prev(); } break;
    case ACTION_PLAY: { if (d==0) ch.play(); } break;
    case ACTION_PAUSE: { if (d==0) ch.pause(); } break;
    case ACTION_STOP: { if (d==0) ch.stop(); } break;
    case ACTION_NEXT: { if (d==0) ch.next(); } break;

    case ACTION_EJECT: {
        svc_player *sp = SvcEnumByGuid<svc_player>();
        if (d == 0) {
          if (sp) sp->openFiles(source, "files");
        } else {
          if (sp) sp->openFiles(source);
        }
        WASABI_API_SVC->service_release(sp);
    }
    break;
    case ACTION_EJECTURL: if (d==0) {
      svc_player *sp = SvcEnumByGuid<svc_player>();
      if (sp) sp->openFiles(source, "location");
      api->service_release(sp);
    }
    break;
    case ACTION_EJECTDIR: if (d==0) {
      svc_player *sp = SvcEnumByGuid<svc_player>();
      if (sp) sp->openFiles(source, "directory");
      api->service_release(sp);
    }
    break;

    case ACTION_VOLUME_UP: if (d==0) {
      int v=ch.getVolume();
      ch.setVolume(MIN(255,(v+5)));
    }
    break;

    case ACTION_VOLUME_DOWN: if (d==0) {
      int v=ch.getVolume();
      ch.setVolume(MAX(0,(v-5))); 
    }
    break;

    case ACTION_REWIND_5S: if (d==0) {
      int p=ch.getPosition();
      ch.setPosition(MAX(0,(p-5000)));
    }
    break;

    case ACTION_FFWD_5S: if (d==0) {
      int p=ch.getPosition();
      int mp=ch.getLength();
      ch.setPosition(MIN(mp,(p+5000)));
    }
    break;

    case ACTION_EQ_AUTO: 
      if (d==0) ch.setEqAuto(!ch.getEqAuto());
    break;
      
    case ACTION_EQ_RESET: {
      if (d==0) for(int i=0;i<10;i++) ch.setEqBand(i,0);
    }
    break;
    
    case ACTION_EQ_TOGGLE: if (d==0) ch.setEqStatus(!ch.getEqStatus()); break;

    case ACTION_MUTE: if (d==0) {
      ch.setMute(!ch.getMute());;
    }
    break;

    case ACTION_TOGGLE_REPEAT:
    case ACTION_TOGGLE_SHUFFLE: if (d==0) {
      // {45F3F7C1-A6F3-4ee6-A15E-125E92FC3F8D}
      const GUID pledit_guid = 
      { 0x45f3f7c1, 0xa6f3, 0x4ee6, { 0xa1, 0x5e, 0x12, 0x5e, 0x92, 0xfc, 0x3f, 0x8d } };
      CfgItem *pli=WASABI_API_CONFIG->config_getCfgItemByGuid(pledit_guid);
      if(pli) {
        if(pvtid==ACTION_TOGGLE_REPEAT) pli->setDataAsInt("Repeat",!pli->getDataAsInt("Repeat"));
        if(pvtid==ACTION_TOGGLE_SHUFFLE) pli->setDataAsInt("Shuffle",!pli->getDataAsInt("Shuffle"));
      }
    }
    break;

    case ACTION_TOGGLE_CROSSFADER: if (d==0) {
      // {FC3EAF78-C66E-4ED2-A0AA-1494DFCC13FF}
      const GUID xfade_guid = 
      { 0xFC3EAF78, 0xC66E, 0x4ED2, { 0xA0, 0xAA, 0x14, 0x94, 0xDF, 0xCC, 0x13, 0xFF } };
      CfgItem *pli=WASABI_API_CONFIG->config_getCfgItemByGuid(xfade_guid);
      if(pli) pli->setDataAsInt("Enable crossfading",!pli->getDataAsInt("Enable crossfading"));
    }
    break;
  }
  if (pvtid >= ACTION_PLAY_CD && pvtid < ACTION_PLAY_CD+16) if (d==0) {
    const GUID cdda_guid = 
    { 0x86b40069, 0x126f, 0x4e11, { 0xa8, 0x7f, 0x55, 0x8f, 0xfa, 0x3d, 0xff, 0xa8 } };
#if 0//BU: need some api to send messages like this
    ComponentManager::sendNotify(cdda_guid, 12345, pvtid-ACTION_PLAY_CD);
#endif
  }
  return 1;
}

const char *CoreActions::getHelp(int action) {
  static String name;
  switch (action) {
    case ACTION_PREV: name = _("Previous track"); break;
    case ACTION_PAUSE: name = _("Pause/Resume playback"); break;
    case ACTION_STOP: name = _("Stop playback"); break;
    case ACTION_NEXT: name = _("Next track"); break;
    case ACTION_EJECT: name = _("Load file"); break;
    case ACTION_EJECTURL: name = _("Load URL"); break;
    case ACTION_EJECTDIR: name = _("Load directory"); break;
    case ACTION_SEEK: name = _("Seek"); break;
    case ACTION_VOLUME: name = _("Volume"); break;
    case ACTION_PAN: name = _("Panning"); break;
    case ACTION_EQ_TOGGLE: name = _("Toggle equalizer"); break;
    case ACTION_EQ_PREAMP: name = _("Toggle preamplifier"); break;
    case ACTION_EQ_BAND: name = _("Change equalizer band"); break;
    case ACTION_EQ_AUTO: name = _("Auto equalizer"); break;
    case ACTION_EQ_RESET: name = _("Reset equalizer"); break;
    case ACTION_VOLUME_UP: name = _("Volume up"); break;
    case ACTION_VOLUME_DOWN: name = _("Volume down"); break;
    case ACTION_REWIND_5S: name = _("Rewind 5 seconds"); break;
    case ACTION_FFWD_5S: name = _("Fast forward 5 seconds"); break;
    case ACTION_MUTE: name = _("Mute/Unmute sound"); break;
    case ACTION_TOGGLE_REPEAT: name = _("Toggle repeat"); break;
    case ACTION_TOGGLE_SHUFFLE: name = _("Toggle shuffle"); break;
    case ACTION_TOGGLE_CROSSFADER: name = _("Toggle crossfader"); break;
  }
  return name;
}
