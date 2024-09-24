#include <precomp.h>
#include "wa2buckitems.h"
#include "wa2frontend.h"
#include "wa2wndembed.h"
#include "gen.h"
#include "../Agave/Language/api_language.h"


//-------------------------------------------------------------------------------------------
Wa2BucketItem::Wa2BucketItem(GUID g, const wchar_t *t) : BucketItemT<ButtonWnd> (g, t) {
}

Wa2BucketItem::~Wa2BucketItem() {
}

//-------------------------------------------------------------------------------------------

PlBucketItem::PlBucketItem() : Wa2BucketItem(INVALID_GUID, L"Playlist Editor") {
}

PlBucketItem::~PlBucketItem() {
}

void PlBucketItem::onLeftPush(int x, int y) {
  BUCKETITEM_PARENT::onLeftPush(x, y);
  wa2.setWindowVisible(IPC_GETWND_PE, !wa2.isWindowVisible(IPC_GETWND_PE));
}


//-------------------------------------------------------------------------------------------
EmbedBucketItem::EmbedBucketItem() : Wa2BucketItem(INVALID_GUID, L"Embed?") {
}

EmbedBucketItem::~EmbedBucketItem() {
}

void EmbedBucketItem::onLeftPush(int x, int y) {
  BUCKETITEM_PARENT::onLeftPush(x, y);
  //wa2.setWindowVisible(IPC_GETWND_PE, !wa2.isWindowVisible(IPC_GETWND_PE));
}

//-------------------------------------------------------------------------------------------
#ifdef MINIBROWSER_SUPPORT

MbBucketItem::MbBucketItem() : Wa2BucketItem(INVALID_GUID, L"Minibrowser") {
}

MbBucketItem::~MbBucketItem() {
}

void MbBucketItem::onLeftPush(int x, int y) {
  BUCKETITEM_PARENT::onLeftPush(x, y);
  wa2.setWindowVisible(IPC_GETWND_MB, !wa2.isWindowVisible(IPC_GETWND_MB));
}
#endif

//-------------------------------------------------------------------------------------------
VidBucketItem::VidBucketItem() : Wa2BucketItem(INVALID_GUID, L"Video Window") {
}

VidBucketItem::~VidBucketItem() {
}

//-------------------------------------------------------------------------------------------
void VidBucketItem::onLeftPush(int x, int y) {
  BUCKETITEM_PARENT::onLeftPush(x, y);
  wa2.setWindowVisible(IPC_GETWND_VIDEO, !wa2.isWindowVisible(IPC_GETWND_VIDEO));
}

//-------------------------------------------------------------------------------------------

MlBucketItem::MlBucketItem() : Wa2BucketItem(INVALID_GUID, L"Winamp Library") {
}

MlBucketItem::~MlBucketItem() {
}

#define WA_MEDIALIB_MENUITEM_ID 23123

void MlBucketItem::onLeftPush(int x, int y) {
  BUCKETITEM_PARENT::onLeftPush(x, y);
  SendMessageW(wa2.getMainWindow(), WM_COMMAND, WA_MEDIALIB_MENUITEM_ID, 0);
}

//-------------------------------------------------------------------------------------------

VisBucketItem::VisBucketItem() : Wa2BucketItem(INVALID_GUID, L"Visualizations") {
}

VisBucketItem::~VisBucketItem() {
}

void VisBucketItem::onLeftPush(int x, int y) {
  BUCKETITEM_PARENT::onLeftPush(x, y);
  wa2.toggleVis();
}