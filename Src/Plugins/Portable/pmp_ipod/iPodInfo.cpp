#include "api.h"
#include "iPodInfo.h"
#include "resource.h"
#include "../../General/gen_ml/ml.h"
#include "../../Library/ml_pmp/pmp.h"

#include "../xml/obj_xml.h"
#include "../plist/loader.h"

#include <api/service/waservicefactory.h>

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>

extern PMPDevicePlugin plugin;

static const ArtworkFormat ipod_color_artwork_info[] =
{
	{THUMB_COVER_SMALL,        56,  56, 1017, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       140, 140, 1016, RGB_565, 4, 4},
	{THUMB_PHOTO_TV_SCREEN,   720, 480, 1019, RGB_565, 4, 4},
	{THUMB_PHOTO_LARGE,       130,  88, 1015, RGB_565, 4, 4},
	{THUMB_PHOTO_FULL_SCREEN, 220, 176, 1013, RGB_565, 4, 4},
	{THUMB_PHOTO_SMALL,        42,  30, 1009, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_nano_artwork_info[] =
{
	{THUMB_COVER_SMALL,        42,  42, 1031, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       100, 100, 1027, RGB_565, 4, 4},
	{THUMB_PHOTO_LARGE,        42,  37, 1032, RGB_565, 4, 4},
	{THUMB_PHOTO_FULL_SCREEN, 176, 132, 1023, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_video_artwork_info[] =
{
	{THUMB_COVER_SMALL,       100, 100, 1028, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       200, 200, 1029, RGB_565, 4, 4},
	{THUMB_PHOTO_TV_SCREEN,   720, 480, 1019, RGB_565, 4, 4},
	{THUMB_PHOTO_LARGE,       130,  88, 1015, RGB_565, 4, 4},
	{THUMB_PHOTO_FULL_SCREEN, 320, 240, 1024, RGB_565, 4, 4},
	{THUMB_PHOTO_SMALL,        50,  41, 1036, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_7g_artwork_info[] =
{
	{THUMB_COVER_SMALL,        55,  55, 1061, RGB_565, 4, 4},
	{THUMB_COVER_MEDIUM1,     128, 128, 1055, RGB_565, 4, 4},
	{THUMB_COVER_LARGE,       320, 320, 1060, RGB_565, 4, 4},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
};

static const ArtworkFormat ipod_touch_artwork_info[] =
{
	{THUMB_COVER_SMALL,        55,  55, 3006, RGB_555, 16, 4096},
	{THUMB_COVER_MEDIUM1,      64,  64, 3003, RGB_555_REC, 16, 4096},
	{THUMB_COVER_MEDIUM2,      88,  88, 3007, RGB_555, 16, 4096},
	{THUMB_COVER_MEDIUM3,     128, 128, 3002, RGB_555_REC, 16, 4096},
	{THUMB_COVER_MEDIUM4,     256, 256, 3001, RGB_555_REC, 16, 4096},
	{THUMB_COVER_LARGE,       320, 320, 3005, RGB_555, 16, 4096},
	{THUMB_INVALID,            -1,  -1,   -1, RGB_555, 4, 4}
};

/*
static const ArtworkFormat ipod_mobile_1_artwork_info[] = {
	{THUMB_COVER_SMALL,        50,  50, 2002},
	{THUMB_COVER_LARGE,       150, 150, 2003},
	{THUMB_INVALID,            -1,  -1,   -1}
};
*/

//maps model to artwork format
static const ArtworkFormat *ipod_artwork_info_table[] =
{
	NULL,                      // invalid
	ipod_color_artwork_info,   // color
	NULL,                      // regular
	NULL,                      // mini
	NULL,                      // shuffle
	ipod_video_artwork_info,   // video
	ipod_nano_artwork_info,    // nano
	ipod_7g_artwork_info,      // classic
	ipod_7g_artwork_info,      // fat nano
	ipod_touch_artwork_info,   // touch
};


// this list compiled from http://www.thismuchiknow.co.uk/?page_id=27 and is kept in the same order as that table for easy updating
// when new ipods come out, let's keep this up to date.
// at the moment this is just used as a mapping from part number to model, for album art
static const iPodModelInfo ipod_info_table[] =
{
	//1st gen ipods
	{L"8513",   IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //mac
	{L"8541",   IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //mac
	{L"8697",   IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //pc
	{L"8709",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //mac

	//2nd gen ipods
	{L"8737",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //mac
	{L"8740",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //pc
	{L"8738",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //mac
	{L"8741",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //pc

	//3rd gen ipods
	{L"8976",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"8946",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"8948",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9244",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9245",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9460",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	//1st gen mini
	{L"9160",   IPOD_MODEL_MINI,    IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9436",   IPOD_MODEL_MINI,    IPOD_COLOR_BLUE,   IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9435",   IPOD_MODEL_MINI,    IPOD_COLOR_PINK,   IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9434",   IPOD_MODEL_MINI,    IPOD_COLOR_GREEN,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9437",   IPOD_MODEL_MINI,    IPOD_COLOR_GOLD,   IDB_CLASSIC_16, IDB_CLASSIC_160},

	//4th gen ipods
	{L"9282",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9268",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"E435",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //HP branded
	{L"E436",  IPOD_MODEL_REGULAR, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //HP	branded
	{L"9787",  IPOD_MODEL_REGULAR, IPOD_COLOR_U2,     IDB_CLASSIC_16, IDB_CLASSIC_160},

	//4th gen ipod photos
	{L"9585",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9586",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	//shuffles
	{L"A133", IPOD_MODEL_SHUFFLE, IPOD_COLOR_WHITE,  IDB_SHUFFLE1G_16, IDB_SHUFFLE1G_160},
	{L"9724", IPOD_MODEL_SHUFFLE, IPOD_COLOR_WHITE,  IDB_SHUFFLE1G_16, IDB_SHUFFLE1G_160},
	{L"9725",   IPOD_MODEL_SHUFFLE, IPOD_COLOR_WHITE,  IDB_SHUFFLE1G_16, IDB_SHUFFLE1G_160},

	// more ipod photos
	{L"9829",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9830",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	// ipod mini 2nd gen
	{L"9959",   IPOD_MODEL_MINI,    IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160}, // pepsi giveaway ipod
	{L"9800",   IPOD_MODEL_MINI,    IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9802",   IPOD_MODEL_MINI,    IPOD_COLOR_BLUE,   IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9804",   IPOD_MODEL_MINI,    IPOD_COLOR_PINK,   IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9806",   IPOD_MODEL_MINI,    IPOD_COLOR_GREEN,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9801",   IPOD_MODEL_MINI,    IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9803",   IPOD_MODEL_MINI,    IPOD_COLOR_BLUE,   IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9805",   IPOD_MODEL_MINI,    IPOD_COLOR_PINK,   IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"9807",   IPOD_MODEL_MINI,    IPOD_COLOR_GREEN,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	//HP colour ipods
	{L"S492",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"S493",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	//HP ipod mini 2nd gen
	{L"W753",   IPOD_MODEL_MINI,    IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"X762",   IPOD_MODEL_MINI,    IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160},

	//more 4th gen ipod photos
	{L"A079",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A127",  IPOD_MODEL_COLOR,   IPOD_COLOR_U2,     IDB_CLASSIC_16, IDB_CLASSIC_160},

	//HP ipod shuffles
	{L"X765", IPOD_MODEL_SHUFFLE, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"X766",   IPOD_MODEL_SHUFFLE, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	/*
	//harry potter ipod 4G, don't know serial number. but that's ok because it was only on sale for a month, so fuck it.
	{L"????",  IPOD_MODEL_COLOR,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	*/

	//ipod nano 1st gen
	{L"A004",   IPOD_MODEL_NANO,    IPOD_COLOR_WHITE,  IDB_NANO1G_16, IDB_NANO1G_160},
	{L"A099",   IPOD_MODEL_NANO,    IPOD_COLOR_BLACK,  IDB_NANO1G_16, IDB_NANO1G_160},
	{L"A005",   IPOD_MODEL_NANO,    IPOD_COLOR_WHITE,  IDB_NANO1G_16, IDB_NANO1G_160},
	{L"A107",   IPOD_MODEL_NANO,    IPOD_COLOR_BLACK,  IDB_NANO1G_16, IDB_NANO1G_160},

	//ipod video
	{L"A002",  IPOD_MODEL_VIDEO,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A146",  IPOD_MODEL_VIDEO,   IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A003",  IPOD_MODEL_VIDEO,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A147",  IPOD_MODEL_VIDEO,   IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A253",  IPOD_MODEL_VIDEO,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160}, //harry potter ipod 5G

	//1gig nano
	{L"A350",   IPOD_MODEL_NANO,    IPOD_COLOR_WHITE,  IDB_NANO1G_16, IDB_NANO1G_160},
	{L"A352",   IPOD_MODEL_NANO,    IPOD_COLOR_BLACK,  IDB_NANO1G_16, IDB_NANO1G_160},

	// U2 ipod video
	{L"A452",  IPOD_MODEL_VIDEO,   IPOD_COLOR_U2,     IDB_CLASSIC_16, IDB_CLASSIC_160},

	//2nd gen nano
	{L"A477",   IPOD_MODEL_NANO,    IPOD_COLOR_SILVER, IDB_NANO2G_16, IDB_NANO2G_160},
	{L"A426",   IPOD_MODEL_NANO,    IPOD_COLOR_SILVER, IDB_NANO2G_16, IDB_NANO2G_160},
	{L"A428",   IPOD_MODEL_NANO,    IPOD_COLOR_BLUE,   IDB_NANO2G_16, IDB_NANO2G_160},
	{L"A487",   IPOD_MODEL_NANO,    IPOD_COLOR_GREEN,  IDB_NANO2G_16, IDB_NANO2G_160},
	{L"A489",   IPOD_MODEL_NANO,    IPOD_COLOR_PINK,   IDB_NANO2G_16, IDB_NANO2G_160},
	{L"A497",   IPOD_MODEL_NANO,    IPOD_COLOR_BLACK,  IDB_NANO2G_16, IDB_NANO2G_160},

	// ipod video 6th gen
	{L"A444",  IPOD_MODEL_VIDEO,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A446",  IPOD_MODEL_VIDEO,   IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A448",  IPOD_MODEL_VIDEO,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A450",  IPOD_MODEL_VIDEO,   IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	//2nd gen shuffle
	{L"A564",   IPOD_MODEL_SHUFFLE, IPOD_COLOR_SILVER, IDB_SHUFFLE2G_16, IDB_SHUFFLE2G_160},

	// ipod video u2 6th gen
	{L"A664",  IPOD_MODEL_VIDEO,   IPOD_COLOR_U2,     IDB_CLASSIC_16, IDB_CLASSIC_160},

	//product red ipod nano
	{L"A725",   IPOD_MODEL_NANO,    IPOD_COLOR_RED,    IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A899",   IPOD_MODEL_NANO,    IPOD_COLOR_RED,    IDB_CLASSIC_16, IDB_CLASSIC_160},

	// coloured versions of ipod shuffle 2nd gen
	{L"A947",   IPOD_MODEL_SHUFFLE, IPOD_COLOR_PINK,   IDB_SHUFFLE2G_16, IDB_SHUFFLE2G_160},
	{L"A949",   IPOD_MODEL_SHUFFLE, IPOD_COLOR_BLUE,   IDB_SHUFFLE2G_16, IDB_SHUFFLE2G_160},
	{L"A951",   IPOD_MODEL_SHUFFLE, IPOD_COLOR_GREEN,  IDB_SHUFFLE2G_16, IDB_SHUFFLE2G_160},
	{L"A953",   IPOD_MODEL_SHUFFLE, IPOD_COLOR_ORANGE, IDB_SHUFFLE2G_16, IDB_SHUFFLE2G_160},

	// fat nanos
	{L"A978",   IPOD_MODEL_FATNANO, IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A980",   IPOD_MODEL_FATNANO, IPOD_COLOR_SILVER, IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"B249",   IPOD_MODEL_FATNANO, IPOD_COLOR_BLUE,   IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"B253",   IPOD_MODEL_FATNANO, IPOD_COLOR_GREEN,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"B261",   IPOD_MODEL_FATNANO, IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"B257",   IPOD_MODEL_FATNANO, IPOD_COLOR_RED,    IDB_CLASSIC_16, IDB_CLASSIC_160},

	// ipod classic
	{L"B147",  IPOD_MODEL_CLASSIC, IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"B029",  IPOD_MODEL_CLASSIC, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"B150", IPOD_MODEL_CLASSIC, IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"B145", IPOD_MODEL_CLASSIC, IPOD_COLOR_WHITE,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	// ipod touch
	{L"A623",   IPOD_MODEL_TOUCH,   IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},
	{L"A627",  IPOD_MODEL_TOUCH,   IPOD_COLOR_BLACK,  IDB_CLASSIC_16, IDB_CLASSIC_160},

	//insert info about new models here (be sure to take first char off the product code)...
};

static const iPodModelInfo 
shuffle1g_info = {L"XXXX", IPOD_MODEL_SHUFFLE, IPOD_COLOR_PINK,   IDB_SHUFFLE1G_16, IDB_SHUFFLE1G_160},
shuffle2g_info = {L"XXXX", IPOD_MODEL_SHUFFLE, IPOD_COLOR_PINK,   IDB_SHUFFLE2G_16, IDB_SHUFFLE2G_160},
shuffle3g_info = {L"XXXX", IPOD_MODEL_SHUFFLE, IPOD_COLOR_SILVER, IDB_SHUFFLE3G_16, IDB_SHUFFLE3G_160},
shuffle4g_info = {L"XXXX", IPOD_MODEL_SHUFFLE, IPOD_COLOR_SILVER, IDB_SHUFFLE4G_16, IDB_SHUFFLE4G_160},
classic_info   = {L"XXXX", IPOD_MODEL_CLASSIC, IPOD_COLOR_WHITE,  IDB_CLASSIC_16,   IDB_CLASSIC_160},
video_info     = {L"XXXX", IPOD_MODEL_VIDEO,   IPOD_COLOR_WHITE,  IDB_CLASSIC_16,   IDB_CLASSIC_160},
nano1g_info    = {L"XXXX", IPOD_MODEL_NANO,    IPOD_COLOR_WHITE,  IDB_NANO1G_16,    IDB_NANO1G_160},
nano2g_info    = {L"XXXX", IPOD_MODEL_NANO,    IPOD_COLOR_WHITE,  IDB_NANO2G_16,    IDB_NANO2G_160},
nano3g_info    = {L"XXXX", IPOD_MODEL_NANO,    IPOD_COLOR_WHITE,  IDB_NANO3G_16,    IDB_NANO3G_160},
nano4g_info    = {L"XXXX", IPOD_MODEL_NANO,    IPOD_COLOR_WHITE,  IDB_NANO4G_16,    IDB_NANO4G_160};

static INT_PTR CALLBACK selectipodtype_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		BringWindowToTop(hwndDlg);
		wchar_t * sysinfo = (wchar_t*)lParam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
		wchar_t path[] = {sysinfo[0],L":\\"};
		wchar_t name[32] = {0};
		GetVolumeInformation(path,name,32,NULL,NULL,NULL,NULL,0);
		wchar_t buf[100] = {0};
		wchar_t s[32] = {0};
		GetDlgItemText(hwndDlg,IDC_IPODINFO,s,32);
		StringCchPrintf(buf,100,L"%s (%s) %s",path,name,s);
		SetDlgItemText(hwndDlg,IDC_IPODINFO,buf);
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
		case IDC_RADIO4:
		case IDC_RADIO5:
		case IDC_RADIO6:
		case IDC_RADIO7:
		case IDC_RADIO8:
			EnableWindow(GetDlgItem(hwndDlg,IDOK),TRUE);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,1);
			break;
		case IDOK:
		{
			char *m;
			if (IsDlgButtonChecked(hwndDlg,IDC_RADIO1)) m = "A133"; //shuffle
			else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO2)) m = "9586"; //photo
			else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO3)) m = "A002"; //video
			else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO4)) m = "A005"; //nano
			else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO6)) m = "A623"; //touch
			else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO7)) m = "B145"; //classic
			else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO8)) m = "B257"; //fatnano
			else m = "8976"; //other
			wchar_t * sysinfo = (wchar_t*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			FILE *f = _wfopen(sysinfo,L"a+b");
			if (f)
			{
				fprintf(f,"ModelNumStr: M%s\n",m);
				fclose(f);
			}
		}
		EndDialog(hwndDlg,0);
		break;
		}
		break;
	}
	return 0;
}

/* This table was extracted from ipod-model-table from podsleuth svn trunk
 * on 2008-06-14 (which seems to match podsleuth 0.6.2)
*/
static const iPodSerialToModel serial_to_model_mapping[] =
{
	{ L"LG6", L"8541" },
	{ L"NAM", L"8541" },
	{ L"MJ2", L"8541" },
	{ L"ML1", L"8709" },
	{ L"MME", L"8709" },
	{ L"MMB", L"8737" },
	{ L"MMC", L"8738" },
	{ L"NGE", L"8740" },
	{ L"NGH", L"8740" },
	{ L"MMF", L"8741" },
	{ L"NLW", L"8946" },
	{ L"NRH", L"8976" },
	{ L"QQF", L"9460" },
	{ L"PQ5", L"9244" },
	{ L"PNT", L"9244" },
	{ L"NLY", L"8948" },
	{ L"NM7", L"8948" },
	{ L"PNU", L"9245" },
	{ L"PS9", L"9282" },
	{ L"Q8U", L"9282" },
	{ L"V9V", L"9787" },
	{ L"S2X", L"9787" },
	{ L"PQ7", L"9268" },
	{ L"TDU", L"A079" },
	{ L"TDS", L"A079" },
	{ L"TM2", L"A127" },
	{ L"SAZ", L"9830" },
	{ L"SB1", L"9830" },
	{ L"SAY", L"9829" },
	{ L"R5Q", L"9585" },
	{ L"R5R", L"9586" },
	{ L"R5T", L"9586" },
	{ L"PFW", L"9160" },
	{ L"PRC", L"9160" },
	{ L"QKL", L"9436" },
	{ L"QKQ", L"9436" },
	{ L"QKK", L"9435" },
	{ L"QKP", L"9435" },
	{ L"QKJ", L"9434" },
	{ L"QKN", L"9434" },
	{ L"QKM", L"9437" },
	{ L"QKR", L"9437" },
	{ L"S41", L"9800" },
	{ L"S4C", L"9800" },
	{ L"S43", L"9802" },
	{ L"S45", L"9804" },
	{ L"S47", L"9806" },
	{ L"S4J", L"9806" },
	{ L"S42", L"9801" },
	{ L"S44", L"9803" },
	{ L"S48", L"9807" },
	{ L"RS9", L"9724" },
	{ L"QGV", L"9724" },
	{ L"TSX", L"9724" },
	{ L"PFV", L"9724" },
	{ L"R80", L"9724" },
	{ L"RSA", L"9725" },
	{ L"TSY", L"9725" },
	{ L"C60", L"9725" },
	{ L"VTE", L"A546" },
	{ L"VTF", L"A546" },
	{ L"XQ5", L"A947" },
	{ L"XQS", L"A947" },
	{ L"XQV", L"A949" },
	{ L"XQX", L"A949" },
	{ L"YX7", L"A949" },
	{ L"XQY", L"A951" },
	{ L"YX8", L"A951" },
	{ L"XR1", L"A953" },
	{ L"YXA", L"B233" },
	{ L"YX6", L"B225" },
	{ L"YX7", L"B228" },
	{ L"YX9", L"B225" },
	{ L"UNA", L"A350" },
	{ L"UNB", L"A350" },
	{ L"UPR", L"A352" },
	{ L"UPS", L"A352" },
	{ L"SZB", L"A004" },
	{ L"SZV", L"A004" },
	{ L"SZW", L"A004" },
	{ L"SZC", L"A005" },
	{ L"SZT", L"A005" },
	{ L"TJT", L"A099" },
	{ L"TJU", L"A099" },
	{ L"TK2", L"A107" },
	{ L"TK3", L"A107" },
	{ L"VQ5", L"A477" },
	{ L"VQ6", L"A477" },
	{ L"V8T", L"A426" },
	{ L"V8U", L"A426" },
	{ L"V8W", L"A428" },
	{ L"V8X", L"A428" },
	{ L"VQH", L"A487" },
	{ L"VQJ", L"A487" },
	{ L"VQK", L"A489" },
	{ L"VKL", L"A489" },
	{ L"WL2", L"A725" },
	{ L"WL3", L"A725" },
	{ L"X9A", L"A726" },
	{ L"X9B", L"A726" },
	{ L"VQT", L"A497" },
	{ L"VQU", L"A497" },
	{ L"Y0P", L"A978" },
	{ L"Y0R", L"A980" },
	{ L"YXR", L"B249" },
	{ L"YXV", L"B257" },
	{ L"YXT", L"B253" },
	{ L"YXX", L"B261" },
	{ L"SZ9", L"A002" },
	{ L"WEC", L"A002" },
	{ L"WED", L"A002" },
	{ L"WEG", L"A002" },
	{ L"WEH", L"A002" },
	{ L"WEL", L"A002" },
	{ L"TXK", L"A146" },
	{ L"TXM", L"A146" },
	{ L"WEE", L"A146" },
	{ L"WEF", L"A146" },
	{ L"WEJ", L"A146" },
	{ L"WEK", L"A146" },
	{ L"SZA", L"A003" },
	{ L"SZU", L"A003" },
	{ L"TXL", L"A147" },
	{ L"TXN", L"A147" },
	{ L"V9K", L"A444" },
	{ L"V9L", L"A444" },
	{ L"WU9", L"A444" },
	{ L"VQM", L"A446" },
	{ L"V9M", L"A446" },
	{ L"V9N", L"A446" },
	{ L"WEE", L"A446" },
	{ L"V9P", L"A448" },
	{ L"V9Q", L"A448" },
	{ L"V9R", L"A450" },
	{ L"V9S", L"A450" },
	{ L"V95", L"A450" },
	{ L"V96", L"A450" },
	{ L"WUC", L"A450" },
	{ L"W9G", L"A664" }, /* 30GB iPod Video U2 5.5g */
	{ L"Y5N", L"B029" }, /* Silver Classic 80GB */
	{ L"YMV", L"B147" }, /* Black Classic 80GB */
	{ L"YMU", L"B145" }, /* Silver Classic 160GB */
	{ L"YMX", L"B150" }, /* Black Classic 160GB */
	{ L"2C5", L"B562" }, /* Silver Classic 120GB */
	{ L"2C7", L"B565" }, /* Black Classic 120GB */
	{ L"9ZS", L"C293" }, /* Silver Classic 160GB (2009) */
	{ L"9ZU", L"C297" }, /* Black Classic 160GB (2009) */

	{ L"37P", L"B663" }, /* 4GB Green Nano 4g */
	{ L"37Q", L"B666" }, /* 4GB Yellow Nano 4g */
	{ L"37H", L"B654" }, /* 4GB Pink Nano 4g */
	{ L"1P1", L"B480" }, /* 4GB Silver Nano 4g */
	{ L"37K", L"B657" }, /* 4GB Purple Nano 4g */
	{ L"37L", L"B660" }, /* 4GB Orange Nano 4g */
	{ L"2ME", L"B598" }, /* 8GB Silver Nano 4g */
	{ L"3QS", L"B732" }, /* 8GB Blue Nano 4g */
	{ L"3QT", L"B735" }, /* 8GB Pink Nano 4g */
	{ L"3QU", L"B739" }, /* 8GB Purple Nano 4g */
	{ L"3QW", L"B742" }, /* 8GB Orange Nano 4g */
	{ L"3QX", L"B745" }, /* 8GB Green Nano 4g */
	{ L"3QY", L"B748" }, /* 8GB Yellow Nano 4g */
	{ L"3R0", L"B754" }, /* 8GB Black Nano 4g */
	{ L"3QZ", L"B751" }, /* 8GB Red Nano 4g */
	{ L"5B7", L"B903" }, /* 16GB Silver Nano 4g */
	{ L"5B8", L"B905" }, /* 16GB Blue Nano 4g */
	{ L"5B9", L"B907" }, /* 16GB Pink Nano 4g */
	{ L"5BA", L"B909" }, /* 16GB Purple Nano 4g */
	{ L"5BB", L"B911" }, /* 16GB Orange Nano 4g */
	{ L"5BC", L"B913" }, /* 16GB Green Nano 4g */
	{ L"5BD", L"B915" }, /* 16GB Yellow Nano 4g */
	{ L"5BE", L"B917" }, /* 16GB Red Nano 4g */
	{ L"5BF", L"B918" }, /* 16GB Black Nano 4g */

	{ L"71V", L"C027" }, /* 8GB Silver Nano 5g */
	{ L"71Y", L"C031" }, /* 8GB Black Nano 5g */
	{ L"721", L"C034" }, /* 8GB Purple Nano 5g */
	{ L"726", L"C037" }, /* 8GB Blue Nano 5g */
	{ L"72A", L"C040" }, /* 8GB Green Nano 5g */
	{ L"72F", L"C046" }, /* 8GB Orange Nano 5g */
	{ L"72L", L"C050" }, /* 8GB Pink Nano 5g */

	{ L"72Q", L"C060" }, /* 16GB Silver Nano 5g */
	{ L"72R", L"C062" }, /* 16GB Black Nano 5g */
	{ L"72S", L"C064" }, /* 16GB Purple Nano 5g */
	{ L"72X", L"C066" }, /* 16GB Blue Nano 5g */
	{ L"734", L"C068" }, /* 16GB Green Nano 5g */
	{ L"738", L"C070" }, /* 16GB Yellow Nano 5g */
	{ L"739", L"C072" }, /* 16GB Orange Nano 5g */
	{ L"73A", L"C074" }, /* 16GB Red Nano 5g */
	{ L"73B", L"C075" }, /* 16GB Pink Nano 5g */

	{ L"4NZ", L"B867" }, /* 4GB Silver Shuffle 4g */
	{ L"891", L"C164" }, /* 4GB Black Shuffle 4g */

	{ L"W4T", L"A627" }, /* 16GB Silver iPod Touch (1st gen) */
	{ L"0JW", L"B376" }, /* 32GB Silver iPod Touch (1st gen) */
	{ L"201", L"B528" }, /* 8GB Silver iPod Touch (2nd gen) */
	{ L"203", L"B531" }, /* 16GB Silver iPod Touch (2nd gen) */
	{ L"75J", L"C086" }, /* 8GB Silver iPod Touch (3rd gen) */
	{ L"6K2", L"C008" }, /* 32GB Silver iPod Touch (3rd gen) */
	{ L"6K4", L"C011" }, /* 64GB Silver iPod Touch (3rd gen) */

	{ L"VR0", L"A501" }, /* 4GB Silver iPhone 1st gen */
	{ L"WH8", L"A712" }, /* 8GB Silver iPhone */
	{ L"0KH", L"B384" }, /* 16GB Silver iPhone */
	{ L"Y7H", L"B046" }, /* 8GB Black iPhone 3G */
	{ L"Y7K", L"B496" }, /* 16GB Black iPhone 3G */
	{ L"3NP", L"C131" }, /* 16GB Black iPhone 3GS */
	{ L"3NR", L"C133" }  /* 32GB Black iPhone 3GS */
};

static const wchar_t *GetModelStrForFamilyID(unsigned int familyID)
{
	switch (familyID)
	{
	case 4: // iPod 4
		return L"9282";
	case 5: // iPod 4 (photo)
		return L"9830";
	case 6: // iPod 5
		return L"A002";
	case 7: // nano 1
		return L"A004";
	case 9: // nano 2
		return L"A477";
	case 11: // classic
		return L"B147";
	case 12: // fat nano
		return L"A978";
	case 128: // shuffle
		return L"A133";
	case 130: // shuffle 2
		return L"A947";
	default:
		return 0;
	}
}

static const iPodModelInfo *GetiPodInfoForModelStr(const wchar_t *modelstr)
{
	// now locate this ipod in our table
	int l = sizeof(ipod_info_table)/sizeof(ipod_info_table[0]);
	for (int i=0; i<l; i++)
	{
		if (_wcsnicmp(ipod_info_table[i].model_number,modelstr,wcslen(ipod_info_table[i].model_number))==0)
			return &ipod_info_table[i]; // success!
	}
	return 0;
}

static const iPodModelInfo *GetiPodInfoForFamilyID(unsigned int familyID)
{
	switch(familyID)
	{
	case 6:
		return &video_info;
	case 7:
		return &nano1g_info;
	case 9:
		return &nano2g_info;
	case 11:
		return &classic_info;
	case 12:
		return &nano3g_info;
	case 15:
		return &nano4g_info;
	case 128: 
		return &shuffle1g_info;
	case 130: // shuffle 2G
		return &shuffle2g_info;
	case 132: // shuffle 3G
		return &shuffle3g_info;
	case 133: // shuffle 4G
		return &shuffle4g_info;
	}
	return 0;
}

const wchar_t* GetModelStrForSerialNumber(const wchar_t *serialNumber)
{
	// now locate this ipod in our table
	int l = sizeof(serial_to_model_mapping)/sizeof(iPodSerialToModel);

	INT serialNumberLen = lstrlen(serialNumber);

	if (serialNumberLen < 3)
	{
		return NULL;
	}

	const wchar_t *last3OfSerialNumber = &serialNumber[serialNumberLen-3];

	for (int i=0; i<l; i++)
	{
		int compareRet = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, last3OfSerialNumber, -1, serial_to_model_mapping[i].serial, -1)-2;
		if (compareRet==0)
			return serial_to_model_mapping[i].model_number; // success!
	}
	return 0;
}

extern bool ParseSysInfoXML(wchar_t drive_letter, char * xml, int xmllen);

iPodInfo::iPodInfo(const iPodModelInfo *_model)
{
	family_id = 0;
	color = _model->color;
	model = _model->model;
	model_number = _wcsdup(_model->model_number);
	image16 = _model->image16;
	image160 = _model->image160;
	fwid=0;
	supportedArtworkFormats=0;
	numberOfSupportedFormats=0;
	shadow_db_version=0;
}

iPodInfo::~iPodInfo()
{
	free(fwid);
	free(model_number);
	delete supportedArtworkFormats;
}

void iPodInfo::SetFWID(const uint8_t *new_fwid)
{
	fwid = (uint8_t *)malloc(8);
	memcpy(fwid, new_fwid, 8);
}

iPodInfo *GetiPodInfo(wchar_t drive)
{
	static const iPodModelInfo unknown = {NULL, IPOD_MODEL_INVALID, IPOD_COLOR_WHITE};

	unsigned char fwid[8]={0};
	bool have_fwid=false;
	char xml[65536] = {0};
	if (ParseSysInfoXML(drive, xml, sizeof(xml)/sizeof(char)))
	{
		// go fetch the FamilyID so we can construct a model string
		DWORD bytesRead = strlen(xml);//sizeof(xml)/sizeof(char);

		// use the plist handler here instead of fishing for the familyid string
		// in the xml

		// instantiate the plist loader
		plistLoader it;

		obj_xml *parser=0;
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
		if (factory)
		{
			parser = (obj_xml *)factory->getInterface();
		}

		if (parser)
		{
			// load the XML, this creates an iTunes DB in memory, and returns the	root key
			parser->xmlreader_open();
			parser->xmlreader_registerCallback(L"plist\f*", &it);
			parser->xmlreader_feed(xml, bytesRead);
			parser->xmlreader_feed(0, 0);
			parser->xmlreader_unregisterCallback(&it);
			parser->xmlreader_close();
			plistKey *root_key = &it;
			plistData *root_dict = root_key->getData();
			if (root_dict)
			{
				// get Firewire ID
				plistKey *fwidKey = ((plistDict*)root_dict)->getKey(L"FireWireGUID");
				if (fwidKey)
				{
					plistData *fwidData = fwidKey->getData();
					if (fwidData)
					{
						const wchar_t* p = fwidData->getString();
						for (int i=0; i<8 && *p; i++)
						{
							char num[3]={0,0,0};
							num[0] = *(p++);
							num[1] = *(p++);
							fwid[i] = (uint8_t)strtoul(num,NULL,16);
						}
						have_fwid=true;
					}
				}


				// check for the existance of sqlite
				plistKey *sqliteKey = ((plistDict*)root_dict)->getKey(L"SQLiteDB");
				if (sqliteKey)
				{
					plistData *sqliteData = sqliteKey->getData();
					if (sqliteData)
					{
						const wchar_t* sqliteString = sqliteData->getString();
						if (sqliteString)
						{
							int compareRet = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, sqliteString, -1, L"1", -1)-2;

							// At this point we dont want to support the sqlite family of ipods
							// so, return unknown if sqlite found
							if (compareRet == 0)
							{
								return 0;
							}
						}
					}
				} // end sqlite check

				// check for FamilyID
				plistKey *familyKey = ((plistDict*)root_dict)->getKey(L"FamilyID");
				if (familyKey)
				{
					plistData *familyData = familyKey->getData();
					if (familyData)
					{
						const wchar_t* familyIDString = familyData->getString();
						if (familyIDString)
						{
							const wchar_t *modelStr = NULL;
							unsigned int familyID = _wtoi(familyIDString);
							// first, try to look up the iPod by family ID
							const iPodModelInfo *info = GetiPodInfoForFamilyID(familyID);
							if (!info)
							{
								modelStr = GetModelStrForFamilyID(familyID);

								// if modelString not apparent, as the case is in most
								// 5th gen nanos and classics
								if (!info && !modelStr)
								{
									plistKey *serialNumberKey = ((plistDict*)root_dict)->getKey(L"SerialNumber");
									if (serialNumberKey)
									{
										plistData *serialNumberData = serialNumberKey->getData();
										if (serialNumberData)
										{
											const wchar_t* serialNumberString = serialNumberData->getString();

											if (serialNumberString)
											{
												modelStr = GetModelStrForSerialNumber(serialNumberString);
											}
										}
									}
								}
							}

							if (modelStr || info)
							{
								if (!info)
									info = GetiPodInfoForModelStr(modelStr);

								if (info)
								{
									iPodInfo* retInfo = new iPodInfo(info);

									if (have_fwid)
										retInfo->SetFWID(fwid);
									
									plistKey *shadow_db_key = ((plistDict*)root_dict)->getKey(L"ShadowDB");
									if (shadow_db_key)
									{
										plistData *shadow_db_data = shadow_db_key->getData();
										if (shadow_db_data && shadow_db_data->getType() == PLISTDATA_BOOLEAN)
										{
											plistBoolean *shadow_db_boolean = (plistBoolean *)shadow_db_data;
											if (shadow_db_boolean->getValue())
												retInfo->shadow_db_version = 1;

											plistKey *shadow_db_version_key = ((plistDict*)root_dict)->getKey(L"ShadowDBVersion");
											if (shadow_db_version_key)
											{
												plistData *shadow_db_version_data = shadow_db_version_key->getData();
												if (shadow_db_version_data && shadow_db_version_data->getType() == PLISTDATA_INTEGER)
												{
													plistInteger *shadow_db_version_integer= (plistInteger *)shadow_db_version_data;
													retInfo->shadow_db_version = shadow_db_version_integer->getValue();
												}
											}
										}
									}
									// now try and populate the ArtworkFormats from the plist
									// looks something like this
									/*****************************************************
									<key>AlbumArt</key>
									<array>
										<key>1069</key>
										<dict>
											<key>FormatId</key>
											<integer>1069</integer>
											<key>RenderWidth</key>
											<integer>142</integer>
											<key>RenderHeight</key>
											<integer>142</integer>
											<key>PixelFormat</key>
											<string>4C353635</string>
											<key>Interlaced</key>
											<false/>
											<key>ColorAdjustment</key>
											<integer>0</integer>
											<key>GammaAdjustment</key>
											<real>2.2</real>
											<key>Crop</key>
											<false/>
											<key>AlignRowBytes</key>
											<true/>
											<key>BackColor</key>
											<string>00000000</string>
											<key>AssociatedFormat</key>
											<integer>131072</integer>
											<key>ExcludedFormats</key>
											<integer>-1</integer>
										</dict>
										<key>1055</key>
										<dict>
											<key>FormatId</key>
											<integer>1055</integer>
											<key>RenderWidth</key>
											<integer>128</integer>
											<key>RenderHeight</key>
											<integer>128</integer>
											<key>PixelFormat</key>
											<string>4C353635</string>
											<key>Interlaced</key>
											<false/>
											<key>ColorAdjustment</key>
											<integer>0</integer>
											<key>GammaAdjustment</key>
											<real>2.2</real>
											<key>Crop</key>
											<true/>
											<key>AlignRowBytes</key>
											<true/>
											<key>BackColor</key>
											<string>00000000</string>
											<key>AssociatedFormat</key>
											<integer>0</integer>
										</dict>
									</array>
									*******************************************************************/

									// look for the AlbumArt dict
									plistKey *albumArtKey = ((plistDict*)root_dict)->getKey(L"AlbumArt");

									if (albumArtKey)
									{
										plistArray* albumArtArray = (plistArray *) albumArtKey->getData();

										if (albumArtArray)
										{
											int numFormats = albumArtArray->getNumItems();
											ArtworkFormat* artworkFormats = new ArtworkFormat[numFormats];


											retInfo->supportedArtworkFormats = &artworkFormats[0];
											retInfo->numberOfSupportedFormats = numFormats;

											for (int i=0;i<numFormats;i++)
											{
												// we need to populate this structure
												/**
												static const ArtworkFormat ipod_color_artwork_info[] = {
														{THUMB_COVER_SMALL,        56,  56, 1017, RGB_565, 4, 4},
														{THUMB_COVER_LARGE,       140, 140, 1016, RGB_565, 4, 4},
														{THUMB_PHOTO_TV_SCREEN,   720, 480, 1019, RGB_565, 4, 4},
														{THUMB_PHOTO_LARGE,       130,  88, 1015, RGB_565, 4, 4},
														{THUMB_PHOTO_FULL_SCREEN, 220, 176, 1013, RGB_565, 4, 4},
														{THUMB_PHOTO_SMALL,        42,  30, 1009, RGB_565, 4, 4},
														{THUMB_INVALID,            -1,  -1,   -1, RGB_565, 4, 4}
													};
												*/
												plistDict *albumArtFormatDict = 0;
												plistData *albumArtFormatKey = (plistKey *)albumArtArray->enumItem(i);
												if (albumArtFormatKey->getType() == PLISTDATA_KEY)
												{
													albumArtFormatDict = (plistDict *)((plistKey *)albumArtFormatKey)->getData();
												}
												else
												{ // Nano 4G doesn't store keys in the AlbumArt array
													albumArtFormatDict = (plistDict *)albumArtFormatKey;
												}

												int numKeys = albumArtFormatDict->getNumKeys();

												if (numKeys)
												{
													for (int j=0; j<numKeys; j++)
													{
														plistKey *albumArtFormatItemKey = albumArtFormatDict->enumKey(j);
														const wchar_t* albumArtFormatKeyName = albumArtFormatItemKey->getName();

														// we need all the arwork formats under AlbumArt, just use
														// a thumb type that we know is accepted
														artworkFormats[i].type = THUMB_COVER_SMALL;

														// these are 4, they just are
														artworkFormats[i].row_align = 4;
														artworkFormats[i].image_align = 4;

														// gather the FormatId
														if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, albumArtFormatKeyName, -1, L"FormatId", -1)-2 == 0)
														{
															const wchar_t* albumArtFormatValue = albumArtFormatItemKey->getData()->getString();
															if (albumArtFormatValue != NULL)
															{
																artworkFormats[i].correlation_id = _wtoi(albumArtFormatValue);
															}
														}
														// gather the RenderWidth
														if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, albumArtFormatKeyName, -1, L"RenderWidth", -1)-2 == 0)
														{
															const wchar_t* albumArtFormatValue = albumArtFormatItemKey->getData()->getString();
															if (albumArtFormatValue != NULL)
															{
																artworkFormats[i].width = _wtoi(albumArtFormatValue);
															}
														}
														// gather the RenderHeight
														if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, albumArtFormatKeyName, -1, L"RenderHeight", -1)-2 == 0)
														{
															const wchar_t* albumArtFormatValue = albumArtFormatItemKey->getData()->getString();
															if (albumArtFormatValue != NULL)
															{
																artworkFormats[i].height = _wtoi(albumArtFormatValue);
															}
														}
														// gather the PixelFormat
														if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, albumArtFormatKeyName, -1, L"PixelFormat", -1)-2 == 0)
														{
															const wchar_t* albumArtFormatValue = albumArtFormatItemKey->getData()->getString();
															if (albumArtFormatValue != NULL)
															{
																artworkFormats[i].format = RGB_565;
															}
														}
													}
												}
											}
										}
									}
									retInfo->family_id = familyID;
									return retInfo;
								}
							}

						}
					}
				} // end familyid
			}
		} // end plist parser
	}

	for (int yy=0; yy<2; yy++)
	{
		wchar_t sysinfo[] = {drive,L":\\iPod_Control\\Device\\SysInfo"};

		FILE *f = _wfopen(sysinfo,L"rt");
		if (f)
		{
			wchar_t *modelnr=NULL;
			wchar_t buf[1024] = {0};
			while (fgetws(buf,1024,f))
			{
				int len = wcslen(buf);
				//snip off trailing newline
				if (len>0 && buf[len-1]==10)
				{
					buf[len-1]=0; len--;
				}
				wchar_t *colon = wcschr(buf,L':');
				if (colon)
				{
					*colon=0;
					if (!wcscmp(L"ModelNumStr",buf))  // found ModelNumStr line..
					{
						modelnr = colon+1;
						while (modelnr && *modelnr == L' ') modelnr++;
						if (!(*modelnr >= L'0' && *modelnr <= L'9')) modelnr++;
						break; // modelnr found, so we're done
					}
				}
			}

			fclose(f);

			if (modelnr && *modelnr)
			{
				const iPodModelInfo *info = GetiPodInfoForModelStr(modelnr);
				if (info)
				{
					iPodInfo* retInfo = new iPodInfo(info);
					if (have_fwid)
						retInfo->SetFWID(fwid);

					return retInfo;
				}
			}
		}
		if (!yy)
		{
			int d = WASABI_API_DIALOGBOXPARAM(IDD_SELECTIPODTYPE,plugin.hwndWinampParent,selectipodtype_dlgproc,(LPARAM)sysinfo);
			if (d) return NULL;
		}
	}
	return new iPodInfo(&unknown);
}

const ArtworkFormat* GetArtworkFormats(const iPodInfo* info)
{
	if (!info) return NULL;
	return ipod_artwork_info_table[info->model];
}

