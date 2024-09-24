#ifndef STRICT
#define STRICT 1
#endif
#include <windows.h>
#include <math.h>

#include <malloc.h>

#include <mmsystem.h>
#include <dsound.h>
#ifndef MF_NO_DMCRAP
#include <dmusici.h>
#include <dmusicf.h>
#endif

class WReader;
struct CTempoMap;
struct CSysexMap;

#include "utils.h"
#include "midifile.h"
