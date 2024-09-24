#pragma once
#include <bfc/platform/types.h>
#include "BitReader.h"
#include "IDCT.h"
#include "IDCTRef.h"
#include "lib.h"

/* this is necessary for the max resolution 16CIF */
#define MBC                             88
#define MBR                             72

#define PSC        1
#define PSC_LENGTH        17
#define SE_CODE                         31

#define MODE_INTER                      0
#define MODE_INTER_Q                    1
#define MODE_INTER4V                    2
#define MODE_INTRA                      3
#define MODE_INTRA_Q                    4
#define MODE_INTER4V_Q                  5

#define PBMODE_NORMAL                   0
#define PBMODE_MVDB                     1
#define PBMODE_CBPB_MVDB                2

#define ESCAPE                          7167
#define ESCAPE_INDEX                    102
#define EP_FORWARD_PREDICTION             0
#define EI_EP_UPWARD_PREDICTION           1
#define EP_BIDIRECTIONAL_PREDICTION       2
#define EI_EP_INTRA_PREDICTION            3
#define MAX_LAYERS                      2

/* picture types */
#define PCT_INTRA                       0
#define PCT_INTER                       1
#define PCT_DISPOSABLE_INTER						2
//#define PCT_IPB                         2
#define PCT_B                           3

#define ON                              1
#define OFF                             0
#define YES                             1
#define NO                              0

#define B_EI_EP_STUFFING                  5
#define INVALID_MBTYPE                    255

#define B_DIRECT_PREDICTION               0
#define B_FORWARD_PREDICTION              1
#define B_BACKWARD_PREDICTION             2
#define B_BIDIRECTIONAL_PREDICTION        3
#define B_INTRA_PREDICTION                4

#define SF_SQCIF                        1  /* 001 */
#define SF_QCIF                         2  /* 010 */
#define SF_CIF                          3  /* 011 */
#define SF_4CIF                         4  /* 100 */
#define SF_16CIF                        5  /* 101 */


/* this is necessary for the max resolution 16CIF */
#define MBC                             88
#define MBR                             72

#define NO_VEC                          999

typedef unsigned char *Frame[3];
class Decoder
{
public:
	Decoder();
	~Decoder();
	int init();

	void getpicture(Frame frame);
	int getheader();

	int DecodeFrame(YV12_PLANES *yv12, int *width, int *height, int *keyframe);
	

	static unsigned char *clp;

	BitReader buffer;
private:
	void reconstruct(int bx, int by, int mode);
	
	void get_I_P_MBs();
	void horiz_edge_filter(unsigned char *rec, int width, int height, int chr);
	void vert_edge_filter(unsigned char *rec, int width, int height, int chr);
	void edge_filter(unsigned char *lum, unsigned char *Cb, unsigned char *Cr,                 int width, int height);
	void vert_post_filter(unsigned char *rec, int width, int height, int chr);
	void horiz_post_filter(unsigned char *rec, int width, int height, int chr);
	void PostFilter(unsigned char *lum, unsigned char *Cb, unsigned char *Cr,int width, int height);
	void addblock(int comp, int bx, int by, int addflag);
	int find_pmv(int x, int y, int block, int comp);
	void getpicturehdr();
	

	void getblock(int comp, int mode);
	void clearblock(int comp);

	void startcode();

	/* vlc */
	int getTMNMV(void);
	int getMCBPC(void);
	int getMBTYPE (int *cbp_present, int *quant_present);
	int getMCBPCintra(void);
	int getCBPY(void);

private:
	Frame refframe, oldrefframe, newframe;
	Frame edgeframe, edgeframeorig;
	int MV[2][5][MBR+1][MBC+2];
	int modemap[MBR+1][MBC+2];
	int coded_map[MBR + 1][MBC + 1];
	int quant_map[MBR + 1][MBC + 1];
	unsigned int horizontal_size,vertical_size,mb_width,mb_height;
	unsigned int coded_picture_width, coded_picture_height;
	unsigned int chrom_width,chrom_height;
	int pict_type;
	int fault;
	int deblock;
	int refidct;
	int quant;
	int escapemode;
	bool firstFrame;
	
	IDCT idct;

	  /* block data */
  short block[12][64];
	static bool initted;
	static unsigned char clp_table[1024];
	

};