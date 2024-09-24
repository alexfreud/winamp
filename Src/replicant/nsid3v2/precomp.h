//
//  precomp.h
//  nsid3v2
//

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "foundation/error.h"
#include "foundation/types.h"


#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"

#include "nu/utf.h"
#include "nx/nxstring.h"


#ifdef __cplusplus 

#include <new>
#include "nu/AutoWide.h"
#include "nu/PtrDeque.h"

#include "nsid3v2/header.h"
#include "nsid3v2/tag.h"

#endif
