#define STRICT
#include <windows.h>
#include "../Winamp/out.h"
#include "ds2.h"
#include "../pfc/pfc.h"

#define NAME "DirectSound output "DS2_ENGINE_VER

extern cfg_int cfg_def_fade;

class FadeCfg
{
public:
	const wchar_t * name;
	cfg_int time;	
	cfg_int on,usedef;
	inline UINT get_time() {return on ? (usedef ? cfg_def_fade : time) : 0;}
	inline operator int() {return get_time();}
	FadeCfg(const wchar_t* name,const wchar_t* vname,int vtime,bool von,bool vusedef);
};

#ifdef HAVE_SSRC
typedef struct
{
	UINT src_freq,src_bps,dst_freq,dst_bps;
} RESAMPLING_STATUS;
#endif

extern FadeCfg cfg_fade_start,cfg_fade_firststart,cfg_fade_stop,cfg_fade_pause,cfg_fade_seek;
