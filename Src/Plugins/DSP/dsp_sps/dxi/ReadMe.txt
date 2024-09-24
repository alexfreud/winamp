The Cakewalk DirectX Plug-In Wizard has created
the following files for you:

AudioPlugIn.h, AudioPlugIn.cpp:
    CAudioPlugIn, the DirectX plug-in object.

AudioPlugInPropPage.h, AudioPlugInPropPage.cpp:
    CAudioPlugInPropPage, an class that implements the plug-in's
    property page (IPropertyPage).

AudioPlugIn.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Developer Studio.

res\AudioPlugIn.rc2
    This file contains resources that are not edited by Microsoft 
    Developer Studio.  You should place all resources not
    editable by the resource editor in this file.

AudioPlugIn.def
    This file contains information about the DLL that must be
    provided to run with Microsoft Windows.  It defines parameters
    such as the name and description of the DLL.  It also exports
	functions from the DLL.

AudioPlugIn.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

///////////////////////////////////////////////////////////
Support files:

AudioPlugInApp.h
AudioPlugInApp.cpp:
    Entry points for component registration and deregistration.

MediaParams.h
MediaParams.cpp:
    CMediaParams, a helper class to implement all pertinent DirectX automation
    intefaces, such as IMediaParams and IMediaParamsInfo.

ParamEnvelope.h
ParamEnvelope.cpp:
    CParamEnvelope, a container for a single parameter's envelope, i.e., its
    evolving shape over time.  CMediaParams keeps a collection of these.

///////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named AudioPlugIn.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
