#ifndef NULLSOFT_DOWNLOADSDIALOGH
#define NULLSOFT_DOWNLOADSDIALOGH

#include "DownloadStatus.h"

class OmService;
HWND CALLBACK DownloadDialog_Create(HWND hParent, OmService *service);

void DownloadsUpdated();
void DownloadsUpdated( DownloadToken token, const DownloadedFile *f );
void DownloadsUpdated( const DownloadStatus::Status &s, DownloadToken token );

#endif