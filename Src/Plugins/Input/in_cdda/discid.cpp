#include "main.h"
#include "cddb.h"
#include <strsafe.h>

/*
* cddb_sum
*	Convert an integer to its text string representation, and
*	compute its checksum.  Used by cddb_discid to derive the
*	disc ID.
*
* Args:
*	n - The integer value.
*
* Return:
*	The integer checksum.
*/
int cddb_sum(int n)
{
	char	buf[12],
	*p;
	int	ret = 0;

	/* For backward compatibility this algorithm must not change */
	StringCchPrintfA(buf, 12, "%lu", n);
	for (p = buf; *p != '\0'; p++)
		ret += (*p - '0');

	return (ret);
}

/*
* cddb_discid
*	Compute a magic disc ID based on the number of tracks,
*	the length of each track, and a checksum of the string
*	that represents the offset of each track.
*
* Return:
*	The integer disc ID.
*/

unsigned long cddb_discid(unsigned char nTracks, unsigned int* pnMin, unsigned int* pnSec)
{
	int	i,
	t = 0,
	    n = 0;

	/* For backward compatibility this algorithm must not change */
	for (i = 0; i < (int) nTracks; i++)
	{
		n += cddb_sum((pnMin[i] * 60) + pnSec[i]);

		t += ((pnMin[i + 1] * 60) + pnSec[i + 1]) - ((pnMin[i] * 60) + pnSec[i]);
	}

	return ((n % 0xff) << 24 | t << 8 | nTracks);
}

// Functions used to generate the CDDB id

void CDGetEndFrame(MCIDEVICEID wDeviceID,
                   DINFO* psDI,
                   unsigned int nOffset,
                   unsigned int* pnFrame,
                   unsigned int* pnMin,
                   unsigned int* pnSec)
{
	MCI_STATUS_PARMS sMCIStatus;

	sMCIStatus.dwItem = MCI_STATUS_LENGTH;
	MCISendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT,
	               (DWORD_PTR)(LPVOID) &sMCIStatus);
#ifdef _WIN64
	*pnFrame = (unsigned int)((double)sMCIStatus.dwReturn * (75.0 / 1000.0));
#else
	{
		int nFrame;
		static double tmp = 75.0 / 1000.0;
		unsigned long a = sMCIStatus.dwReturn;
		__asm
		{
			fld qword ptr tmp
			fild dword ptr a
			fmul
			fistp dword ptr nFrame
		}
		*pnFrame = nFrame;
	}
#endif
	pnFrame[0] += 1 + nOffset; // Due to bug in MCI according to CDDB docs!

	psDI->nDiscLength = (pnFrame[0] / 75);

	*pnMin = pnFrame[0] / 75 / 60;
	*pnSec = (pnFrame[0] / 75) % 60;
}


void CDGetAbsoluteTrackPos(MCIDEVICEID wDeviceID,
                           unsigned int nTrack,
                           unsigned int* pnFrame,
                           unsigned int* pnMin,
                           unsigned int* pnSec)
{
	MCI_STATUS_PARMS sMCIStatus;

	sMCIStatus.dwItem = MCI_STATUS_POSITION;
	sMCIStatus.dwTrack = nTrack;
	MCISendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT,
	               (DWORD_PTR)(LPVOID) &sMCIStatus);
#ifdef _WIN64
	*pnFrame = (int)((double)sMCIStatus.dwReturn * (75.0 / 1000.0));
#else
	{
		static double tmp = 75.0 / 1000.0;
		unsigned long a = sMCIStatus.dwReturn;
		__asm
		{
			fld qword ptr tmp
			fild dword ptr a
			fmul
			fistp dword ptr a
		}
		*pnFrame = a;
	}
#endif


	*pnMin = *pnFrame / 75 / 60;
	*pnSec = (*pnFrame / 75) % 60;
}

int GetDiscID(MCIDEVICEID wDeviceID, DINFO* psDI)
{
	MCI_SET_PARMS sMCISet;
	unsigned int nLoop;
	unsigned int nMCITracks = CDGetTracks(wDeviceID);
	unsigned int* pnMin = NULL;
	unsigned int* pnSec = NULL;

	if (nMCITracks > 65535) return 1;

	if (nMCITracks > 128) nMCITracks = 128;
	psDI->ntracks = nMCITracks;

	pnMin = (unsigned int*)GlobalAlloc(GPTR, (nMCITracks + 1) * 2 * sizeof(unsigned int));
	if (!pnMin) return 1;

	pnSec = pnMin + (nMCITracks + 1);

	sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	MCISendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPVOID) &sMCISet);

	for (nLoop = 0 ; nLoop < nMCITracks; nLoop ++)
		CDGetAbsoluteTrackPos(wDeviceID, nLoop + 1, &psDI->pnFrames[nLoop], &pnMin[nLoop], &pnSec[nLoop]);

	CDGetEndFrame(wDeviceID, psDI, psDI->pnFrames[0], &psDI->pnFrames[nLoop], &pnMin[nLoop], &pnSec[nLoop]);

	psDI->CDDBID = cddb_discid((unsigned char)nMCITracks, pnMin, pnSec);

	sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	MCISendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPVOID) &sMCISet);

	if (pnMin)
	{
		GlobalFree(pnMin);
	}
	return 0;
}