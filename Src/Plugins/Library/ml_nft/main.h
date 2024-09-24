#ifndef NULLSOFT_NFT_MAIN_H
#define NULLSOFT_NFT_MAIN_H

#include <windows.h>
#include "../Plugins/General/gen_ml/ml.h"
#include "../nu/MediaLibraryInterface.h"
#include "resource.h"
#include <windowsx.h>
#include "resource.h"
#include "../winamp/wa_ipc.h"
#include "../Plugins/General/gen_ml/ml.h"
#include "../Plugins/General/gen_ml/config.h"
#include "api__ml_nft.h"


#define NFT_BASE_URL    L"https://nftlib.winamp.com/"

extern winampMediaLibraryPlugin plugin;
INT_PTR nft_pluginMessageProc( int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3 );
extern int nft_treeItem;



#endif  // !NULLSOFT_NFT_MAIN_H