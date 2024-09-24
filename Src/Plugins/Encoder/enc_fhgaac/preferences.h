#pragma once
#include <windows.h>
#include "config.h"

extern HINSTANCE enc_fhg_HINST;
INT_PTR CALLBACK Preferences_MP4_DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Preferences_ADTS_DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* common to both */
extern const int bitrate_slider_precision;
void UpdateInfo(HWND hwnd, const AACConfiguration *config);
void UpdateUI(HWND hwnd, AACConfigurationFile *config);
int Preferences_GetEncoderVersion(char *version_string, size_t cch);
unsigned int GetSliderBitrate(HWND hwnd);

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi