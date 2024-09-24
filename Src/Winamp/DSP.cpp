#include "Main.h"
#include "dsp.h"

static int initted=0;
static HINSTANCE hLib=0;
static winampDSPModule *mod=0;

void dsp_init(void)
{
	if (g_safeMode)
		return;

	wchar_t str[1024] = {0};

	if (initted)
		dsp_quit();

	if (!config_dspplugin_name[0])
		return;

	PathCombineW(str,DSPDIR,config_dspplugin_name);
	hLib = LoadLibraryW(str);

	if (!hLib)
		return;

	winampDSPGetHeaderType pr = (winampDSPGetHeaderType) GetProcAddress(hLib,"winampDSPGetHeader2");

	if (!pr || (pr(hMainWindow)->version < DSP_HDRVER && pr(hMainWindow)->version >= DSP_HDRVER+0x10) || !(mod = pr(hMainWindow)->getModule(config_dspplugin_num))) 
	{
		FreeLibrary(hLib);
		hLib=0;
		return;
	}

	mod->hwndParent=hMainWindow;
	mod->hDllInstance=hLib;
	mod->Init(mod);
	initted=1;
}

void dsp_quit(void)
{
	if (!initted)
		return;

	initted=0;
	if (mod)
	{
		if (!(config_no_visseh&2))
		{
			try
			{
				mod->Quit(mod);
			}
			catch(...)
			{
			}
		}
		else
		{
			mod->Quit(mod);
		}
	}

	FreeLibrary(hLib);
	hLib=0;
	mod=0;
}

int dsp_isactive(void)
{
	return (in_mod && initted && mod);
}

int dsp_dosamples(short int *samples, int numsamples, int bps, int nch, int srate)
{
	if (dsp_isactive())
	{
		if (mod)
			return (mod->ModifySamples(mod,samples,numsamples,bps,nch,srate));
	}

	return numsamples;
}