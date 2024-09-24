#ifndef NULLOSFT_MEDIALIBRARY_STOCK_OBJECTS_HEADER
#define NULLOSFT_MEDIALIBRARY_STOCK_OBJECTS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

#define CACHED_DC			0x0000

#define WNDBCK_BRUSH		0x0001
#define ITEMBCK_BRUSH		0x0002
#define HILITE_BRUSH		0x0003
#define ITEMTEXT_BRUSH		0x0004

#define BRUSH_MIN			0x0001
#define BRUSH_MAX			0x0004

#define PEN_MIN				0x0020
#define PEN_MAX				0x0028

#define HILITE_PEN			0x0020
#define HEADERTOP_PEN		0x0021
#define HEADERMIDDLE_PEN	0x0022
#define HEADERBOTTOM_PEN	0x0023
#define WNDBCK_PEN			0x0024
#define MENUBORDER_PEN		0x0025
#define TOOLTIPBORDER_PEN	0x0026
#define ITEMBCK_PEN			0x0027
#define ITEMTEXT_PEN		0x0028

#define SKIN_FONT			0x0100
#define DEFAULT_FONT		0x0101


void MlStockObjects_Init();
void MlStockObjects_Free();
void MlStockObjects_Reset();
HANDLE MlStockObjects_Get(UINT type);

#endif // NULLOSFT_MEDIALIBRARY_STOCK_OBJECTS_HEADER