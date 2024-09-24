/* $Header: /cvs/root/winamp/vlb/audio_io.cpp,v 1.1 2009/04/28 20:21:07 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: audio_io.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Audio I/O include file
 *
\***************************************************************************/

#include "audio_io_dsp.h"
#include "audio_io.h"

AudioIOControl::AudioIOControl()
	:psFormatInfo(NULL),
	iError(AUDIO_ERROR_NONE)
{}

AudioIOControl::AudioIOControl(AudioIOControl&oCopyObject)
	:psFormatInfo(oCopyObject.psFormatInfo),
	iError(oCopyObject.iError)
{}

AudioIOControl& AudioIOControl::operator=(AudioIOControl&oAssignmentObject)
{
	psFormatInfo=oAssignmentObject.psFormatInfo;
	iError=oAssignmentObject.iError;
	return *this;
}

AudioIOControl::~AudioIOControl()
{}

int AudioIOControl::SetFormatInfo(AUDIO_FORMATINFO*psFormatInfo)
{
	this->psFormatInfo=psFormatInfo;
	return 0;
}

const AUDIO_FORMATINFO *AudioIOControl::GetFormatInfo() const
{
	return psFormatInfo;
}

