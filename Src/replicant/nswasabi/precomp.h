//
//  precomp.h
//  nswasabi
//

#include <assert.h>
#include <limits.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "foundation/error.h"
#include "foundation/types.h"

#include "nu/ByteReader.h"
#include "nu/ByteWriter.h"
#include "nu/LockFreeItem.h"
#include "nu/ThreadLoop.h"

#include "nx/nxcondition.h"
#include "nx/nxstring.h"
#include "nx/nxuri.h"
#include "nx/nxmutablestring.h"

#include "metadata/metadata.h"
#include "metadata/MetadataKeys.h"

#include "nsid3v1/nsid3v1.h"
#include "nsid3v2/nsid3v2.h"

#include "service/ifc_servicefactory.h"
