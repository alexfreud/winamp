/* $Header: /cvs/root/winamp/vlb/exception.h,v 1.1 2009/04/28 20:21:09 audiodsp Exp $ */

/***************************************************************************\ 
 *
 *           Copyright 2000-2002 Dolby Laboratories, Inc.  All Rights 
 *                Reserved.  Do not copy.  Do not distribute.  
 *                     Confidential information.
 *
 *           (C) copyright Fraunhofer - IIS (1998)
 *                All Rights Reserved
 *
 *   filename: exception.cpp
 *   project : MPEG-2 AAC Decoder
 *   contents/description: common exception object
 *
 * $Header: /cvs/root/winamp/vlb/exception.h,v 1.1 2009/04/28 20:21:09 audiodsp Exp $
 *
\***************************************************************************/

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#define AAC_OK                      0x0000

#define AAC_FAILURE_BASE            0x1000

#define AAC_UNIMPLEMENTED           (AAC_FAILURE_BASE | 1)
#define AAC_NOTADIFHEADER           (AAC_FAILURE_BASE | 2)
#define AAC_DOESNOTEXIST            (AAC_FAILURE_BASE | 3)
#define AAC_ENDOFSTREAM             (AAC_FAILURE_BASE | 4)
#define AAC_SYNCERROR				(AAC_FAILURE_BASE | 5)
#define AAC_CRCERROR				(AAC_FAILURE_BASE | 6)
#define AAC_INPUT_BUFFER_EMPTY		(AAC_FAILURE_BASE | 7)
#define AAC_INVALIDCODEBOOK         (AAC_FAILURE_BASE | 8)

#ifdef MAIN_PROFILE
#define AAC_INVALIDPREDICTORRESET   (AAC_FAILURE_BASE | 9)
#endif

#define AAC_UNSUPPORTEDWINDOWSHAPE  (AAC_FAILURE_BASE | 10)
#define AAC_DOLBY_NOT_SUPPORTED		(AAC_FAILURE_BASE | 11)
#define	AAC_ILLEGAL_PROFILE			(AAC_FAILURE_BASE | 12)

#ifdef __cplusplus

/** Base Class For Exceptions.

    This class defines the interface that all exceptions possibly thrown by
    \Ref{CAacDecoder} or its input objects are derived from. 
*/

class CAacException
{

public :

  /// Exception Constructor.

  CAacException (int _value, char *_text) : value(_value), text(_text) {} ;

  /// Exception Destructor.

  ~CAacException () {} ;

  /** Exception Code Method.

      This method returns an exception status code, that can be used programatically.
  */

  int What () { return value ; }

  /** Exception Text Method.

      This method returns a pointer to a plain text explanation of what happened.
      It might be used e.g. for user notification messages or the like.
  */

  char *Explain () { return text ; }

protected :

  int value ;
  char *text ;

} ;

#define DECLARE_EXCEPTION(a,b,c) class a : public CAacException { public : a () : CAacException (b, c) {} ; }

#endif

#endif
