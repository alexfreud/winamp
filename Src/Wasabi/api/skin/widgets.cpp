#include <precomp.h>
#include <api/skin/widgets.h>

#include <api/skin/widgets/group.h>

#ifdef WASABI_WIDGETS_LAYER
#include <api/skin/widgets/layer.h>
#endif

#ifdef WASABI_WIDGETS_ANIMLAYER
#include <api/skin/widgets/animlayer.h>
#endif

#ifdef WASABI_WIDGETS_BUTTON
#include <api/skin/widgets/button.h>
#endif

#ifdef WASABI_WIDGETS_TGBUTTON
#include <api/skin/widgets/tgbutton.h>
#endif

#ifdef WASABI_WIDGETS_GUIOBJECT
#include <api/script/objects/guiobj.h>
#endif

#ifdef WASABI_WIDGETS_GROUPLIST
#include <api/skin/widgets/grouplist.h>
#endif

#ifdef WASABI_WIDGETS_MOUSEREDIR
#include <api/skin/widgets/mouseredir.h>
#endif

#ifdef WASABI_WIDGETS_SLIDER
#include <api/skin/widgets/pslider.h>
#endif

#ifdef WASABI_WIDGETS_MEDIASLIDERS
#include <api/skin/widgets/seqband.h>
#include <api/skin/widgets/seqpreamp.h>
#include <api/skin/widgets/svolbar.h>
#include <api/skin/widgets/sseeker.h>
#include <api/skin/widgets/spanbar.h>
#endif

#ifdef WASABI_WIDGETS_MEDIAVIS
#include <api/skin/widgets/sa.h>
#endif

#ifdef WASABI_WIDGETS_MEDIAEQCURVE
#include <api/skin/widgets/seqvis.h>
#endif

#ifdef WASABI_WIDGETS_MEDIASTATUS
#include <api/skin/widgets/sstatus.h>
#endif

#ifdef _WIN32
#include <api/skin/widgets/wa2/xuiwa2slider.h>
#endif

#ifdef WASABI_WIDGETS_SVCWND
#include <api/skin/widgets/script/svcwnd.h>
#endif

#ifdef WASABI_WIDGETS_TEXT
#include <api/skin/widgets/text.h>
#endif

#ifdef WASABI_WIDGETS_EDIT
#include <api/skin/widgets/edit.h>
#endif

#ifdef WASABI_WIDGETS_TITLEBAR
#include <api/skin/widgets/title.h>
#endif

#ifdef WASABI_WIDGETS_COMPBUCK
#include <api/skin/widgets/compbuck2.h>
#endif

#ifdef WASABI_WIDGETS_BROWSER
#include <api/skin/widgets/mb/xuibrowser.h>
#ifdef WASABI_WIDGETS_BROWSERSVC
#include <api/skin/widgets/mb/iebrowser.h>
#include <api/skin/widgets/mb/mbsvc.h>
#endif
#endif

#ifdef WASABI_WIDGETS_FRAME
#include <api/skin/widgets/xuiframe.h>
#endif

#ifdef WASABI_WIDGETS_GRID
#include <api/skin/widgets/xuigrid.h>
#endif

#ifdef WASABI_WIDGETS_QUERYDRAG
#include <api/skin/widgets/xuiquerydrag.h>
#endif

#ifdef WASABI_WIDGETS_QUERYLIST
#include <api/skin/widgets/db/xuiquerylist.h>
#endif

#ifdef WASABI_WIDGETS_FILTERLIST
#include <api/skin/widgets/db/xuifilterlist.h>
#endif

#ifdef WASABI_WIDGETS_QUERYLINE
#include <api/skin/widgets/db/xuiqueryline.h>
#endif

#ifdef WASABI_WIDGETS_WNDHOLDER
#include <api/skin/widgets/xuiwndholder.h>
#endif

#ifdef WASABI_COMPILE_WNDMGR

#ifdef WASABI_WIDGETS_LAYOUTSTATUS
#include <api/skin/widgets/xuistatus.h>
#endif

#endif // wndmgr

#ifdef WASABI_WIDGETS_TABSHEET
#include <api/skin/widgets/xuitabsheet.h>
#endif

#ifdef WASABI_WIDGETS_CHECKBOX
#include <api/skin/widgets/xuicheckbox.h>
#endif

#ifdef WASABI_WIDGETS_TITLEBOX
#include <api/skin/widgets/xuititlebox.h>
#endif

#ifdef WASABI_WIDGETS_CUSTOMOBJECT
#include <api/skin/widgets/xuicustomobject.h>
#endif

#ifdef WASABI_WIDGETS_OSWNDHOST
#include <api/skin/widgets/xuioswndhost.h>
#endif

#ifdef WASABI_WIDGETS_RADIOGROUP
#include <api/skin/widgets/xuiradiogroup.h>
#endif

#ifdef WASABI_TOOLOBJECT_HIDEOBJECT
#include <api/skin/widgets/xuihideobject.h>
#endif

#ifdef WASABI_TOOLOBJECT_SENDPARAMS
#include <api/skin/widgets/xuisendparams.h>
#endif

#ifdef WASABI_TOOLOBJECT_ADDPARAMS
#include <api/skin/widgets/xuiaddparams.h>
#endif

#ifdef WASABI_WIDGETS_LIST
#include <api/skin/widgets/xuilist.h>
#endif

#ifdef WASABI_WIDGETS_TREE
#include <api/skin/widgets/xuitree.h>
#endif

#ifdef WASABI_WIDGETS_DROPDOWNLIST
#include <api/skin/widgets/xuidropdownlist.h>
#endif

#ifdef WASABI_WIDGETS_COMBOBOX
#include <api/skin/widgets/xuicombobox.h>
#endif

#ifdef WASABI_WIDGETS_HISTORYEDITBOX
#include <api/skin/widgets/xuihistoryedit.h>
#endif

#ifdef WASABI_WIDGETS_OBJECTDIRECTORY
#include <api/skin/widgets/xuiobjdirwnd.h>
#endif

#ifdef WASABI_WIDGETS_RECTANGLE
#include <api/skin/widgets/xuirect.h>
#endif

#ifdef WASABI_WIDGETS_PATHPICKER
#include <api/skin/widgets/xuipathpicker.h>
#endif

#ifdef WASABI_WIDGETS_GRADIENT
#include <api/skin/widgets/xuigradientwnd.h>
#endif

#ifdef WASABI_WIDGETS_MENU
#include <api/skin/widgets/xuimenu.h>
#endif

#include <api/skin/widgets/xuidownloadslist.h>

#ifdef WASABI_COMPILE_STATSWND
#include <api/skin/widgets/stats/xuistats.h>
#include <api/skin/widgets/stats/statswnd.h>
#endif

extern StringW g_resourcepath;

Widgets::Widgets() {
  count = 0;

  registerService(new XuiObjectCreator<GuiObjectXuiSvc>);

  #ifdef WASABI_WIDGETS_LAYER
    registerService(new XuiObjectCreator<LayerXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_ANIMLAYER
    registerService(new XuiObjectCreator<AnimLayerXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_BUTTON
    registerService(new XuiObjectCreator<ButtonXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_TGBUTTON
    registerService(new XuiObjectCreator<ToggleButtonXuiSvc>);
    registerService(new XuiObjectCreator<nStatesTgButtonXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_GROUPLIST
    registerService(new XuiObjectCreator<GroupListXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_MOUSEREDIR
    registerService(new XuiObjectCreator<MouseRedirXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_SLIDER
    registerService(new XuiObjectCreator<SliderXuiSvc>);
  #endif
#ifdef _WIN32
    registerService(new XuiObjectCreator<Wa2SliderXuiSvc>);
#endif
  #ifdef WASABI_WIDGETS_MEDIASLIDERS
    registerService(new XuiObjectCreator<EqBandXuiSvc>);
    registerService(new XuiObjectCreator<EqPreAmpXuiSvc>);
    registerService(new XuiObjectCreator<VolBarXuiSvc>);
    registerService(new XuiObjectCreator<SeekBarXuiSvc>);
    registerService(new XuiObjectCreator<PanBarXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_MEDIAVIS
    registerService(new XuiObjectCreator<VisXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_MEDIAEQCURVE
    registerService(new XuiObjectCreator<EqVisXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_MEDIASTATUS
    registerService(new XuiObjectCreator<StatusXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_SVCWND
    registerService(new XuiObjectCreator<SvcWndXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_TEXT
    registerService(new XuiObjectCreator<TextXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_EDIT
    registerService(new XuiObjectCreator<EditXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_TITLEBAR
    registerService(new XuiObjectCreator<TitleBarXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_COMPBUCK
    registerService(new XuiObjectCreator<ComponentBucketXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_BROWSER
    registerService(new XuiObjectCreator<BrowserXuiSvc>);
  #ifdef WASABI_WIDGETS_BROWSERSVC
    registerService(new waServiceFactoryT<svc_miniBrowser, MbSvc>);
  #endif
  #endif
  #ifdef WASABI_WIDGETS_FRAME
    registerService(new XuiObjectCreator<FrameXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_GRID
    registerService(new XuiObjectCreator<GridXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_QUERYDRAG
    registerService(new XuiObjectCreator<QueryDragXuiSvc>);
  #endif
  #ifdef WASABI_COMPILE_METADB
  #ifdef WASABI_WIDGETS_QUERYLIST
    registerService(new XuiObjectCreator<QueryListXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_FILTERLIST
    registerService(new XuiObjectCreator<FilterListXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_QUERYLINE
    registerService(new XuiObjectCreator<QueryLineXuiSvc>);
  #endif
  #endif // metadb
  #ifdef WASABI_WIDGETS_WNDHOLDER
    registerService(new XuiObjectCreator<WindowHolderXuiSvc>);
    registerService(new XuiObjectCreator<WindowHolderXuiSvc2>);
  #endif // components
  #ifdef WASABI_COMPILE_WNDMGR
  #ifdef WASABI_WIDGETS_LAYOUTSTATUS
    registerService(new XuiObjectCreator<LayoutStatusXuiSvc>);
  #endif
  #endif // wndmgr
  #ifdef WASABI_WIDGETS_TABSHEET
    registerService(new XuiObjectCreator<ScriptTabSheetXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_CHECKBOX
    registerService(new XuiObjectCreator<ScriptCheckBoxXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_TITLEBOX
    registerService(new XuiObjectCreator<ScriptTitleBoxXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_CUSTOMOBJECT
    registerService(new XuiObjectCreator<CustomObjectXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_OSWNDHOST
    registerService(new XuiObjectCreator<OSWndHostXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_RADIOGROUP
    registerService(new XuiObjectCreator<ScriptRadioGroupXuiSvc>);
  #endif
  #ifdef WASABI_TOOLOBJECT_HIDEOBJECT
    registerService(new XuiObjectCreator<HideObjectXuiSvc>);
  #endif
  #ifdef WASABI_TOOLOBJECT_SENDPARAMS
    registerService(new XuiObjectCreator<SendParamsXuiSvc>);
  #endif
  #ifdef WASABI_TOOLOBJECT_ADDPARAMS
    registerService(new XuiObjectCreator<AddParamsXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_LIST
    registerService(new XuiObjectCreator<ScriptListXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_TREE
    registerService(new XuiObjectCreator<ScriptTreeXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_DROPDOWNLIST
    registerService(new XuiObjectCreator<DropDownListXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_COMBOBOX
    registerService(new XuiObjectCreator<ComboBoxXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_HISTORYEDITBOX
    registerService(new XuiObjectCreator<HistoryEditXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_OBJECTDIRECTORY
    registerService(new XuiObjectCreator<ScriptObjDirWndXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_RECTANGLE
    registerService(new XuiObjectCreator<ScriptRectXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_PATHPICKER
    registerService(new XuiObjectCreator<PathPickerXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_GRADIENT
    registerService(new XuiObjectCreator<GradientWndXuiSvc>);
  #endif
  #ifdef WASABI_WIDGETS_MENU
    registerService(new XuiObjectCreator<MenuXuiSvc>);
  #endif

	//registerService(new XuiObjectCreator<DownloadsListXuiSvc>);

  #ifdef WASABI_COMPILE_WNDMGR
    //registerSkinFile("xml/msgbox/msgbox.xml");
  #endif

  #ifdef WASABI_WIDGETS_TOOLTIPS
    //registerSkinFile("xml/tooltips/tooltips.xml");
  #endif

  #ifdef WASABI_COMPILE_STATSWND
  registerService(new XuiObjectCreator<XuiStatsXuiSvc>);
  statswnd = new StatsWnd();
  #endif

  //loadResources();
  WASABI_API_SYSCB->syscb_registerCallback(static_cast<SysCallbackI *>(this));
}

Widgets::~Widgets() {
#ifdef WASABI_COMPILE_STATSWND
  delete statswnd;
#endif
  WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SysCallbackI *>(this));
  if (WASABI_API_SVC != NULL) 
	{
		int i=factories.getNumItems();
		while (i--)
			WASABI_API_SVC->service_deregister(factories[i]);
  }
  factories.deleteAll();
}


void Widgets::registerService(waServiceFactoryI *f) 
{
  WASABI_API_SVC->service_register(f);
  factories.addItem(f);
}

int Widgets::skincb_onBeforeLoadingElements() {
  if (count++ > 0) // if 0, we're already loaded so that the lib is usable without 'a skin'
    loadResources(); 
  return 1;
}

void Widgets::loadResources() 
{
	// TODO: benski> want to put this into gen_ff somewhere, instead.
	WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\winamp\\cover\\cover.xml"));
	WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\winamp\\thinger\\thinger.xml"));

  #ifndef WA3COMPATIBILITY // ifNdef
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\wasabi\\wasabi.xml"));
  #endif
  #ifdef WASABI_WIDGETS_PATHPICKER
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\pathpicker\\pathpicker.xml"));
  #endif
  #ifdef WASABI_WIDGETS_LAYOUTSTATUS
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\statusbar\\statusbar.xml"));
  #endif
  #ifdef WASABI_WIDGETS_TABSHEET
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\tabsheet\\tabsheet.xml"));
  #endif
  #ifdef WASABI_WIDGETS_CHECKBOX
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\checkbox\\checkbox.xml"));
  #endif
  #ifdef WASABI_WIDGETS_TITLEBOX
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\titlebox\\titlebox.xml"));
  #endif
  #ifdef WASABI_WIDGETS_DROPDOWNLIST
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\dropdownlist\\dropdownlist.xml"));
  #endif
  #ifdef WASABI_WIDGETS_COMBOBOX
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\combobox\\combobox.xml"));
  #endif
  #ifdef WASABI_WIDGETS_HISTORYEDITBOX
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\historyeditbox\\historyeditbox.xml"));
  #endif
  #ifdef WASABI_WIDGETS_TOOLTIPS
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\tooltips\\tooltips.xml"));
  #endif
  #ifdef WASABI_COMPILE_WNDMGR
  //WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,"xml\\msgbox\\msgbox.xml"));
  #endif
  WASABI_API_SKIN->loadSkinFile(StringPathCombine(g_resourcepath,L"xml\\about\\about.xml"));
}
