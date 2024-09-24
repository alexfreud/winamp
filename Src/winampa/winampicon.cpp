#include "../winamp/resource.h"

int geticonid(int x)
{
	switch (x)
	{
		case 1: return IDI_FILEICON;
		case 2: return IDI_FILEICON2;
		case 3: return IDI_FILEICON3;
		case 4: return IDI_FILEICON10;
		case 5: return IDI_FILEICON5;
		case 6: return IDI_FILEICON6;
		case 7: return IDI_FILEICON7;
		case 8: return IDI_FILEICON8;
		case 9: return IDI_FILEICON9;
		case 10: return IDI_FILEICON4;
		case 11: return IDI_FILEICON11;
		case 12: return ICON_TB1;
		case 13: return -666;
		default: return ICON_XP;
	}
}