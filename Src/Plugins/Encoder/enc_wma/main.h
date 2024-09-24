#include <windows.h>
#include <stdio.h>

#include "../nsv/enc_if.h"
#include "resource.h"

// LGIVEN Mods 4-25-05
// Config info saved in Winamp.ini [enc_wma]---conf=xxxxxxxxxxx
typedef struct
{
	int config_nch;			// Number of channels of encoder/fmt selected
	int config_bitrate;     // Bitrate of  encoder/fmt selected
	int config_bitsSample;  // Bits/Sample of encoder/fmt selected
	int config_samplesSec;  // Sample rate of encoder/fmt selected
	int config_encoder;     // Encoder offset in table in Config Dialog
	BOOL config_vbr;			// VBR or not
	DWORD config_passes; // number of passes (1 or 2)
}
configtype;

typedef struct
{
	configtype cfg;         // config type struct
	char *configfile;       // Name of config file (...\Winamp.ini)
}
configwndrec;

// Data table values in Config Dialog
// One of these for each format

struct formatType
{
	wchar_t *formatName;       // Format Name (for display)
	int offset;            // offset in WMEncoder for this Encoder
	int nChannels;         // number of channels
	int bitsSample;        // Bits per sample
	int samplesSec;        // Samples per sec
	int bitrate;           // Bitrate value
	int vbr;
};

// One of these for each encoder
struct EncoderType
{
	wchar_t *encoderName;       // Encoder name (for display)
	int offset;             // Offset in WMEncoder
	int numFormats;         // Number of formats in WMEncoder for this encoder
	struct _GUID mediaType; // Media type GUID
	BOOL vbr; 
	DWORD numPasses;
	formatType* formats;
};


BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
