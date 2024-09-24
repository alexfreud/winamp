#ifndef _ANIMLAYER_H
#define _ANIMLAYER_H

#include "layer.h"

// {6B64CD27-5A26-4c4b-8C59-E6A70CF6493A}
static const GUID animLayerGuid = 
{ 0x6b64cd27, 0x5a26, 0x4c4b, { 0x8c, 0x59, 0xe6, 0xa7, 0xc, 0xf6, 0x49, 0x3a } };

#define ANIMLAYER_SCRIPTPARENT Layer

class AnimLayerScriptController : public LayerScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return layerController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern AnimLayerScriptController *animlayerController;


#ifndef _NOSTUDIO

#define TIMER_ANIM 872

#define ANIM_STOPPED 0
#define ANIM_PLAYING 1
#define ANIM_PAUSED 2

#define ANIMLAYER_PARENT Layer

#define ANIM_UNKNOWN 0
#define ANIM_VERT    1
#define ANIM_HORZ    2
#define ANIM_MULTI   3

class AnimatedLayer : public ANIMLAYER_SCRIPTPARENT {
public:
  AnimatedLayer();
	virtual ~AnimatedLayer();

  virtual int onInit();
	virtual int getHeight();
	virtual int getWidth();
	virtual void timerCallback(int id);
  virtual int getSourceOffsetY();
  virtual int getSourceOffsetX();
  virtual void setAutoPlay(int p);
  virtual int setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
  
	virtual void setHeight(int h, int selfset=0);
	virtual void setWidth(int w, int selfset=0);
  virtual SkinBitmap *getBitmap();

  void play();
  void pause();
  void stop();
  int getLength();
  void setStartFrame(int s);
  void setEndFrame(int e);
  void setAutoReplay(int r);
  int getStartFrame();
  int getEndFrame();
  int isPlaying();
  int isPaused();
  int isStopped();
  int getSpeed();
  int getDirection();
  void gotoFrame(int n);
  void setSpeed(int s);
  int getAutoReplay();
  int getCurFrame();
  void setRealtime(int r);

  virtual api_region *getBitmapRegion();
  SkinBitmap *getElementBitmap(int n);
  virtual void makeRegion();
  virtual void deleteRegion();
  void reloadMultipleElements();
  virtual void setElementFrames(int n);
  virtual void setBitmap(const wchar_t *name);

  virtual int onPaint(Canvas *canvas);

protected:
	/*static */void CreateXMLParameters(int master_handle);
  enum {
    ANIMLAYER_AUTOREPLAY=0,
    ANIMLAYER_AUTOPLAY,
    ANIMLAYER_SPEED,
    ANIMLAYER_FRAMEHEIGHT,
    ANIMLAYER_FRAMEWIDTH,
    ANIMLAYER_REALTIME,
    ANIMLAYER_ELEMENTFRAMES,
    ANIMLAYER_START,
    ANIMLAYER_END,
    ANIMLAYER_DEBUG,
  };

private:
  int frameHeight, frameWidth;
  int startframe;
  int endframe;
  int status;
  int curframe;
  int autoreplay;
  int speed;
  int timerset;
  int realtime;
  int style;
  int autoplay;
  PtrList<SkinBitmap> bitmap_elements;
  int multiple_elements_frames;
  int debug;
  int xuihandle;

  void _invalidate();

  void stopTimer();
  void startTimer();

  PtrList<RegionI> regionlist;
  int oldstyle;
	static XMLParamPair params[];

// FG>
// -- SCRIPT -----------------------------------------------------

public:

  // virtuals

  virtual void script_play();
  virtual void script_pause();
  virtual void script_stop();
  virtual int script_getLength();
  virtual void script_setStartFrame(int s);
  virtual void script_setEndFrame(int e);
  virtual void script_setAutoReplay(int r);
  virtual void script_setSpeed(int a);
  virtual int script_getStartFrame();
  virtual int script_getEndFrame();
  virtual int script_getSpeed();
  virtual int script_getDirection();
  virtual int script_getAutoReplay();
  virtual int script_isPlaying();
  virtual int script_isStopped();
  virtual int script_isPaused();
  //virtual void script_gotoFrame(int f);
  virtual void script_onFrame(int n);
  virtual void script_onStop();
  virtual void script_onPlay();
  virtual void script_onPause();
  virtual void script_onResume();
  virtual int script_getCurFrame();
  virtual void script_setRealtime(int r);

  static scriptVar script_vcpu_setSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s);
  static scriptVar script_vcpu_gotoFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar f);
  static scriptVar script_vcpu_setStartFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar s);
  static scriptVar script_vcpu_setEndFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar e);
  static scriptVar script_vcpu_setAutoReplay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar ar);
  static scriptVar script_vcpu_play(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_pause(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_stop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  static scriptVar script_vcpu_isPlaying(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_isPaused(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_isStopped(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getStartFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getEndFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getLength(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getDirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getAutoReplay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getSpeed(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getCurFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_setRealtime(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r);
  static scriptVar script_vcpu_onPlay(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onStop(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onPause(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onResume(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onFrame(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar f);


#else
class AnimatedLayer : public ANIMLAYER_SCRIPTPARENT {

public:

#endif

//  INSERT_SCRIPT_OBJECT_CONTROL

};

extern const wchar_t animLayerXuiObjectStr[];
extern char animLayerXuiSvcName[];
class AnimLayerXuiSvc : public XuiObjectSvc<AnimatedLayer, animLayerXuiObjectStr, animLayerXuiSvcName> {};


#endif
