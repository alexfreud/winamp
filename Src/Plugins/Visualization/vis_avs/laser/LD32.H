/*
  LICENSE
  -------
Copyright 2005 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#define LDDLL_PREFIX
/*************************************************************************
*   Helpful Stuff							 *
**************************************************************************/

/*
This header file defines a prefix. This is used by C++ programs to append a prefix to the definitions.
For C programs that use the LD2000.H file and associated library, use the following
	#define LDDLL_PREFIX		//Define the prefix in LD2000.H file as nothing
	#include "ld2000.h"
For C++ programs, use the following
	#define LDDLL_PREFIX extern "C"		//Define the prefix in LD2000.H file as extern "C" to indicate that the names in the lib file have not been mangled
	#include "ld2000.h"
*/

/*************************************************************************
*   Error Codes 							 *
**************************************************************************/

#define LDERR_OK		    0	    //Normal Return value

//********** These can occur at any time except InitialQMCheck ****
#define LDERR_FIFO_READ_ERROR	    -1	    //Fifo read zero or non even number of bytes
#define LDERR_QM32_SOFTWARE_ERROR   -2	    //QM32 is not responding properly


//********** These are reported by the loaders including LoadPalette
#define LDERR_X29_LOADED	    -11     //File load Successful Return
#define LDERR_LDA_LOADED	    -12	    //values
#define LDERR_ILDA_LOADED	    -13
#define LDERR_DEC3_LOADED	    -14
#define LDERR_LDB_LOADED	    -15
#define LDERR_LDSECURE_LOADED	-16


//********* Returned by ConvertToPointFrame **************
#define LDERR_ALREADY_POINT_ORIENTED -31    //Incase somebody tries to
					    //create a point oriented frame
					    //from a point oriented frame

//******** Currently only returned by CheckSession **************
#define LDERR_NO_SESSION_IN_PROG    -101    //CheckSession return value

//******** Returned by LFileRequest and LPaletteRequest *********
#define LDERR_FILE_REQUEST_CANCEL   -201    //File Requestor Cancelled

//********* Returned by file functions **************************
#define LDERR_FILE_NOT_FOUND	    -401    //Loader errors
#define LDERR_WRONG_FILE_TYPE	    -402
#define LDERR_DISK_FULL 			-403
#define LDERR_DISK_WRITE_PROTECTED  -404
#define LDERR_FILE_WRITE_PROTECTED  -405
#define LDERR_MISC_FILE_ERROR	    -406
#define LDERR_STRING_TOO_LONG	    -407    //Supplied filename is over 128 chars in length

//*********** Returned by frame commands such as DisplayFrame ***
#define LDERR_FRAME_OUT_OF_RANGE    -501    //Misc programming or config errors

//*********** Returned by point commands such as writepoint *****
#define LDERR_POINT_OUT_OF_RANGE    -502

//*********** Returned by show control commands *****************
#define LDERR_TDC_NOT_FOUND		-511
#define LDERR_TRANSITION_NOT_FOUND	-512
#define LDERR_EFFECT_NOT_FOUND  	-513
#define LDERR_SCENE_NOT_FOUND   	-514
#define LDERR_MODULE_NOT_FOUND  	-515
#define LDERR_SHOW_NOT_FOUND		-516
#define LDERR_STRUCTURE_NOT_FOUND	-519

//Once the element has been deleted, this will be returned
#define LDERR_EFFECT_DELETED		-530
#define LDERR_SCENE_DELETED		-531
#define LDERR_MODULE_DELETED		-532
#define LDERR_SHOW_DELETED		-533
#define LDERR_STRUCTURE_DELETED		-539

//If you try to delete something which is inuse one of these will be returned
#define LDERR_EFFECT_INUSE		-540
#define LDERR_SCENE_INUSE		-541
#define LDERR_MODULE_INUSE		-542
#define LDERR_SHOW_INUSE		-543
#define LDERR_STRUCTURE_INUSE		-549

//*********** These should be rare indeed ***********************
#define LDERR_NO_IBM_MEMORY	    -601    //No free IBM Memory
#define LDERR_CANT_OPEN_WINDOW	    -602    //Can't open Window (Debug, FileRequest)

//*********** Alot of commands can return this one **************
#define LDERR_NO_QM32_MEMORY	    -702    //No free QM32 Memory

//*********** Ran out of memory while trying to load a file *****
#define LDERR_FILE_TOO_LARGE	    -703    //No free QM32 Memory to load frames

//********** Several unimplemented DLL commands return this *****
#define LDERR_NOT_IMPLEMENTED	    -801    //DLL Command not implemented

//*** This indicates a timeout during a long communication with the QM32
//*** This only occurs during file loading and saving and ActiveArray functions
#define LDERR_QM32_ERROR	    -901    //QM32 Communication error

//***** These should only be returned by InitialQMCheck *********
#define LDERR_QM32_NOT_PRESENT	    -1001   //QM32 is not present (like it sounds)
#define LDERR_QM32_ADDRESS_CONFLICT -1002   //QM32 is not responding properly
#define LDERR_QM32_INITIALIZATION_ERROR	-1003	 //Same as above, but even weirder

/*************************************************************************
*   QM32 Error Codes							 *
**************************************************************************/

//*** This is a sure sign of QM32 hardware trouble
#define QM32ERROR_NullCommand	1

//*** Usually because of no laser code running in the QM32 but could
//*** be caused by a DLL / Q32 mismatch.
#define QM32ERROR_UndefinedCommand	2

//*** Should only occur during BeginSession. This indicates that a
//*** communications error has occured while loading the file LD.Q32
#define QM32ERROR_DownloadBad	3
#define QM32ERROR_ChecksumBad	4

//*** This is usually caused by a DLL / Q32 mismatch. If it happens
//*** inconsistantly then this could be a sign of QM32 hardware trouble
#define QM32ERROR_Insufficient	5

//*** When the QM32 is reset it does 3 ram tests. If one of them fails, this
//*** would be returned. This is a sign of QM32 hardware trouble.
#define QM32ERROR_DramTestBad	6

//*** If somebody tampers with the rom chip, this would be returned. Actually
//*** nothing may be returned because the QM32 would probably lock up.
#define QM32ERROR_RomTestBad	7

//*** This is equivalent to the Amiga's GURU meditation. It occurs when some
//*** kind of QM32 software violates the system. (divide by zero, access to memory that isn't there, etc.)
//*** In certain circomstances, it may also be caused by a hardware problem.
#define QM32ERROR_AccessError	8

//***************************************************************************
//   Flags used by RedrawFrameFast and other things							*
//***************************************************************************

#define RFF_NOBLIT			64		//RedrawFrameFast should not copy the bitmap to the screen
#define RFF_NOINITMINMAX	128		//RedrawFrameFast should not init Min and Max variables to 0
#define RFF_DOUBLEBUFFER	2048	//RedrawFrameFast should use double buffering
#define RFF_WHITEDOT		8192	//RedrawFrame should place a white dot at the first point
#define RFF_MONITORPZ		32768	//RedrawFrame should use the laser coordinates from the monitor buffer and not the laser coordinates
#define RFF_GRAYBLANKING	65536	//Show blanking lines as gray lines
#define RFF_SELECTEDZONES	131072	//Show only the selected zones, supplied in the Frame parameter of RedrawFrameFast

//***************************************************************************
//   Flags used by DisplayUpdateEx and sometimes by PlayShowEx				*
//***************************************************************************

#define DU_LASEROUTPUT		1		//DisplayUpdate should show laser output
#define DU_BEGINSLEW		16		//DisplayUpdate should slew beginning blanked points leading up to the first track
#define DU_CUTSCAN			256     //DisplayUpdate should immediately stop scanning buffer and start a new buffer


/**************************************************************************
*   Structures															  *
***************************************************************************/

typedef struct tagDMXCHANNELMAP{
	LONG Function;
	SHORT SlewRate;
	SHORT Mode;
	LONG Reserved1;
	LONG Reserved2;
	LONG Reserved3;
	LONG StartupSource;
	LONG StartupValue;
	LONG ShutdownSource;
	LONG ShutdownValue;
	LONG ESTOPSource;
	LONG ESTOPValue;
	LONG DeselectedSource;
	LONG DeselectedValue;
	LONG Source;
	LONG Value;
	LONG STMin;
	LONG DMXMin;
	LONG STMax;
	LONG DMXMax;
	LONG STOrigin;
	LONG STExtent;
	LONG DMXOrigin;
	LONG DMXExtent;
	LONG DMXExtentOV2;
} DMXCHANNELMAP;

typedef struct tagDMXMAP{
	SHORT Reserved1;
	SHORT NumChannels;
	SHORT Reserved2;
	SHORT StartingChannel;
	SHORT Reserved3;
	SHORT DeviceType;
	SHORT Reserved4;
	SHORT Nondisplayed;
} DMXMAP;

typedef struct tagPROJECTIONZONE{
	LONG Scanner;			//Scanner for output

    LONG XSize;				//Geometric Correction variable -- X Size
    LONG YSize;				//Geometric Correction variable -- Y Size
    LONG XPosition;			//Geometric Correction variable -- X Position
    LONG YPosition;			//Geometric Correction variable -- Y Position
    LONG ZRotation;			//Geometric Correction variable -- Z Rotation angle
    LONG XLinearity;		//Geometric Correction variable -- X Linearity
    LONG YLinearity;		//Geometric Correction variable -- Y Linearity
    LONG XKeystone;			//Geometric Correction variable -- X Keystone
    LONG YKeystone;			//Geometric Correction variable -- Y Keystone
//10    
	LONG XPincussion;		//Geometric Correction variable -- X Pin
    LONG YPincussion;		//Geometric Correction variable -- Y Pin
    LONG XPincussionOffset;	//Geometric Correction variable -- X Pin Offset
    LONG YPincussionOffset;	//Geometric Correction variable -- Y Pin Offset
	LONG XBow;				//Geometric Correction variable -- X Bow
    LONG YBow;				//Geometric Correction variable -- Y Bow
    LONG XBowOffset;		//Geometric Correction variable -- X Bow Offset
    LONG YBowOffset;		//Geometric Correction variable -- Y Bow Offset
    LONG XShear;			//Geometric Correction variable -- X Shear
    LONG YShear;			//Geometric Correction variable -- Y Shear
//20    
	LONG XSymmetry;			//Geometric Correction variable -- X Symmetry
    LONG YSymmetry;			//Geometric Correction variable -- Y Symmetry
    LONG Reserved1;			//Geometric Correction variable -- Reserved for future use
    LONG Reserved2;			//Geometric Correction variable -- Reserved for future use

    LONG NoMinimumPointsFlag;			//BOOL: Don't require minimum points during DisplayUpdate
    LONG AllowManipulationsFlag;		//BOOL: Allow Scale, Rotate, Position, Perspective and other effects.
    LONG AllowMasterResizeFlag;			//BOOL: Allow the master size and position controls to affect this projection zone
    LONG UseGeometricCorrectionData;	//BOOL: Observe geometric correction data when calculating beam position
    
    LONG PreviewWindowXSize;				//X Size of the image drawn on the preview window
    LONG PreviewWindowYSize;				//Y Size of the image drawn on the preview window
//30    
	LONG PreviewWindowXPosition;			//X Position of the image drawn on the preview window
    LONG PreviewWindowYPosition;			//Y Position of the image drawn on the preview window
    LONG PreviewWindowXProjectorPosition;	//X Position of the center of all of the beam lines
    LONG PreviewWindowYProjectorPosition;	//Y Position of the center of all of the beam lines

    LONG PreviewWindowMirrorXOutput;		//BOOL: Produce a duplicate output with mirrored X output
    LONG PreviewWindowMirrorXProjector;		//BOOL: Produce a duplicate output with mirrored X position
    LONG PreviewWindowShowAsBeamsFlag;		//BOOL: When drawing on the preview window, show this as beams
    
    LONG ScannerOptimizer;					//BOOL: Optimize blanking path within this projection zone
    
    LONG UseProtectionData;					//BOOL: Observe protection data when calculating brightness
    
    LONG Reserved7;							//Reserved for future use
//40 long words = 160 bytes
    LONG Reserved8;							//Reserved for future use
	LONG Reserved9;
	LONG Reserved10;
	LONG Reserved11;
	LONG Reserved12;

	char Name[24];							//40 byte name field

	char Reserved200[200];					//200 reserved bytes

    short GeometricCorrectionData[8450];	//65*65*2 integers for geometric correction
    
    char ProtectionZoneData[4096];			//64*64=4096 bytes for protection zone bitmap
} PROJECTIONZONE;



typedef struct tagCOLORREGSTRUCT{
    LONG ColorIndex;
    LONG RedScreenVal;
    LONG GreenScreenVal;
    LONG BlueScreenVal;
    LONG RedLaserVal;
    LONG GreenLaserVal;
    LONG BlueLaserVal;
    LONG DeepBlueLaserVal;
    LONG YellowLaserVal;
    LONG FocusLaserVal;
	LONG IntensityLaserVal;
	LONG ShutterLaserVal;
} COLORREGSTRUCT;

typedef struct tagPTSTRUCT{
    LONG XCoord;
    LONG YCoord;
    LONG ZCoord;
    LONG FCoord;
    LONG RGBValue;
    LONG X3DCoord;
    LONG Y3DCoord;
    LONG Group;
    LONG Status;
} PTSTRUCT;

typedef struct tagFRAMESTRUCT{
    LONG VOFlag;
    LONG ScanRateMultiplier;
    LONG AbstractFlag;
    LONG NumPoints;
    char FrameNote[24];
} FRAMESTRUCT;

typedef struct tagFRAMESTRUCTEX{
	LONG ChangedFlag;				//Indicates that this frame has changed since the last time it was saved. This is read-only. 
	LONG ThreeDFlag;				//Indicates that this frame is stored internally as 3D data. This is read-only.
	LONG BeamBrushFlag;				//Indicates that this frame stores beam brush points internally. This is read-only.
	LONG VectorFlag;				//Indicates that this frame is to be rendered using the vector renderer. This is read/write.
    LONG AbstractFlag;				//Indicates that this frame has additional abstract information and that this should be rendered as an abstract.
	LONG DMXFlag;					//Indicates that this frame has DMX data in addition to point and perhaps abstract data.
	LONG RasterFlag;				//Indicates that this frame is a raster frame. No special internal handling is done at this time.
	LONG MaxRenderedFlag;			//Indicates that this frame was rendered by 3D Studio MAX.
	LONG SecureFrameFlag;			//Indicates that this frame is secured.
	LONG Reserved3;					//Reserved for future use
	LONG PreferredPalette;			//Palette that this frame will use unless overridden by Track.
	LONG PreferredProjectionZone;	//Projection zone that this frame will be projected onto unless overridden by track.
	LONG AnimationCount;			//Number of frames to the end of the animation. Range is 0 to 65535.
	LONG ClipartClass;				//Number, usually bit-encoded, that describes the frame. This is only 16-bits internally.
    LONG ScanRate;					//Scan rate for this frame. If positive, handled as a multiplier. If negative, treated as exact sample rate in 1K increments.
    LONG NumPoints;					//Number of data points in this frame.
    char FrameNote[24];				//23 character, plus null, frame note.
} FRAMESTRUCTEX;

                    
//*************************************************************************
//* Constants
//*************************************************************************

#define frontview 1
#define topview 2
#define sideview 3
#define threeDview 4
#define BLACK RGB(0,0,0)
#define DARKGRAY RGB(128,128,128)
#define LIGHTGRAY RGB(192,192,192)
#define WHITE RGB(255, 255, 255)


//**********************************************************************************
//	LD2000.DLL API Functions
//**********************************************************************************


//-------------------------------------------------------------------------------------------
//	LD Comm Window Functions
//-------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI OpenLDCommWindow(void);
LDDLL_PREFIX LONG WINAPI CloseLDCommWindow(void);
LDDLL_PREFIX LONG WINAPI WriteToLDCommWindow(LPSTR SUPPLY_OutputString);
LDDLL_PREFIX LONG WINAPI QM32BeepOnError(LONG OnOff);	//Controls whether or not the Comm window automatically opens


//--------------------------------------------------------------------------------------------
//	Port functions. Only used in the Interactive Character project.
//--------------------------------------------------------------------------------------------
              
LDDLL_PREFIX unsigned short WINAPI ldInp(unsigned short PortID);
                         
LDDLL_PREFIX unsigned short WINAPI ldInpw(unsigned short PortID);
                         
LDDLL_PREFIX unsigned short WINAPI ldOutp(unsigned short PortID, short DataByte);
                         
LDDLL_PREFIX unsigned short WINAPI ldOutpw(unsigned short PortID, unsigned short DataWord);

//--------------------------------------------------------------------------------------------
//	Status and setting and retrieval
//--------------------------------------------------------------------------------------------
                         
LDDLL_PREFIX LONG WINAPI GetLDStatus(LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI GetDates(LPLONG RETURN_DLLDate, LPLONG RETURN_LibDate, LPLONG RETURN_RomDate);

LDDLL_PREFIX LONG WINAPI GetSerialNumber(LPLONG RETURN_SerialNumber);


//------------------------------------------------------------------------------------------
//	QuadMod and session functions
//------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI GetLDDLLModuleUsage(LPLONG ModuleUsage);

LDDLL_PREFIX LONG WINAPI QM32AvailMemEx(LPLONG RETURN_TotalMemory, LPLONG RETURN_TotalFreeMemory, LPLONG RETURN_LargestBlock, LPLONG RETURN_NumFreeBlocks);
LDDLL_PREFIX LONG WINAPI QM32DefragmentMem(LPLONG RETURN_NumBlocksCombined);
LDDLL_PREFIX LONG WINAPI RebootQM32(void);
LDDLL_PREFIX LONG WINAPI InitialQMCheck(LPLONG RETURN_LDStatus);
LDDLL_PREFIX LONG WINAPI BeginSession(LONG MaxFrames, LONG MaxPoints, LONG MaxBuffer, LONG UndoFrames, LPLONG RETURN_LDStatus);
LDDLL_PREFIX LONG WINAPI CheckSession(LPLONG RETURN_LDStatus);
LDDLL_PREFIX LONG WINAPI CheckSessionSettings(LPLONG RETURN_MaxFrames, LPLONG RETURN_MaxPoints, LPLONG RETURN_MaxBuffer, LPLONG RETURN_UndoFrames, LPLONG RETURN_LDStatus);
LDDLL_PREFIX LONG WINAPI EndSession(void);

LDDLL_PREFIX LONG WINAPI BeginSessionEx(LPLONG RETURN_Version, LPLONG RETURN_MaxFrames, LPLONG RETURN_MaxPoints, LPLONG RETURN_MaxBuffer, LPLONG RETURN_UndoFrames, LPLONG RETURN_LDStatus);
//Does everything necessary to start a session including checking settings and loading parameters from INI file including palette and geometric correction data


//------------------------------------------------------------------------------------------
//	Working Scanners, Tracks, Frames
//------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI SetWorkingScanners(LONG Scanner);
LDDLL_PREFIX LONG WINAPI GetWorkingScanners(LONG *RETURN_ScannerSelection);

LDDLL_PREFIX LONG WINAPI SetWorkingTracks(LONG Track);

LDDLL_PREFIX LONG WINAPI SetWorkingFrame(LONG FrameNumber);
LDDLL_PREFIX LONG WINAPI GetWorkingFrame(LPLONG RETURN_Frame);


//-------------------------------------------------------------------------------------------
//	Projection Zone Functions
//-------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI ReadProjectionZone(LONG ZoneNumber, PROJECTIONZONE *RETURN_PZ);

LDDLL_PREFIX LONG WriteProjectionZone(LONG ZoneNumber, PROJECTIONZONE *SUPPLY_PZ);

LDDLL_PREFIX LONG WINAPI LoadProjectionZones(LPSTR ZoneFilename, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI SaveProjectionZones(LPSTR ZoneFilename, LPLONG RETURN_LDStatus);


//-----------------------------------------------------------------------------------------
//	Showtime Synchronization Functions
//-----------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI QM32MultiCommand(LONG Allow);
LDDLL_PREFIX LONG WINAPI SetSyncParameters(LONG Proportional, LONG Integral, LONG Derivative);
LDDLL_PREFIX LONG WINAPI SendSync(LONG CurrentTime);
LDDLL_PREFIX LONG WINAPI SendMultiSync(LONG CurrentTime);
LDDLL_PREFIX LONG WINAPI GetShowTimeMilliseconds(LPLONG RETURN_CurrentTime);
LDDLL_PREFIX LONG WINAPI GetQMEvents(LPLONG RETURN_nEvents, LPSTR RETURN_MIDIEventStruct);
LDDLL_PREFIX LONG WINAPI SendSyncGetQMEvents(LONG CurrentTime, LPLONG RETURN_nEvents, LPSTR RETURN_MIDIEventStruct);


//--------------------------------------------------------------------------------------------
//	Showtime operation functions
//--------------------------------------------------------------------------------------------


LDDLL_PREFIX LONG WINAPI SetResetTTL(LONG IOMask, LONG OutputData);	//This may go away

LDDLL_PREFIX LONG WINAPI PlayShowEx(LONG StartTime, LONG EndTime, LONG PlaySpeed, LONG Mode);	

LDDLL_PREFIX LONG WINAPI StopShow(void);	//Correct way to stop a show in progress.
    
    
LDDLL_PREFIX LONG WINAPI StopShowEx(LONG Mode);	//Mode parameter allows you to skip the final refresh after PlayShow has ended.

//--------------------------------------------------------------------------------------------
//	Showtime file functions
//--------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI LoadShowStructures(LPSTR Filename, LONG StructureTypes, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI SaveShowStructures(LPSTR Filename, LONG StructureTypes, LPINT RETURN_LDStatus);


//--------------------------------------------------------------------------------------------
//	Laser Display Functions
//--------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI ResetLD(void);			//Updates all track variables 

LONG WINAPI DisplayGeometricCorrection( long Enable, long YLine, short *GeometryData);	//GeometryData must be in device coordinates

LDDLL_PREFIX LONG WINAPI DisplaySize( float XSize, float YSize, float ZSize, float CSize, float FSize);	// Normalized Variables

LDDLL_PREFIX LONG WINAPI DisplayPosition( float XPosition, float YPosition ); // Normalized Variables

LDDLL_PREFIX LONG WINAPI DisplayAuxOutput(float OutputVoltage);

LDDLL_PREFIX LONG WINAPI lsDisplayAuxOutput(float OutputVoltage, float ZSize, float ZOffset);

LDDLL_PREFIX LONG WINAPI DisplayMasterAngle(float XAngle, float YAngle, float ZAngle);	  //Normalized


LDDLL_PREFIX LONG WINAPI DisplayFreq3(long PointFreq, long VectorFreq, long AbstractFreq, LPLONG RETURN_ActualPointFreq, LPLONG RETURN_ActualVectorFreq, LPLONG RETURN_ActualAbstractFreq);

LDDLL_PREFIX LONG WINAPI DisplayScanLimits(LONG FromPt, LONG ToPt);

LDDLL_PREFIX LONG WINAPI DisplayObjectSettings(LONG VisibleBeginEmph, LONG VisibleMiddleEmph,
				     LONG VisibleEndEmph, LONG VisibleDensity,
				     LONG BlankedBeginEmph, LONG BlankedEndEmph,
				     LONG BlankedDensity);

LDDLL_PREFIX LONG WINAPI DisplayAbstractSettings(LONG NumVisible, LONG NumOverscan, LONG NumOverlap);

LDDLL_PREFIX LONG WINAPI DisplayFlags(LONG Flags);
                       
LDDLL_PREFIX LONG WINAPI SetGetDisplayFlags(LONG Mode, LONG TrackNumber, LPLONG RETURN_Flags);

LDDLL_PREFIX LONG WINAPI DisplayMaskFlags(LONG MaskFlags);
                       
LDDLL_PREFIX LONG WINAPI DisplayScale(float XScale, float YScale, float ZScale);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayRotCenter(float XRotCent, float YRotCent, float ZRotCent);	// Normalized

LDDLL_PREFIX LONG WINAPI DisplayAngle(float XAngle, float YAngle, float ZAngle);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayAngleDegrees(float XAngle, float YAngle, float ZAngle);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayRotOrder(LONG RotationOrder);

LDDLL_PREFIX LONG WINAPI DisplayWAngle(float XAngle, float YAngle, float ZAngle);	 //Normalized

LDDLL_PREFIX LONG WINAPI DisplayWAngleDegrees(float XAngle, float YAngle, float ZAngle);	 //Normalized

LDDLL_PREFIX LONG WINAPI AutoRotateInit(float InitialXAngle, float InitialYAngle,
			      float InitialZAngle, LONG AnimateFromFrame,
			      LONG AnimateToFrame, long AutoRotateTrack);

LDDLL_PREFIX LONG WINAPI AutoRotateIncrement(float XIncrement, float YIncrement, float ZIncrement, LONG TenthFlag);

LDDLL_PREFIX LONG WINAPI AutoRotatePlus(LONG Rotate, LONG GenerateAbstract, LONG Animate, LONG Extra);	//Pretty much unneccessary now

LDDLL_PREFIX LONG WINAPI DisplayPostTrans(float XOffs, float YOffs, float ZOffs);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayStereoSettings(float EyeSeparation, long EndOfScanDelay,
				     float StereoIDLEVoltage, float StereoLEFTVoltage, float StereoRIGHTVoltage); //Normalized

LDDLL_PREFIX LONG WINAPI DisplayPerspective(float Dist, float Xobs, float Yobs, float Zobs);//Norm

LDDLL_PREFIX LONG WINAPI DisplayWindow( float WinLeft,
			      float WinRight, float WinBottom, float WinTop,
			      float WinRear, float WinFront);  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayWindowX(float WinLeft, float WinRight);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayWindowY(float WinBot, float WinTop);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayWindowZ(float WinRear, float WinFront);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayCSize(float ColorSize);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayPaletteSelect(LONG Palette1, LONG Palette2);

LDDLL_PREFIX LONG WINAPI DisplayPaletteMix(float MixValue);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayHueOffset(LONG HueStart, LONG HueEnd, LONG HueNew);

LDDLL_PREFIX LONG WINAPI DisplaySaturation(float SaturationLevel);  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayPath(LONG Frame, LONG Point);

LDDLL_PREFIX LONG WINAPI DisplayIris(float IrisLevel);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayColorWheel(float ColorWheelLevel);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayGoboWheel(float Gobo1, float Gobo2, float Gobo3);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayGoboRotate(float Gobo1, float Gobo2, float Gobo3);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayFrost(float Frost1, float Frost2, float Frost3);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayFocus(float FocusLevel);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayMiscControl(float M1, float M2, float M3);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplayDepthCue(float DepthEffect, float MinimumZ, float MaximumZ);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayTrackAllow(long Permission);

LDDLL_PREFIX LONG WINAPI DisplayProjectionZones(long ProjectionZoneCode);

LDDLL_PREFIX LONG WINAPI SetGetDisplayProjectionZones(LONG Mode, LONG TrackNumber, LPLONG RETURN_ProjectionZoneCode);

LDDLL_PREFIX LONG WINAPI DisplayFrame(LONG Frame);

LDDLL_PREFIX LONG WINAPI SetGetDisplayFrame(LONG Mode, LONG TrackNumber, LPLONG RETURN_FrameNumber);

LDDLL_PREFIX LONG WINAPI DisplayFrameMix(LONG ToFrame, float MixValue);	  //Normalized

LDDLL_PREFIX LONG WINAPI DisplayPreTrans( float XOffs, float YOffs, float ZOffs);	//Normalized

LDDLL_PREFIX LONG WINAPI DisplaySkew(LONG TrackDelay, LONG ColorShift, LONG FocusShift);
LDDLL_PREFIX LONG WINAPI DisplayMinimumPoints(LONG MinimumPoints);

LDDLL_PREFIX LONG WINAPI DisplayUpdateTest(void);

LONG WINAPI DisplayBufferStatus(LONG *RETURN_BufferIsFree, LONG *RETURN_CurrentOutputPoints);

LDDLL_PREFIX LONG WINAPI DisplayUpdate(void);

LDDLL_PREFIX LONG WINAPI DisplayUpdateToFrame(LONG Frame);

LDDLL_PREFIX LONG WINAPI DisplayUpdateEx(LONG Mode);

LDDLL_PREFIX LONG WINAPI DisplayUpdateMode(LONG SyncMode, LONG LeftRightUpdate);

//--------------------------------------------------------------------------------------------
//	QuadMod DMX-512 functions
//--------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI QMDMXMode(LONG DMXMode, LONG TransmitBreakLength, LONG TransmitMABLength, LONG TransmitStartCode, LONG TransmitNumChannels, LONG TransmitRestLength);

LDDLL_PREFIX LONG WINAPI QMDMXInput(LONG ChannelNumber, LPLONG RETURN_Data);

LDDLL_PREFIX LONG WINAPI QMDMXOutput(LONG ChannelNumber, LONG Data);

LDDLL_PREFIX LONG WINAPI QMDMXVirtualChannels(LONG PortAStartChannel, LONG PortBStartChannel, LONG AUXOutChannel, LONG GraphicsShutterChannel);

LDDLL_PREFIX LONG WINAPI QMDMXStartupFrame(LONG Frame);

LDDLL_PREFIX LONG WINAPI QMDMXShutdownFrame(LONG Frame);

LDDLL_PREFIX LONG WINAPI QMDMXESTOP(void);

LDDLL_PREFIX LONG WINAPI DMXToFrame(void);

LDDLL_PREFIX LONG WINAPI FrameToDMX(void);

LDDLL_PREFIX LONG WINAPI GetDMXMap(DMXMAP *RETURN_DMXMap);

LDDLL_PREFIX LONG WINAPI SetDMXMap(DMXMAP *SUPPLY_DMXMap);

LDDLL_PREFIX LONG WINAPI GetDMXChannelMap(LONG ChannelNumber, DMXCHANNELMAP *RETURN_DMXChannel);

LDDLL_PREFIX LONG WINAPI SetDMXChannelMap(LONG ChannelNumber, DMXCHANNELMAP *SUPPLY_DMXChannel);


//-------------------------------------------------------------------------------------------
//	Frame Loading and Saving
//-------------------------------------------------------------------------------------------


LDDLL_PREFIX LONG WINAPI LoadLDA(LPSTR Filename, LPLONG SUPPLY_FileFromFr, LPLONG SUPPLY_FromFr, LPLONG SUPPLY_ToFr,LPSTR SUPPLY_Password, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI LoadLDB(LPSTR Filename, LPLONG SUPPLY_FileFromFr, LPLONG SUPPLY_FromFr, LPLONG SUPPLY_ToFr, LPSTR SUPPLY_Password, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI LoadLDSecure(LPSTR Filename, LPLONG SUPPLY_FileFromFr, LPLONG SUPPLY_FromFr, LPLONG SUPPLY_ToFr, LPSTR SUPPLY_Password, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI LoadLFile(LPSTR Filename, LPLONG SUPPLY_FromFr, LPLONG SUPPLY_ToFr, LPSTR SUPPLY_Password, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI ReadFileInfo(LPSTR Filename, LPSTR RETURN_Description, LPLONG RETURN_NumFrames, LPLONG RETURN_Format, LPSTR SUPPLY_Password, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI SaveLFile(LPSTR Filename, LPSTR Description, LPLONG SUPPLY_FromFr, LPLONG SUPPLY_ToFr, LPLONG Format, LPSTR SUPPLY_Password, LPLONG RETURN_LDStatus);


//-------------------------------------------------------------------------------------------
//	Frame Functions
//-------------------------------------------------------------------------------------------


LDDLL_PREFIX LONG WINAPI FindFrame(LPSTR Frame, LPLONG RETURN_FrameNumber);

LDDLL_PREFIX LONG WINAPI ReadMaskData(LONG TrackNumber, LONG FAR* RETURN_MaskData);

LDDLL_PREFIX LONG WINAPI SetChangedFlag(LONG FrameNumber, LONG State);

LDDLL_PREFIX LONG WINAPI ReadFrameStruct(LONG FrameNumber, FRAMESTRUCT *RETURN_FrameStruct);

LDDLL_PREFIX LONG WINAPI ReadFrameStructEx(LONG FrameNumber, FRAMESTRUCTEX *RETURN_FrameStructEx);

LDDLL_PREFIX LONG WINAPI WriteFrameStructEx(LONG FrameNumber,FRAMESTRUCTEX *SUPPLY_FrameStructEx);

LDDLL_PREFIX LONG WINAPI ReadFrame(FRAMESTRUCT *RETURN_FrameStruct, PTSTRUCT *RETURN_PointArray);

LDDLL_PREFIX LONG WINAPI WriteFrame(FRAMESTRUCT *SUPPLY_FrameStruct, PTSTRUCT *SUPPLY_PointArray);

LDDLL_PREFIX LONG WINAPI WriteFrameEx(FRAMESTRUCTEX *SUPPLY_FrameStructEx, PTSTRUCT *SUPPLY_PointArray);

LDDLL_PREFIX LONG WINAPI CopyFrameActive(LONG SourceFrame, LONG DestFrame, LPLONG SUPPLY_ActiveArray);

LDDLL_PREFIX LONG WINAPI CopyBlock(LONG FromFrame, LONG ToFrame, LONG DestFrame);

LDDLL_PREFIX LONG WINAPI ReverseBlock(LONG FromFrame, LONG ToFrame);

LDDLL_PREFIX LONG WINAPI DeleteFrames(LONG FromFr, LONG ToFr);

LDDLL_PREFIX LONG WINAPI MergeFrames(LONG source1fr, LONG source2fr, LONG destfr);

LDDLL_PREFIX LONG WINAPI FrameToUndoActive(LPLONG SUPPLY_ActiveArray);	//If no active array is used, supply a NULL pointer.

LDDLL_PREFIX LONG WINAPI UndoToFrameActive(LPLONG RETURN_ActiveArray);		//If no active array is used, supply a NULL pointer.


//--------------------------------------------------------------------------------------------
//	Vector Frame functions
//--------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI ConvertToVectorFrame(LONG OptimizePath, LONG PreserveBlanking,
			       LONG StartingX, LONG StartingY, LONG StartingZ);

LDDLL_PREFIX LONG WINAPI ConvertToPointFrame(LONG VisibleBeginEmph, LONG VisibleMiddleEmph,
				     LONG VisibleEndEmph, LONG VisibleDensity,
				     LONG BlankedBeginEmph, LONG BlankedEndEmph,
				     LONG BlankedDensity);

//-------------------------------------------------------------------------------------------
//	Point Functions
//-------------------------------------------------------------------------------------------


LDDLL_PREFIX LONG WINAPI ReadNumPoints(LPLONG RETURN_NumPoints);

LDDLL_PREFIX LONG WINAPI ReadPoint(LONG PointNumber, PTSTRUCT *RETURN_PtStruct);

LDDLL_PREFIX LONG WINAPI WritePoint(LONG PointNumber, PTSTRUCT *SUPPLY_PtStruct);

LDDLL_PREFIX LONG WINAPI SetInsertOnWrite(LONG TrueOrFalse);	//For WritePoint function


LDDLL_PREFIX LONG WINAPI DeletePoints(LONG FromPt, LONG ToPt);

LDDLL_PREFIX LONG WINAPI InsertPoints(LONG FromPt, LONG ToPt);

LDDLL_PREFIX LONG WINAPI InvertPointsActive(LONG FromPt, LONG ToPt, LONG XOffs, LONG YOffs,
				  LONG ZOffs, LONG Mask, LPLONG SUPPLY_ActiveArray);

LDDLL_PREFIX LONG WINAPI MovePointsActive(LONG FromPt, LONG ToPt, LONG XOffs, LONG YOffs,
				LONG ZOffs, LONG Mask, LPLONG SUPPLY_ActiveArray);

LDDLL_PREFIX LONG WINAPI ResizePointsActive(LONG FromPt, LONG ToPt,
				float XResizepct, float YResizepct, float ZResizepct,
				LONG XResizectr, LONG YResizectr, LONG ZResizectr,
				LONG Mask, LPLONG SUPPLY_ActiveArray);

LDDLL_PREFIX LONG WINAPI RotatePointsActive(LONG FromPt, LONG ToPt,
				float XRotatepct, float YRotatepct, float ZRotatepct,
				LONG XRotatectr, LONG YRotatectr, LONG ZRotatectr,
				LONG Mask, LPLONG SUPPLY_ActiveArray);

LDDLL_PREFIX LONG WINAPI Set_XYZ_TO_3D_Track(LONG Track);

LDDLL_PREFIX LONG WINAPI XYZ_TO_3D(LONG XCoord, LONG YCoord, LONG ZCoord, LPLONG RETURN_X3D, LPLONG RETURN_Y3D, LPLONG RETURN_VisibleFlag);

//-------------------------------------------------------------------------------------------
//	Palette and color Functions
//-------------------------------------------------------------------------------------------


LDDLL_PREFIX LONG WINAPI LoadPalette(LPSTR ColorFilename, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI LoadPaletteDots(LPSTR ColorFilename, LONG *RETURN_PaletteDotColor1, LONG *RETURN_PaletteDotColor2, LONG *RETURN_PaletteDotColor3, LONG *RETURN_PaletteDotColor4, LONG *RETURN_PaletteDotColor5, LONG *RETURN_PaletteDotColor6, LONG *RETURN_PaletteDotColor7, LONG *RETURN_PaletteDotColor8, char *RETURN_PaletteDotName1, char *RETURN_PaletteDotName2, char *RETURN_PaletteDotName3, char *RETURN_PaletteDotName4, char *RETURN_PaletteDotName5, char *RETURN_PaletteDotName6, char *RETURN_PaletteDotName7, char *RETURN_PaletteDotName8, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI SavePalette(LPSTR ColorFilename, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI SavePaletteDots(LPSTR ColorFilename, LONG *SUPPLY_PaletteDotColor1, LONG *SUPPLY_PaletteDotColor2, LONG *SUPPLY_PaletteDotColor3, LONG *SUPPLY_PaletteDotColor4, LONG *SUPPLY_PaletteDotColor5, LONG *SUPPLY_PaletteDotColor6, LONG *SUPPLY_PaletteDotColor7, LONG *SUPPLY_PaletteDotColor8, char *SUPPLY_PaletteDotName1, char *SUPPLY_PaletteDotName2, char *SUPPLY_PaletteDotName3, char *SUPPLY_PaletteDotName4, char *SUPPLY_PaletteDotName5, char *SUPPLY_PaletteDotName6, char *SUPPLY_PaletteDotName7, char *SUPPLY_PaletteDotName8, LPLONG RETURN_LDStatus);

LDDLL_PREFIX LONG WINAPI TrainColorSystem(void);

LDDLL_PREFIX LONG WINAPI UseDefaultColors(void);

LDDLL_PREFIX LONG WINAPI EliminateUnusedColors(LONG LastColorNumber);

LDDLL_PREFIX LONG WINAPI UpdateAlternatePalettes(void);


LDDLL_PREFIX LONG WINAPI ReadColorReg(COLORREGSTRUCT *RETURN_ColorStruct);

LDDLL_PREFIX LONG WINAPI ReadColorRegEx(LONG PaletteSelect, COLORREGSTRUCT *RETURN_ColorStruct);

LDDLL_PREFIX LONG WINAPI ReadNumColors(LPLONG RETURN_NumColors);

LDDLL_PREFIX LONG WINAPI WriteColorReg(COLORREGSTRUCT *SUPPLY_ColorStruct);

LDDLL_PREFIX LONG WINAPI WriteColorRegEx(LONG PaletteSelect, COLORREGSTRUCT *SUPPLY_ColorStruct);

LDDLL_PREFIX LONG WINAPI ReadActuatorColorMode(LPLONG RETURN_Mode);

LDDLL_PREFIX LONG WINAPI WriteActuatorColorMode(LONG Mode);

LDDLL_PREFIX LONG WINAPI RGB4toRGB1(LPLONG SUPPLY_Red, LPLONG SUPPLY_Green, LPLONG SUPPLY_Blue,
			  LPLONG SUPPLY_MSB, LPLONG RETURN_RGBMValue);

LDDLL_PREFIX LONG WINAPI RGB1toRGB4(LPLONG SUPPLY_RGBMValue, LPLONG RETURN_Red, LPLONG RETURN_Green,
			  LPLONG RETURN_Blue, LPLONG RETURN_MSB);

LDDLL_PREFIX LONG WINAPI RGBtoHSV(LPLONG SUPPLY_RGB, LPLONG RETURN_HSV);

LDDLL_PREFIX LONG WINAPI HSVtoRGB(LPLONG SUPPLY_HSV, LPLONG RETURN_RGB);


//-------------------------------------------------------------------------------------------
//	Scale setting and getting functions. Sets coordinate scale for Frame/Point reading/writing
//-------------------------------------------------------------------------------------------



LDDLL_PREFIX LONG WINAPI SetWorkingScale(LONG LogicalLeft, LONG LogicalRight,
			       LONG LogicalBottom, LONG LogicalTop,
			       LONG LogicalRear, LONG LogicalFront);


LDDLL_PREFIX LONG WINAPI SetWorkingScaleX(LONG LogicalLeft, LONG LogicalRight,
				LONG LogicalOriginX, LONG DeviceLeft,
				LONG DeviceRight, LONG DeviceOriginX);

LDDLL_PREFIX LONG WINAPI SetWorkingScaleY(LONG LogicalBottom, LONG LogicalTop,
				LONG LogicalOriginY, LONG DeviceBottom,
				LONG DeviceTop, LONG DeviceOriginY);

LDDLL_PREFIX LONG WINAPI SetWorkingScaleZ(LONG LogicalRear, LONG LogicalFront,
				LONG LogicalOriginZ, LONG DeviceRear,
				LONG DeviceFront, LONG DeviceOriginZ);

LDDLL_PREFIX LONG WINAPI SetWorkingScaleF(LONG LogicalMaxFocus, LONG LogicalOriginF,
				LONG DeviceMaxFocus, LONG DeviceOriginF);

LDDLL_PREFIX LONG WINAPI GetWorkingScaleX(LPLONG LogicalLeft, LPLONG LogicalRight,
				LPLONG LogicalOriginX, LPLONG DeviceLeft,
				LPLONG DeviceRight, LPLONG DeviceOriginX);

LDDLL_PREFIX LONG WINAPI GetWorkingScaleY(LPLONG LogicalBottom, LPLONG LogicalTop,
				LPLONG LogicalOriginY, LPLONG DeviceBottom,
				LPLONG DeviceTop, LPLONG DeviceOriginY);

LDDLL_PREFIX LONG WINAPI GetWorkingScaleZ(LPLONG LogicalRear, LPLONG LogicalFront,
				LPLONG LogicalOriginZ, LPLONG DeviceRear,
				LPLONG DeviceFront, LPLONG DeviceOriginZ);

LDDLL_PREFIX LONG WINAPI GetWorkingScaleF(LPLONG LogicalMaxFocus, LPLONG LogicalOriginF,
				LPLONG DeviceMaxFocus, LPLONG DeviceOriginF);

//-------------------------------------------------------------------------------------------
//	Screen Drawing Functions
//-------------------------------------------------------------------------------------------



LDDLL_PREFIX LONG WINAPI SetLaserWindow(LONG LeftEdge, LONG RightEdge, LONG BottomEdge, LONG TopEdge, LONG MagLevel);

LDDLL_PREFIX LONG WINAPI DrawGrid(HWND hWnd, LONG gridHvalue, LONG gridVvalue, LONG MClearScreen, LONG MShowZero, LONG MShowGrid);

LDDLL_PREFIX LONG WINAPI RedrawFrame(HWND hWnd, LONG frompt, LONG topt, LONG ViewMode,
			   LONG ClearScreenFlag, long HiliteColor, LONG MShowColorAtPts,
			   LONG MShowFocus);

LDDLL_PREFIX LONG WINAPI RedrawFrameActive(HWND hWnd, LONG frompt, LONG topt, LONG ViewMode,
			   LONG ClearScreenFlag, long HiliteColor, LONG MShowColorAtPts,
			   LONG MShowFocus, LPLONG ActiveArray);

LDDLL_PREFIX LONG WINAPI RedrawFrameActiveEx(HWND hWnd, LONG frompt, LONG topt, LONG ViewMode,
			   LONG ClearScreenFlag, long HiliteColor, LONG MShowColorAtPts,
			   LONG MShowFocus, LONG MShowBlankedLines, LONG MShowBlankedPoints, LPLONG ActiveArray);

LDDLL_PREFIX LONG WINAPI RedrawFrameActiveDC(HWND hWnd, HDC hDC, LONG frompt, LONG topt, LONG ViewMode,
			   LONG ClearScreenFlag, long HiliteColor, LONG MShowColorAtPts,
			   LONG MShowFocus, LPLONG ActiveArray);

LDDLL_PREFIX LONG WINAPI RedrawFrameActiveDCEx(HWND hWnd, HDC hDC, LONG frompt, LONG topt, LONG ViewMode,
			   LONG ClearScreenFlag, long HiliteColor, LONG MShowColorAtPts,
			   LONG MShowFocus, LONG MShowBlankedLines, LONG MShowBlankedPoints, LPLONG ActiveArray);

LDDLL_PREFIX LONG WINAPI RedrawFrameFast(HWND hWnd, LONG FrameNumber, LONG ViewMode, LONG ClearScreenFlag, LONG MShowFocus);
                                                     
LDDLL_PREFIX LONG WINAPI RedrawFrameFastDC(HWND hWnd, HDC hDC, LONG FrameNumber, LONG ViewMode, LONG ClearScreenFlag, LONG MShowFocus);

LDDLL_PREFIX LONG WINAPI RedrawFrameFastDCRECT(HWND hWnd, HDC hDC, RECT *rect, LONG FrameNumber, LONG ViewMode, LONG ClearScreenFlag, LONG MShowFocus);

//-------------------------------------------------------------------------------------------
//	Text functions
//-------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI SetLineLimits(LONG NumBlanked, LONG NumEmphasis, LONG NumPoints, LONG Density);

LDDLL_PREFIX LONG WINAPI LoadFont(LPSTR FontFilename, LPLONG RETURN_LDStatus);
LDDLL_PREFIX LONG WINAPI SetLaserChar(LONG ViewMode, LONG TextBlanked, LONG TextEmphasis, LONG Density, LONG Width, LONG Height, LONG MaxAngle, LONG AngleDetect, long RGBColor);
LDDLL_PREFIX LONG WINAPI LaserChar(LONG Ascii, LONG XStart, LONG YStart, LONG BeginPoint, LPLONG RETURN_charXEnd, LPLONG RETURN_charYEnd, LPLONG RETURN_charEndPt);
LDDLL_PREFIX LONG WINAPI GetLaserCharExtent(LONG Ascii, LPLONG RETURN_charXExtent, LPLONG RETURN_charYExtent);

//-------------------------------------------------------------------------------------------
//	QuadMod32 Parallel Port Functions
//-------------------------------------------------------------------------------------------

LDDLL_PREFIX LONG WINAPI ParOn(LONG BankNum, LONG LineNum);

LDDLL_PREFIX LONG WINAPI ParOff(LONG BankNum, LONG LineNum);

LDDLL_PREFIX LONG WINAPI ParIn(LONG BankNum, LONG LineNum, LPLONG RETURN_Value);

LDDLL_PREFIX LONG WINAPI DisplayPortLong(long I_OMask, long OutputValue, LPLONG RETURN_Value);

//-------------------------------------------------------------------------------------------
//	Abstract Generator functions
//-------------------------------------------------------------------------------------------



LDDLL_PREFIX LONG WINAPI InitAbstractOsc1(float XPhase, float YPhase, float ZPhase);

LDDLL_PREFIX LONG WINAPI InitAbstractOsc2(float XPhase, float YPhase, float ZPhase);

LDDLL_PREFIX LONG WINAPI InitAbstractOsc3(float XPhase, float YPhase, float ZPhase);

LDDLL_PREFIX LONG WINAPI InitAbstractMod1(float ModPhase);

LDDLL_PREFIX LONG WINAPI InitAbstractMod2(float ModPhase);

LDDLL_PREFIX LONG WINAPI InitAbstractColor(LONG SequenceStart);

LDDLL_PREFIX LONG WINAPI InitAbstractColorNew(float ColorPhase);

LDDLL_PREFIX LONG WINAPI SetAbstractOsc1(float XFreq, float YFreq, float ZFreq, float XSize,
				float YSize, float ZSize, LONG Wave);
				//Normalized Size Variables

LDDLL_PREFIX LONG WINAPI SetAbstractOsc2(float XFreq, float YFreq, float ZFreq, float XSize,
				float YSize, float ZSize, LONG Wave);
				//Normalized Size Variables

LDDLL_PREFIX LONG WINAPI SetAbstractOsc3(float XFreq, float YFreq, float ZFreq, float XSize,
				float YSize, float ZSize, LONG Wave);
				//Normalized Size Variables

LDDLL_PREFIX LONG WINAPI SetAbstractMod1(float ModFreq, float ModAmplitude, LONG ModWave);
				//Normalized Amplitude Variable

LDDLL_PREFIX LONG WINAPI SetAbstractMod2(float ModFreq, float ModAmplitude, LONG ModWave);
				//Normalized Amplitude Variable

LDDLL_PREFIX LONG WINAPI SetAbstractColor(LONG ColorFreq, LONG ColorWave);

LDDLL_PREFIX LONG WINAPI SetAbstractColorNew(float ColorFreq, LONG ColorWave);

LDDLL_PREFIX LONG WINAPI SwapWaves(void);

LDDLL_PREFIX LONG WINAPI GenerateAbstract(LONG NumVisible, LONG NumOverscan, LONG NumOverlap);

LDDLL_PREFIX LONG WINAPI GenerateAbstractNew(LONG NumVisible, LONG NumOverscan, LONG NumOverlap);

LDDLL_PREFIX LONG WINAPI GetAbstractPhase(LONG SUPPLY_OscID, float *RETURN_XPhase, float *RETURN_YPhase, float *RETURN_ZPhase);

LDDLL_PREFIX LONG WINAPI GetAbstractFreqSizeWave(LONG SUPPLY_OscID, float *RETURN_XFreq,
				       float *RETURN_YFreq, float *RETURN_ZFreq,
				       float *RETURN_XSize, float *RETURN_YSize, float *RETURN_ZSize, int *RETURN_Wave);

LDDLL_PREFIX LONG WINAPI AbstractToFrame(void);

LDDLL_PREFIX LONG WINAPI FrameToAbstract(void);

