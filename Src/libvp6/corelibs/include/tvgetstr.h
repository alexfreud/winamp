/*****************************************************************************

 *

 *   Module      tvGetStr.H

 *               The defines and prototypes for the ToolVox error string

 *               reporting module.

 *

 *		Voxware Proprietary Material

 *		Copyright 1996, Voxware, Inc.

 *		All Rights Resrved

 *

 *		DISTRIBUTION PROHIBITED without

 *		written authorization from Voxware

 *

 ****************************************************************************/





#if (_DEBUG_MESSAGES == 1) || defined(_DEBUG)



#ifndef __TVGETSTR_H_

#define __TVGETSTR_H_



#ifdef __cplusplus

extern "C" {

#endif



/* Windows users must define VOXWARE_??? as a compiler option.  This will   */

/* enable system specific code.                                             */

#if defined (VOXWARE_WIN16) || (VOXWARE_WIN32)

    #define STRING_FORMAT   wsprintf



#elif defined(VOXWARE_MAC)

    #define STRING_FORMAT   sprintf



#elif defined(VOXWARE_HP)

    #define STRING_FORMAT   sprintf



#elif defined(VOXWARE_SUN)

    #define STRING_FORMAT   sprintf



#elif defined(VOXWARE_DOS)

    #define STRING_FORMAT   sprintf



#else



#pragma message ("TVGETSTR.H: Platform indicator #define not setup.")

#pragma message ("TVGETSTR.H: One of the following must be initialized:")

#pragma message ("TVGETSTR.H:      #define VOXWARE_WIN16")

#pragma message ("TVGETSTR.H:      #define VOXWARE_WIN32")

#pragma message ("TVGETSTR.H:      #define VOXWARE_MAC")

#pragma message ("TVGETSTR.H:      #define VOXWARE_SUN")

#pragma message ("TVGETSTR.H:      #define VOXWARE_HP")

#pragma message ("TVGETSTR.H:      #define VOXWARE_AIX")

#pragma message ("TVGETSTR.H:      #define VOXWARE_DOS")

#pragma message ("TVGETSTR.H: Check the Voxware manual for more information.")



#endif





#define TVGETSTR_MAX_STRING_LENGTH	512





void tvGetStringFromError(VOXWARE_RETCODE wVoxwareError, signed long dwReturnCode,

                          char VOX_FAR *lpMessage);









#ifdef __cplusplus

}

#endif



#endif /*__TVGETSTR_H_*/



#endif /* _DEBUG_MESSAGES */

