#ifndef _SPS_COMMON_H_
#define _SPS_COMMON_H_

#if (_MSC_VER <= 1200)
typedef int intptr_t;
#endif

#include "..\..\..\ns-eel2\ns-eel.h"
#include "api__dsp_sps.h"

#define MAX_CODE_LEN 32768
#define MAX_LABEL_LEN 32

char *SPSHELP_gethelptext(int sel);

typedef struct
{
	char code_text[3][MAX_CODE_LEN];

	char slider_labels[4][3][MAX_LABEL_LEN];
	int sliderpos[4];
} SPSPresetConfig;

// someday we'll have multiple of these, stackable like :)
typedef struct 
{
	CRITICAL_SECTION code_cs;
	int code_needrecompile; // &1 = init, &2 = per sample, &4=slider
	NSEEL_VMCTX vm_ctx;
	NSEEL_CODEHANDLE code[3];
	void *sample_buffer;
	int sample_buffer_len;
	int last_nch, last_srate;
	struct
	{
		double *spl0, *spl1;
		double *skip;
		double *repeat;
		double *nch;
		double *srate;
		double *sliders1,*sliders2,*sliders3,*sliders4;
		double *trigger1,*trigger2,*trigger3,*trigger4;
	} vars;
	int triggers[4];
	int sliderchange;

	int bypass; // def1
	SPSPresetConfig curpreset;
	char curpreset_name[2048];
} 
SPSEffectContext; 

void SPS_initapp();
void SPS_quitapp();
void SPS_initcontext(SPSEffectContext *ctx);
void SPS_quitcontext(SPSEffectContext *ctx);
int SPS_process_samples(SPSEffectContext *ctx, void *samples,
						int numsamples, int isfloat, int bps,
						int nch, int srate, int maxout, int minout);
void SPS_load_preset(SPSEffectContext *ctx, char *filename, char *section);
void SPS_save_preset(SPSEffectContext *ctx, char *filename, char *section);

#endif