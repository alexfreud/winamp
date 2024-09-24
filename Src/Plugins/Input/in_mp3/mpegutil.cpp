#include "DecodeThread.h"
#include "main.h"
// benski> cut some old shit
// this code would take the filterbank coefficients to get an approximate spectrum
// and modify the coefficients to do a quick&easy EQ
// it's been off by default for quite a few versions (i think starting with 5.12)
// but not I just yanked it out.  cause none of us have 486SX's any more

static float eq_lookup1[64]={
	4.000000f,3.610166f,3.320019f,3.088821f,2.896617f,
	2.732131f,2.588368f,2.460685f,2.345845f,2.241498f,
	2.145887f,2.057660f,1.975760f,1.899338f,1.827707f,
	1.760303f,1.696653f,1.636363f,1.579094f,1.524558f,
	1.472507f,1.422724f,1.375019f,1.329225f,1.285197f,
	1.242801f,1.201923f,1.162456f,1.124306f,1.087389f,
	1.051628f,1.016951f,0.983296f,0.950604f,0.918821f,
	0.887898f,0.857789f,0.828454f,0.799853f,0.771950f,
	0.744712f,0.718108f,0.692110f,0.666689f,0.641822f,
	0.617485f,0.593655f,0.570311f,0.547435f,0.525008f,
	0.503013f,0.481433f,0.460253f,0.439458f,0.419035f,
	0.398970f,0.379252f,0.359868f,0.340807f,0.322060f,
	0.303614f,0.285462f,0.267593f,0.250000
};

static float eq_lookup2[64] = {
2.00000000f, 1.96825397f, 1.93650794f, 1.90476191f, 1.87301588f,
1.84126985f, 1.80952382f, 1.77777779f, 1.74603176f, 1.71428573f,
1.68253970f, 1.65079367f, 1.61904764f, 1.58730161f, 1.55555558f,
1.52380955f, 1.49206352f, 1.46031749f, 1.42857146f, 1.39682543f,
1.36507940f, 1.33333337f, 1.30158734f, 1.26984131f, 1.23809528f,
1.20634925f, 1.17460322f, 1.14285719f, 1.11111116f, 1.07936513f,
1.04761910f, 1.01587307f, 0.98412699f, 0.95238096f, 0.92063493f,
0.88888890f, 0.85714287f, 0.82539684f, 0.79365081f, 0.76190478f,
0.73015875f, 0.69841272f, 0.66666669f, 0.63492066f, 0.60317463f,
0.57142860f, 0.53968257f, 0.50793654f, 0.47619048f, 0.44444445f,
0.41269842f, 0.38095239f, 0.34920636f, 0.31746033f, 0.28571430f,
0.25396827f, 0.22222222f, 0.19047619f, 0.15873016f, 0.12698413f,
0.09523810f, 0.06349207f, 0.03174603f, 0.00000000
};

unsigned char eq_preamp = 0;
unsigned char eq_enabled = 0;
unsigned char eq_tab[10] = {0};

float g_vis_table[2][2][32][18] = {0};

void mp3GiveVisData(float vistable[2][32][18],int gr, int nch)
{
  if (g_vis_enabled)
  {
    memcpy(&g_vis_table[gr][0][0][0],&vistable[0][0][0],sizeof(float)*32*18*nch);
  }
}

void mp2Equalize(float *xr, int nch, int srate, int nparts)
{
  if (!g_eq_ok || !(mod.UsesOutputPlug & 2)) return;
  if (!eq_enabled) return;
	float *eq_lookup = (config_eqmode&1)?eq_lookup2:eq_lookup1;
	float preamp = eq_lookup[eq_preamp];
	int offs[11] = { 0,1,2,3,4,5,6,9,15,18,32};
	int scale_offs[11] = {0};
	int x;
	unsigned char eqt[12] = {0};
	memcpy(eqt+1,eq_tab,10);
	eqt[0]=eqt[1];
	eqt[11]=eqt[10];
	for (x = 0; x < 11; x ++)
	{
		scale_offs[x] = float_to_long( ((float) offs[x] / (float) srate * 44100.0));
		if (scale_offs[x] > 32) scale_offs[x] = 32;
	}
	{
		if (nch == 1)
		{
			register int i;
			for (i = 0; i < 10; i ++)
			{
				register int x=scale_offs[i];
				register int t=scale_offs[i+1];
				float d = eq_lookup[(int)eqt[i]]*preamp;
				float dd = (eq_lookup[(int)eqt[i+1]]*preamp-d)/(float)(t-x);
				if (dd < 0.000000001f && dd > -0.000000001f) dd = 0.000000001f;
				while (x < t)
				{
    			register float *p = xr+x;
					int e=nparts;
          while (e--)
					{
						*(p) *= d;
            p+=32;
					}
					p += 32*(18-nparts);
					d += dd;
          x++;
				}
			}
		}
		else
		{
			register int i;
			for (i = 0; i < 10; i ++)
			{
				register int x=scale_offs[i];
				register int t=scale_offs[i+1];
				float d = eq_lookup[(int)eqt[i]]*preamp;
				float dd = (eq_lookup[(int)eqt[i+1]]*preamp-d)/(float)(t-x);
				if (dd < 0.000000001f && dd > -0.000000001f) dd = 0.000000001f;
				while (x < t)
				{
    			register float *p = xr+x;
					int e=nparts;
					while (e--)
					{
						*(p+32*18) *= d;
						*(p) *= d;
						p+=32;
					}
					p += 32*(18-nparts);
					d += dd;
					x++;
				}
			}
		}
	}
}


void mp3Equalize(float *xr, int nch, int srate)
{
  if (!g_eq_ok || !(mod.UsesOutputPlug & 2)) return;
  if (!eq_enabled) return;
	float *eq_lookup = (config_eqmode&1)?eq_lookup2:eq_lookup1;
	float preamp = eq_lookup[eq_preamp];
	static int scale_offs[11];
	static int lrate;
	unsigned char eqt[12] = {0};
	memcpy(eqt+1,eq_tab,10);
	eqt[0]=eqt[1];
	eqt[11]=eqt[10];
  if (lrate!=srate)
  {
    lrate=srate;
	  for (int x = 0; x < 11; x ++)
	  {
		  int offs[11] = { 0,2,4, 9,16, 30, 63, 95,153,308, 576};
		  scale_offs[x] = float_to_long( ((float) offs[x] / (float) srate * 44100.0));
		  if (scale_offs[x] > 576) scale_offs[x] = 576;
	  }
  }
	{
		if (nch == 1)
		{
		  register float *p = xr;
		  register float d = eq_lookup[eqt[0]]*preamp;
		  register int i = 0;
		  for (i = 0; i < 10; i ++)
		  {
				register int x=scale_offs[i];
				register int t=scale_offs[i+1];
			  register float dd = (eq_lookup[eqt[i+1]]*preamp-d)/(float)(t-x);
			  while (x++ < t)
			  {
  				*(p++) *= d;
	  			d += dd;
		  	}
		  }
		}
		else
		{
			register float *p = xr;
			register int i;
			for (i = 0; i < 10; i ++)
			{
				register int x=scale_offs[i];
				register int t=scale_offs[i+1];
				float d = eq_lookup[(int)eqt[i]]*preamp;
				float dd = (eq_lookup[(int)eqt[i+1]]*preamp-d)/(float)(t-x);
				if (dd < 0.000000001f && dd > -0.000000001f) dd = 0.000000001f;
				while (x++ < t)
				{
					*(p+32*18) *= d;
					*(p++) *= d;
					d += dd;
				}
			}
		}
	}
}


void genOsc(char *tempdata, short *samples, int len)
{
	float d = 0.0f, dd = len/(75.0f*2.0f);
	short int *sbuf = samples;
	int x,y=0;
	signed char *c = (signed char *) tempdata;
	for (x = 0; x < 75; x ++)
	{
		float q=0;
		int td = float_to_long((d+=dd));
		for (; y < td; y ++)
			q += *sbuf++;
		q *= (32.0f/(dd*65536.0f));
		*c++ = (signed char) float_to_long(q);
	}
}
void genSpec(char *tempdata, float *xr, int nch)
{
	static int offsets[76] = 
	{
		0,1,2,3,4,5,7,8,9,10,12,13,15,16,18,20,21,23,25,27,29,31,33,36,38,41,43,46,48,51,
		54,57,60,64,67,71,74,78,82,86,91,95,100,105,109,115,120,126,131,137,144,150,157,
		164,171,178,186,194,203,211,220,230,239,250,260,271,282,294,306,319,332,345,360,
		374,389,576
	};
	{
		for (int x = 0; x < 75; x++) 
		{
			float sum = 0.0;
			int z;
			int sx = offsets[x];
			int ex = offsets[x+1];
			if (nch == 2)
			{
				float *p = &xr[0];
				int w=32*18;
				for (z = sx; z < ex; z ++)
				{
					register float t1=p[z], t2=p[w+z];
					if (t1 <0.0) t1=-t1;
					if (t2<0.0f) t2=-t2;
					sum += (t1+t2) * 0.5f;
				}
			} 
			else 
			{
				float *p = &xr[0];
				for (z = sx; z < ex; z ++)
				{
					register float t=p[z];
					if (t < 0.0) t=-t;
						sum += t;
				}
			}
			sum *= 1.0f + (x) * 12.0f / (ex-sx) ;
			sum *= 1.8f/24000.0f;
			if (sum < 0.0) sum = 0.0;
			if (sum > 255.0) sum = 255.0;
			tempdata[x] = (unsigned char) float_to_long(sum);
		}
	}
}

void do_layer3_vis(short *samples, float *xr, int nch, int ts)
{
  int vis_waveNch;
  int vis_specNch;
  int csa = mod.SAGetMode();
  int is_vis_running = mod.VSAGetMode(&vis_specNch,&vis_waveNch);
  static char tempdata[75*2];
  int len=32*18*nch;

  if (is_vis_running)
  {
	  char data[576*4] = {0};
	  int data_offs=0;
	  int x;

	  if (nch == 1 && vis_specNch > 0)
	  {
			float *specdata = xr;
			int y;
			for (y=0; y < 576; y++)
			{
				float p = *specdata++ / 24.0f;
				if (p < 0.0) p = -p;
				if (p > 255.0) p = 255.0;
				data[data_offs++] = (unsigned char) float_to_long(p);
			}
			if (vis_specNch == 2)
			{
				memcpy(data+data_offs,data+data_offs-576,576);
				data_offs += 576;
			}
	  }
	  else if (vis_specNch == 2)
	  {
			for (x = 0; x < 2; x ++)
			{
				float *specdata = &xr[x*32*18];
				for (int y=0; y < 576; y++)
				{
					float p = *specdata++ / 24.0f;
					if (p < 0.0) p = -p;
					if (p > 255.0) p = 255.0;
					data[data_offs++] = (unsigned char) float_to_long(p);
				}
		  }
	  }
	  else if (vis_specNch == 1)
	  {
				float *specdata = &xr[0];
				int y;
				for (y = 0; y < 576; y++)
				{
					register float p1=specdata[0],p2=specdata[32*18],p;
					if (p1 < 0.0) p1=-p1;
					if (p2 < 0.0) p2=-p2;
					p = (p1+p2)/ 48.0f;
					specdata++;
					if (p > 255.0) p = 255.0;
					data[data_offs++] = (unsigned char) float_to_long(p);
				}
	  } // end of spectrum code

	  if (nch == 1 && vis_waveNch > 0)
	  {
			for (x = 0; x < 576; x++)
			{
				data[data_offs++] = ((samples[x])>>8);
			}
			if (vis_waveNch == 2)
			{
				memcpy(data+data_offs,data+data_offs-576,576);
				data_offs += 576;
			}
	  }
	  else if (vis_waveNch == 2)
	  {
  		for (x = 0; x < 2; x ++)
	  	{
		  	int y;
				for (y = 0; y < 576; y ++ )
				{
					data[data_offs++] = ((samples[y*2+x])>>8);
				}
      }
	  }
	  else if (vis_waveNch == 1)
	  {
			int x;
			for (x = 0; x < 576; x ++)
			{
				data[data_offs++] = ((int)samples[x*2]+(int)samples[x*2+1])>>9;
			}
	  }
	  mod.VSAAdd(data,ts);
	}
	if (csa==4)
	{
			tempdata[0] = 0;
			tempdata[1] = 0;
			mod.SAAdd(tempdata,ts,4);
	}
  else if (csa == 3)
  {
    genSpec(tempdata,xr,nch);
    genOsc(tempdata+75,samples,len);
		mod.SAAdd(tempdata,ts,0x80000003);
  }
	else if (csa == 2)
	{
    genOsc(tempdata,samples,len);
		mod.SAAdd(tempdata,ts,2);
	}
	else if (csa == 1) {
    genSpec(tempdata,xr,nch);
		mod.SAAdd(tempdata,ts,1);
	}
}

