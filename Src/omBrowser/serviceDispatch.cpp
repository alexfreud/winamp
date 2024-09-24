#include "./main.h"
#include "./service.h"

#define CBCLASS OmService
START_MULTIPATCH;
 START_PATCH(MPIID_OMSVC)
  M_CB(MPIID_OMSVC, ifc_omservice, ADDREF, AddRef);
  M_CB(MPIID_OMSVC, ifc_omservice, RELEASE, Release);
  M_CB(MPIID_OMSVC, ifc_omservice, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETID, GetId);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETNAME, GetName);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETURL, GetUrl);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETURLDIRECT, GetUrlDirect);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETICON, GetIcon);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETEXTERNAL, GetExternal);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETRATING, GetRating);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETVERSION, GetVersion);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETGENERATION, GetGeneration);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETFLAGS, GetFlags);
  M_CB(MPIID_OMSVC, ifc_omservice, API_SETADDRESS, SetAddress);
  M_CB(MPIID_OMSVC, ifc_omservice, API_GETADDRESS, GetAddress);
  M_CB(MPIID_OMSVC, ifc_omservice, API_UPDATEFLAGS, UpdateFlags);
   
 NEXT_PATCH(MPIID_OMSVCDETAILS)
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, ADDREF, AddRef);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, RELEASE, Release);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, API_GETDESCRIPTION, GetDescription);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, API_GETAUTHORFIRST, GetAuthorFirst);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, API_GETAUTHORLAST, GetAuthorLast);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, API_GETUPDATED, GetUpdated);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, API_GETPUBLISHED, GetPublished);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, API_GETTHUMBNAIL, GetThumbnail);
  M_CB(MPIID_OMSVCDETAILS, ifc_omservicedetails, API_GETSCREENSHOT, GetScreenshot);
 
 NEXT_PATCH(MPIID_OMSVCEDITOR)
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, ADDREF, AddRef);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, RELEASE, Release);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETNAME, SetName);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETURL, SetUrl);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETICON, SetIcon);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETRATING, SetRating);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETVERSION, SetVersion);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETGENERATION, SetGeneration);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETFLAGS, SetFlags);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETDESCRIPTION, SetDescription);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETAUTHORFIRST, SetAuthorFirst);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETAUTHORLAST, SetAuthorLast);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETUPDATED, SetUpdated);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETPUBLISHED, SetPublished);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETTHUMBNAIL, SetThumbnail);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETSCREENSHOT, SetScreenshot);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_SETMODIFIED, SetModified);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_GETMODIFIED, GetModified);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_BEGINUPDATE, BeginUpdate);
  M_CB(MPIID_OMSVCEDITOR, ifc_omserviceeditor, API_ENDUPDATE, EndUpdate);
   
 NEXT_PATCH(MPIID_OMSVCCOPIER)
  M_CB(MPIID_OMSVCCOPIER, ifc_omservicecopier, ADDREF, AddRef);
  M_CB(MPIID_OMSVCCOPIER, ifc_omservicecopier, RELEASE, Release);
  M_CB(MPIID_OMSVCCOPIER, ifc_omservicecopier, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSVCCOPIER, ifc_omservicecopier, API_COPYTO, CopyTo);

 NEXT_PATCH(MPIID_OMSVCCOMMAND)
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, ADDREF, AddRef);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, RELEASE, Release);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, API_QUERYSTATE, QueryState);
  M_CB(MPIID_OMSVCCOMMAND, ifc_omservicecommand, API_EXEC, Exec);

 NEXT_PATCH(MPIID_OMSVCEVENTMNGR)
  M_CB(MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr, ADDREF, AddRef);
  M_CB(MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr, RELEASE, Release);
  M_CB(MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr, API_REGISTERHANDLER, RegisterEventHandler);
  M_CB(MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr, API_UNREGISTERHANDLER, UnregisterEventHandler);
  M_CB(MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr, API_SIGNAL_SERVICECHANGE, Signal_ServiceChange);
  M_CB(MPIID_OMSVCEVENTMNGR, ifc_omserviceeventmngr, API_SIGNAL_COMMANDSTATECHANGE, Signal_CommandStateChange);
 
 NEXT_PATCH(MPIID_OMSVCHOSTEXT)
  M_CB(MPIID_OMSVCHOSTEXT, ifc_omservicehostext, API_GETHOST, GetHost);
  M_CB(MPIID_OMSVCHOSTEXT, ifc_omservicehostext, API_SETHOST, SetHost);

 END_PATCH
END_MULTIPATCH;
#undef CBCLASS