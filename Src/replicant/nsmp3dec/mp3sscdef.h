/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3sscdef.h
 *   project : ---
 *   author  : Martin Sieler
 *   date    : 1998-02-16
 *   contents/description: ssc definitions (Structured Status Code)
 *
 *
\***************************************************************************/

/*
 * $Date: 2010/11/17 20:46:04 $
 * $Id: mp3sscdef.h,v 1.1 2010/11/17 20:46:04 audiodsp Exp $
 */

#ifndef __MP3SSCDEF_H__
#define __MP3SSCDEF_H__

/*------------------------- includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*\
 *
 *  Standard error/return values are 32 bit values layed out as follows:
 *
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +---+-+-+-----------------------+-------------------------------+
 *  |Sev|C|R|     Handler           |               Code            |
 *  +---+-+-+-----------------------+-------------------------------+
 *
 *  where
 *
 *      Sev - is the severity code
 *
 *          00 - Success
 *          01 - Informational
 *          10 - Warning
 *          11 - Error
 *
 *      C       - is the Customer code flag
 *
 *      R       - is a reserved bit
 *
 *      Handler - is the handler code
 *
 *      Code    - is the facility's status code
 *
\*-------------------------------------------------------------------------*/

/*
 * define the Severity codes
 */

#define SSC_SEV_SUCCESS  0x00000000L
#define SSC_SEV_INFO     0x40000000L
#define SSC_SEV_WARNING  0x80000000L
#define SSC_SEV_ERROR    0xc0000000L

/*
 * define masks to extract the fields
 */

#define SSC_MASK_SEVERITY 0xc0000000L
#define SSC_MASK_HANDLER  0x0fff0000L
#define SSC_MASK_CODE     0x0000ffffL

/*
 * define MACROS to test an error/return code
 */

#define SSC_GETSEV(x) ( (x) & SSC_MASK_SEVERITY )


/* Check, if an SSC indicates success */
#define SSC_SUCCESS(x) (((SSC_GETSEV(x)==SSC_SEV_SUCCESS)||(SSC_GETSEV(x)==SSC_SEV_INFO))?1:0)

/* Check, if an SSC indicates an information */
#define SSC_INFO(x)    ((SSC_GETSEV(x)==SSC_SEV_INFO)?1:0)

/* Check, if an SSC indicates a warning */
#define SSC_WARNING(x) ((SSC_GETSEV(x)==SSC_SEV_WARNING)?1:0)

/* Check, if an SSC indicates an error */
#define SSC_ERROR(x)   ((SSC_GETSEV(x)==SSC_SEV_ERROR)?1:0)

/*-------------------------------------------------------------------------*\
 *
 *  SSC classes (handler)
 *
\*-------------------------------------------------------------------------*/

#define SSC_HANDLER_GEN 0x00000000L

#define SSC_I_GEN (SSC_SEV_INFO    | SSC_HANDLER_GEN)
#define SSC_W_GEN (SSC_SEV_WARNING | SSC_HANDLER_GEN)
#define SSC_E_GEN (SSC_SEV_ERROR   | SSC_HANDLER_GEN)

/*-------------------------------------------------------------------------*/

#define SSC_HANDLER_IO 0x00010000L

#define SSC_I_IO (SSC_SEV_INFO    | SSC_HANDLER_IO)
#define SSC_W_IO (SSC_SEV_WARNING | SSC_HANDLER_IO)
#define SSC_E_IO (SSC_SEV_ERROR   | SSC_HANDLER_IO)

/*-------------------------------------------------------------------------*/

#define SSC_HANDLER_MPGA 0x01010000L

#define SSC_I_MPGA (SSC_SEV_INFO    | SSC_HANDLER_MPGA)
#define SSC_W_MPGA (SSC_SEV_WARNING | SSC_HANDLER_MPGA)
#define SSC_E_MPGA (SSC_SEV_ERROR   | SSC_HANDLER_MPGA)

/*-------------------------------------------------------------------------*\
 *
 *  SSC codes
 *
\*-------------------------------------------------------------------------*/

typedef enum
  {
  SSC_OK                    = 0x00000000L,

  SSC_E_WRONGPARAMETER      = (SSC_E_GEN  |  1),
  SSC_E_OUTOFMEMORY         = (SSC_E_GEN  |  2),
  SSC_E_INVALIDHANDLE       = (SSC_E_GEN  |  3),

  SSC_E_IO_GENERIC          = (SSC_W_IO   |  1),
  SSC_E_IO_OPENFAILED       = (SSC_W_IO   |  2),
  SSC_E_IO_CLOSEFAILED      = (SSC_W_IO   |  3),
  SSC_E_IO_READFAILED       = (SSC_W_IO   |  4),

  SSC_I_MPGA_CRCERROR       = (SSC_I_MPGA |  1),
  SSC_I_MPGA_NOMAINDATA     = (SSC_I_MPGA |  2),

  SSC_E_MPGA_GENERIC        = (SSC_E_MPGA |  1),
  SSC_E_MPGA_WRONGLAYER     = (SSC_E_MPGA |  2),
  SSC_E_MPGA_BUFFERTOOSMALL = (SSC_E_MPGA |  3),

  SSC_W_MPGA_SYNCSEARCHED   = (SSC_W_MPGA |  1),
  SSC_W_MPGA_SYNCLOST       = (SSC_W_MPGA |  2),
  SSC_W_MPGA_SYNCNEEDDATA   = (SSC_W_MPGA |  3),
  SSC_W_MPGA_SYNCEOF        = (SSC_W_MPGA |  4)
  } SSC;

/*-------------------------------------------------------------------------*/
#endif
