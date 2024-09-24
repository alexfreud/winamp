#ifndef __WA2BUCKETITEMS_H
#define __WA2BUCKETITEMS_H

#include <api/wnd/bucketitem.h>

#define WA2BUCKETITEM_PARENT Wa2BucketItem

//-------------------------------------------------------------------------------------------
class Wa2BucketItem : public BucketItemT<ButtonWnd> {
  public:
    Wa2BucketItem(GUID g=INVALID_GUID, const wchar_t *text=NULL);
    virtual ~Wa2BucketItem();
};

//-------------------------------------------------------------------------------------------
class PlBucketItem : public Wa2BucketItem {
  public:
    PlBucketItem();
    virtual ~PlBucketItem();
    void onLeftPush(int x, int y);
};

//-------------------------------------------------------------------------------------------
class EmbedBucketItem : public Wa2BucketItem {
  public:
    EmbedBucketItem();
    virtual ~EmbedBucketItem();
    void onLeftPush(int x, int y);
};

#ifdef MINIBROWSER_SUPPORT

//-------------------------------------------------------------------------------------------
class MbBucketItem : public Wa2BucketItem {
  public:
    MbBucketItem();
    virtual ~MbBucketItem();
    void onLeftPush(int x, int y);
};

#endif

//-------------------------------------------------------------------------------------------
class VidBucketItem : public Wa2BucketItem {
  public:
    VidBucketItem();
    virtual ~VidBucketItem();
    void onLeftPush(int x, int y);
};

//-------------------------------------------------------------------------------------------
class VisBucketItem : public Wa2BucketItem {
  public:
    VisBucketItem();
    virtual ~VisBucketItem();
    void onLeftPush(int x, int y);
};

//-------------------------------------------------------------------------------------------
class MlBucketItem : public Wa2BucketItem {
  public:
    MlBucketItem();
    virtual ~MlBucketItem();
    void onLeftPush(int x, int y);
};




#endif