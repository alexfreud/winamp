#include <precomp.h>

#include "corehandle.h"

#include <api/api.h>
#include <api/core/buttons.h>
#include <api/service/svcs/svc_coreadmin.h>
#include <api/service/svc_enum.h> // for castService

using namespace UserButton;

static svc_coreAdmin *da_coreadmin=NULL;
static int instancecount=0;

static void initDaCoreAdmin() {
  if(!da_coreadmin) {
    if (WASABI_API_SVC != NULL) {
      waServiceFactory *s=WASABI_API_SVC->service_enumService(WaSvc::COREADMIN,0);
      if (s != NULL) {
//CUT    ASSERTPR(s,"Core Admin non present!");
        da_coreadmin=castService<svc_coreAdmin>(s);
      }
    }
  }
}

CoreHandle::CoreHandle(CoreToken tok) {
  initDaCoreAdmin();
  instancecount++;
  createdcore=0;
  token = NO_CORE_TOKEN;
  if (da_coreadmin) {
    if(da_coreadmin->verifyToken(tok)) token=tok;
    else {
      token=da_coreadmin->createCore();
      createdcore=1;
    }
  }
}

CoreHandle::CoreHandle(const char *name) {
  initDaCoreAdmin();
  instancecount++;
  createdcore=0;
  token = NO_CORE_TOKEN;
  if (da_coreadmin) {
    token=da_coreadmin->nameToToken(name);
    if(token==-1) {
      token=da_coreadmin->createCore(name);
      createdcore=1;
    }
  }
}

CoreHandle::~CoreHandle() {
  instancecount--;
  if (da_coreadmin) {
    if(createdcore) da_coreadmin->freeCoreByToken(token);
    if(!instancecount) {
      WASABI_API_SVC->service_release(da_coreadmin);
      da_coreadmin=NULL;
    }
  }
}

int CoreHandle::isCoreLoaded() {
  if (!da_coreadmin) return FALSE;
  return TRUE;
}

int CoreHandle::setNextFile(const char *playstring, const char *destination) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->setNextFile(token, playstring, destination);
}

void CoreHandle::prev() {
  userButton(PREV);
}

void CoreHandle::play() {
  userButton(PLAY);
}

void CoreHandle::pause() {
  userButton(PAUSE);
}

void CoreHandle::stop() {
  userButton(STOP);
}

void CoreHandle::next() {
  userButton(NEXT);
}

void CoreHandle::userButton(int button) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->userButton(token, button);
}

const char *CoreHandle::getSupportedExtensions() {
  if (da_coreadmin == NULL) return "";
  return da_coreadmin->getSupportedExtensions();
}

const char *CoreHandle::getExtSupportedExtensions() {
  if (da_coreadmin == NULL) return "";
  return da_coreadmin->getExtSupportedExtensions();
}

const char *CoreHandle::getExtensionFamily(const char *extension) {
  if (da_coreadmin == NULL) return "";
  return da_coreadmin->getExtensionFamily(extension);
}

void CoreHandle::registerExtension(const char *extensions, const char *extension_name) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->registerExtension(extensions, extension_name);
}

void CoreHandle::unregisterExtension(const char *extensions) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->unregisterExtension(extensions);
}

String CoreHandle::getTitle() {
  if (da_coreadmin == NULL) return String("");
  return String(da_coreadmin->getTitle(token));
}

int CoreHandle::getStatus() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getStatus(token);
}

const char *CoreHandle::getCurrent() {
  if (da_coreadmin == NULL) return "";
  return da_coreadmin->getCurrent(token);
}

int CoreHandle::getNumTracks() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getNumTracks(token);
}

int CoreHandle::getCurPlaybackNumber() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getCurPlaybackNumber(token);
}

int CoreHandle::getPosition() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getPosition(token);
}

int CoreHandle::getWritePosition() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getWritePosition(token);
}

int CoreHandle::setPosition(int ms) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->setPosition(token,ms);
}

int CoreHandle::getLength() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getLength(token);
}

int CoreHandle::getPluginData(const char *playstring, const char *name, char *data, int data_len, int data_type) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getPluginData(playstring, name, data, data_len, data_type);
}

unsigned int CoreHandle::getVolume() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getVolume(token);
}

void CoreHandle::setVolume(unsigned int vol) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setVolume(token, vol);
}

int CoreHandle::getPan() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getPan(token);
}

void CoreHandle::setPan(int bal) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setPan(token, bal);
}

void CoreHandle::setMute(int mute) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setMute(token, mute);
}

int CoreHandle::getMute() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getMute(token);
}

void CoreHandle::addCallback(CoreCallback *cb) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->addCallback(token, cb);
}

void CoreHandle::delCallback(CoreCallback *cb) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->delCallback(token, cb);
}

int CoreHandle::getVisData(void *dataptr, int sizedataptr) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getVisData(token, dataptr, sizedataptr);
}

int CoreHandle::getLeftVuMeter() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getLeftVuMeter(token);
}

int CoreHandle::getRightVuMeter() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getRightVuMeter(token);
}
  
int CoreHandle::registerSequencer(ItemSequencer *seq) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->registerSequencer(token, seq);
}

int CoreHandle::deregisterSequencer(ItemSequencer *seq) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->deregisterSequencer(token, seq);
}

ItemSequencer *CoreHandle::getSequencer() {
  if (da_coreadmin == NULL) return NULL;
  return da_coreadmin->getSequencer(token);
}

int CoreHandle::getEqStatus() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getEqStatus(token);
}

void CoreHandle::setEqStatus(int enable) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setEqStatus(token, enable);
}

int CoreHandle::getEqPreamp() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getEqPreamp(token);
}

void CoreHandle::setEqPreamp(int pre) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setEqPreamp(token, pre);
}

int CoreHandle::getEqBand(int band) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getEqBand(token, band);
}

void CoreHandle::setEqBand(int band, int val) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setEqBand(token, band, val);
}

int CoreHandle::getEqAuto() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getEqAuto(token);
}

void CoreHandle::setEqAuto(int enable) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setEqAuto(token, enable);
}

void CoreHandle::setPriority(int priority) {
  if (da_coreadmin == NULL) return;
  da_coreadmin->setPriority(token, priority);
}

int CoreHandle::getPriority() {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->getPriority(token);
}

void CoreHandle::rebuildConvertersChain() {
  if (da_coreadmin == NULL) return;
  da_coreadmin->rebuildConvertersChain(token);
}

int CoreHandle::sendConvertersMsg(const char *msg, const char *value) {
  if (da_coreadmin == NULL) return 0;
  return da_coreadmin->sendConvertersMsg(token, msg, value);
}
