/* $Header: /cvs/root/winamp/vlb/DataIO.h,v 1.1 2009/04/28 20:21:06 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *   filename: DataIO.h
 *   project : MPEG-2 AAC Decoder
 *   contents/description: Data I/O include header
 *
 * $Header: /cvs/root/winamp/vlb/DataIO.h,v 1.1 2009/04/28 20:21:06 audiodsp Exp $
 *
\***************************************************************************/

#ifndef DATAIOCONTROL
#define DATAIOCONTROL

#define DATA_IO_READ	0
#define DATA_IO_WRITE	1

#define DATA_IO_ERROR_NONE		0
#define DATA_IO_ERROR_INVALID	-1


class DataIOControl{
	public:
		virtual int IO(	void*pvBuffer,
						int iSize,
						int iCount)=0; 
		virtual int Seek(long lOffset, int iOrigin)=0;
//		virtual int Close()=0;
		virtual int EndOf()=0;
		virtual int DICGetLastError()=0;
		virtual int DICGetDirection()=0;
};

#endif

