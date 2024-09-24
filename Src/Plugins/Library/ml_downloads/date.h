#ifndef _OD_DATE_
#define _OD_DATE_

#include <time.h>



/* These functions all work with "Internet"(RFC 822) format Date/Time strings only */
/* An example of an RFC 822 format Date/Time string is "Thu, 28 Aug 2003 21:30:47 EDT" */

/* converts the RFC 822 format date string into UTC Calendar time */
time_t getDateSecs(char *date);

/* returns a string that represents the given UTC Calendar time value as an
   RFC 822 format string.  The buffer is user-supplied and must be at least 
   30 bytes in size. */
char *getDateStr(time_t tmval, char *buffer, int gmt);

#endif /*_OD_DATE_*/

