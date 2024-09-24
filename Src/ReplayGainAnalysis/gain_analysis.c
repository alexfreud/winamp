/*
*  ReplayGainAnalysis - analyzes input samples and give the recommended dB change
*  Copyright (C) 2001 David Robinson and Glen Sawyer
*  Improvements and optimizations added by Frank Klemm, and by Marcel Müller 
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*  concept and filter values by David Robinson (David@Robinson.org)
*    -- blame him if you think the idea is flawed
*  original coding by Glen Sawyer (mp3gain@hotmail.com)
*    -- blame him if you think this runs too slowly, or the coding is otherwise flawed
*
*  lots of code improvements by Frank Klemm ( http://www.uni-jena.de/~pfk/mpp/ )
*    -- credit him for all the _good_ programming ;)
*
*
*  For an explanation of the concepts and the basic algorithms involved, go to:
*    http://www.replaygain.org/
*/

/*
 *  Here's the deal. Call
 *
 *    InitGainAnalysis ( long samplefreq );
 *
 *  to initialize everything. Call
 *
 *    AnalyzeSamples ( const Float_t*  left_samples,
 *                     const Float_t*  right_samples,
 *                     size_t          num_samples,
 *                     int             num_channels );
 *
 *  as many times as you want, with as many or as few samples as you want.
 *  If mono, pass the sample buffer in through left_samples, leave
 *  right_samples NULL, and make sure num_channels = 1.
 *
 *    GetTitleGain()
 *
 *  will return the recommended dB level change for all samples analyzed
 *  SINCE THE LAST TIME you called GetTitleGain() OR InitGainAnalysis().
 *
 *    GetAlbumGain()
 *
 *  will return the recommended dB level change for all samples analyzed
 *  since InitGainAnalysis() was called and finalized with GetTitleGain().
 *
 *  Pseudo-code to process an album:
 *
 *    Float_t       l_samples [4096];
 *    Float_t       r_samples [4096];
 *    size_t        num_samples;
 *    unsigned int  num_songs;
 *    unsigned int  i;
 *
 *    InitGainAnalysis ( 44100 );
 *    for ( i = 1; i <= num_songs; i++ ) {
 *        while ( ( num_samples = getSongSamples ( song[i], left_samples, right_samples ) ) > 0 )
 *            AnalyzeSamples ( left_samples, right_samples, num_samples, 2 );
 *        fprintf ("Recommended dB change for song %2d: %+6.2f dB\n", i, GetTitleGain() );
 *    }
 *    fprintf ("Recommended dB change for whole album: %+6.2f dB\n", GetAlbumGain() );
 */

/*
 *  So here's the main source of potential code confusion:
 *
 *  The filters applied to the incoming samples are IIR filters,
 *  meaning they rely on up to <filter order> number of previous samples
 *  AND up to <filter order> number of previous filtered samples.
 *
 *  I set up the AnalyzeSamples routine to minimize memory usage and interface
 *  complexity. The speed isn't compromised too much (I don't think), but the
 *  internal complexity is higher than it should be for such a relatively
 *  simple routine.
 *
 *  Optimization/clarity suggestions are welcome.
 */

/*
 *  30 Aug 2006 - Ben Allison (benski[]nullsoft.com)
 *   Modification to allow for multiple instances to be run simtulaneously (via context pointer)
 *  03 July 2007 - Marc Lerch (marc.lerch[]gmail.com) and Ben Allison (benski[]nullsoft.com)
 *   Coefficients for 64000, 88200 and 96000 sampling rates
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gain_analysis.h"
#include "../nsutil/iir.h"
#include "../nsutil/stats.h"

typedef unsigned short Uint16_t;
typedef signed short Int16_t;
typedef unsigned int Uint32_t;
typedef signed int Int32_t;

#define YULE_ORDER         10
#define BUTTER_ORDER        2
#define RMS_PERCENTILE      0.95        // percentile which is louder than the proposed level
#define MAX_SAMP_FREQ   192000.          // maximum allowed sample frequency [Hz]
#define RMS_WINDOW_TIME     0.050       // Time slice size [s]
#define STEPS_per_dB      100.          // Table entries per dB
#define MAX_dB            120.          // Table entries for 0...MAX_dB (normal max. values are 70...80 dB)

#define MAX_ORDER               (BUTTER_ORDER > YULE_ORDER ? BUTTER_ORDER : YULE_ORDER)
#define MAX_SAMPLES_PER_WINDOW  (size_t) (MAX_SAMP_FREQ * RMS_WINDOW_TIME)      // max. Samples per Time slice
#define PINK_REF                64.82 //298640883795                              // calibration value


typedef struct
{
	Float_t linprebuf [MAX_ORDER * 2];
	Float_t* linpre;                                          // left input samples, with pre-buffer
	Float_t loutbuf [MAX_SAMPLES_PER_WINDOW + MAX_ORDER];
	Float_t* lout;                                            // left "out" (i.e. post second filter) samples
	Float_t rinprebuf [MAX_ORDER * 2];
	Float_t* rinpre;                                          // right input samples ...
	Float_t routbuf [MAX_SAMPLES_PER_WINDOW + MAX_ORDER];
	Float_t* rout;
	size_t sampleWindow;                                    // number of samples required to reach number of milliseconds required for RMS window
	size_t totsamp;
	double lsum;
	double rsum;
	int freqindex;
	int first;

	Uint32_t A [(size_t)(STEPS_per_dB * MAX_dB)];
	Uint32_t B [(size_t)(STEPS_per_dB * MAX_dB)];
	nsutil_iir_t butter_iir;
	nsutil_iir_t yule_iir;
} ReplayGainContext;


#ifdef WIN32
#ifndef __GNUC__
#pragma warning ( disable : 4305 )
#endif
#endif

typedef const float yule_coefficients[2*(YULE_ORDER + 1)]; 
static yule_coefficients ABYule[] =
{
{ 0.038575994352, -0.02160367184185, -0.00123395316851, -9.291677959e-005, -0.01655260341619, 0.02161526843274, -0.02074045215285, 0.00594298065125, 0.00306428023191, 0.00012025322027, 0.00288463683916,
  1, -3.8466461711807, 7.8150165300554, -11.34170355132, 13.055042193275, -12.287598951453, 9.4829380631979, -5.87257861776, 2.7546586187461, -0.86984376593551, 0.13919314567432 },
{ 0.0541865640643, -0.02911007808948, -0.00848709379851, -0.00851165645469, -0.00834990904936, 0.02245293253339, -0.02596338512915, 0.01624864962975, -0.00240879051584, 0.00674613682247, -0.00187763777362,
  1, -3.4784594855007, 6.3631777756615, -8.5475152747187, 9.4769360780128, -8.8149868137015, 6.85401540937, -4.3947099607956, 2.1961168489077, -0.75104302451432, 0.13149317958808 },
{ 0.15457299681924, -0.09331049056315, -0.06247880153653, 0.02163541888798, -0.05588393329856, 0.04781476674921, 0.00222312597743, 0.03174092540049, -0.01390589421898, 0.00651420667831, -0.00881362733839,
  1, -2.3789883497308, 2.8486815115633, -2.6457717022983, 2.2369765745171, -1.671481533676, 1.0059595480855, -0.45953458054983, 0.16378164858596, -0.05032077717131, 0.0234789740702 },
{ 0.30296907319327, -0.22613988682123, -0.08587323730772, 0.03282930172664, -0.00915702933434, -0.02364141202522, -0.00584456039913, 0.06276101321749, -8.28086748e-006, 0.00205861885564, -0.02950134983287,
  1, -1.6127316513725, 1.0797749225997, -0.2565625775407, -0.1627671912044, -0.22638893773906, 0.39120800788284, -0.22138138954925, 0.04500235387352, 0.02005851806501, 0.00302439095741 },
{ 0.33642304856132, -0.2557224142557, -0.11828570177555, 0.11921148675203, -0.07834489609479, -0.0046997791438, -0.0058950022444, 0.05724228140351, 0.00832043980773, -0.0163538138454, -0.0176017656815,
  1, -1.498589793678, 0.87350271418188, 0.12205022308084, -0.80774944671438, 0.47854794562326, -0.12453458140019, -0.04067510197014, 0.08333755284107, -0.04237348025746, 0.02977207319925 },
{ 0.4491525660845, -0.14351757464547, -0.22784394429749, -0.01419140100551, 0.04078262797139, -0.12398163381748, 0.04097565135648, 0.10478503600251, -0.01863887810927, -0.03193428438915, 0.00541907748707,
  1, -0.62820619233671, 0.29661783706366, -0.372563729424, 0.00213767857124, -0.42029820170918, 0.22199650564824, 0.00613424350682, 0.06747620744683, 0.05784820375801, 0.03222754072173 },
{ 0.56619470757641, -0.75464456939302, 0.1624213774223, 0.16744243493672, -0.18901604199609, 0.3093178284183, -0.27562961986224, 0.00647310677246, 0.08647503780351, -0.0378898455484, -0.00588215443421,
  1, -1.0480033512635, 0.29156311971249, -0.26806001042947, 0.00819999645858, 0.45054734505008, -0.33032403314006, 0.0673936833311, -0.04784254229033, 0.01639907836189, 0.01807364323573 },
{ 0.58100494960553, -0.53174909058578, -0.14289799034253, 0.17520704835522, 0.02377945217615, 0.15558449135573, -0.25344790059353, 0.01628462406333, 0.06920467763959, -0.03721611395801, -0.00749618797172,
  1, -0.51035327095184, -0.31863563325245, -0.20256413484477, 0.1472815413433, 0.38952639978999, -0.23313271880868, -0.05246019024463, -0.02505961724053, 0.02442357316099, 0.01818801111503 },
{ 0.53648789255105, -0.42163034350696, -0.00275953611929, 0.04267842219415, -0.10214864179676, 0.14590772289388, -0.02459864859345, -0.11202315195388, -0.04060034127, 0.0478866554818, -0.02217936801134,
  1, -0.2504987195602, -0.43193942311114, -0.03424681017675, -0.04678328784242, 0.26408300200955, 0.15113130533216, -0.17556493366449, -0.18823009262115, 0.05477720428674, 0.0470440968812 },
{ 0.38524531015142, -0.27682212062067, -0.09980181488805, 0.09951486755646, -0.08934020156622, -0.00322369330199, -0.00110329090689, 0.03784509844682, 0.01683906213303, -0.01147039862572, -0.01941767987192,
  1, -1.2970891840453, 0.90399339674203, -0.29613799017877, -0.42326645916207, 0.379348874022, -0.37919795944938, 0.23410283284785, -0.03892971758879, 0.00403009552351, 0.03640166626278 },
{ 0.08717879977844, -0.01000374016172, -0.06265852122368, -0.0111932880095, -0.0011427937296, 0.02081333954769, -0.01603261863207, 0.01936763028546, 0.00760044736442, -0.00303979112271, -0.00075088605788,
  1, -2.6281631147215, 3.5373453581799, -3.8100344867892, 3.9129163673013, -3.5351860589629, 2.7135686615787, -1.8672331184659, 1.1207538236766, -0.4857408688689, 0.11330544663849 },
{ 0.03144914734085, -0.06151729206963, 0.08066788708145, -0.09737939921516, 0.08943210803999, -0.0698998467201, 0.04926972841044, -0.03161257848451, 0.01456837493506, -0.00316015108496, 0.00132807215875,
  1, -4.8737731309003, 12.039221601402, -20.101511183814, 25.103885344152, -24.290655608159, 18.271584690907, -10.452495525606, 4.30319491872, -1.1371699207019, 0.14510733527035 },
{ 0.02613056568174, -0.08128786488109, 0.14937282347325, -0.21695711675126, 0.25010286673402, -0.23162283619278, 0.17424041833052, -0.1029959921668, 0.04258696481981, -0.00977952936493, 0.00105325558889,
  1, -5.7362547709212, 16.15249794355, -29.686549124645, 39.557061556741, -39.825245562463, 30.50605345013, -17.430517728212, 7.0515457390802, -1.8078383972051, 0.22127840210813 },
{ 0.02667482047416, -0.11377479336097, 0.23063167910965, -0.30726477945593, 0.33188520686529, -0.33862680249063, 0.3180716153134, -0.2373079692988, 0.12273894790371, -0.03840017967282, 0.00549673387936,
  1, -6.318364516573, 18.313513108018, -31.882100148159, 36.537921469767, -28.233930364676, 14.247252582272, -4.0467098001285, 0.18865757280515, 0.25420333563908, -0.06012333531065 },
{ 0.00588138296683, -0.01613559730421, 0.02184798954216, -0.01742490405317, 0.0046463564378, 0.01117772513205, -0.02123865824368, 0.0195935441335, -0.01079720643523, 0.00352183686289, -0.00063124341421,
  1, -5.9780882364201, 16.213625079641, -25.729237306526, 25.404706631395, -14.661662877711, 2.8159748435975, 2.5144712596973, -2.2357530698529, 0.75788151036791, -0.10078025199029 },
{ 0.00528778718259, -0.01893240907245, 0.03185982561867, -0.02926260297838, 0.00715743034072, 0.01985743355827, -0.03222614850941, 0.02565681978192, -0.01210662313473, 0.00325436284541, -0.00044173593001,
  1, -6.2493210845629, 17.423443205385, -27.868197090549, 26.790873446813, -13.437110814851, -0.66023612948173, 6.0365809181494, -4.2492657703031, 1.4082926870919, -0.19480852628112 },
{ 0.00553120584305, -0.02112620545016, 0.03549076243117, -0.03362498312306, 0.01425867248183, 0.01344686928787, -0.03392770787836, 0.0346413645953, -0.02039116051549, 0.00667420794705, -0.00093763762995,
  1, -6.1458171083992, 16.047859036758, -22.190891314077, 15.247564715803, -0.52001440400238, -8.0048864169994, 6.6091609476886, -2.3785602281092, 0.33106947986101, 0.00459820832036 },
{ 0.0063968235945, -0.02556437970955, 0.04230854400938, -0.03722462201267, 0.01718514827295, 0.00610592243009, -0.03065965747365, 0.04345745003539, -0.03298592681309, 0.01320937236809, -0.00220304127757,
  1, -6.1481462352343, 15.800024571416, -20.784875876869, 11.988485523103, 3.3646201506261, -10.224198683595, 6.6559970214647, -1.6714186111048, -0.05417956536718, 0.07374767867406 },
{ 0.00268568524529, -0.0085237942608, 0.00852704191347, 0.00146116310295, -0.00950855828762, 0.00625449515499, 0.00116183868722, -0.00362461417136, 0.00203961000134, -0.00050664587933, 4.327455427e-005,
  1, -5.5751278276304, 12.442910560658, -12.874627996812, 3.0855484696158, 6.6249345988069, -7.0766276631325, 2.5117554273644, 0.06731510802735, -0.24567753819213, 0.03961404162376 },
{ 0.01184742123123, -0.04631092400086, 0.06584226961238, -0.02165588522478, -0.05656260778952, 0.0860749359276, -0.03375544339786, -0.04216579932754, 0.06416711490648, -0.03444708260844, 0.00697275872241,
  1, -5.2472731834817, 10.608215851922, -8.7412766581041, -1.3390607137168, 8.0797288209661, -5.4617991895085, 0.54318070652536, 0.8745096922428, -0.34656083539754, 0.03034796843589 }
};

typedef __declspec(align(32)) const float butter_coefficients[2*(BUTTER_ORDER + 1)];
static butter_coefficients ABButter[] =
{
{ 0.98621192462708, -1.9724238492542, 0.98621192462708, 1, -1.9722337291953, 0.97261396931306 },
{ 0.98500175787242, -1.9700035157448, 0.98500175787242, 1, -1.9697785558262, 0.9702284756635 },
{ 0.97938932735214, -1.9587786547043, 0.97938932735214, 1, -1.958353809754, 0.95920349965459 },
{ 0.97531843204928, -1.9506368640986, 0.97531843204928, 1, -1.9500275914988, 0.95124613669835 },
{ 0.97316523498161, -1.9463304699632, 0.97316523498161, 1, -1.9456102356653, 0.94705070426118 },
{ 0.96454515552826, -1.9290903110565, 0.96454515552826, 1, -1.9278328697704, 0.93034775234268 },
{ 0.96009142950541, -1.9201828590108, 0.96009142950541, 1, -1.9185895303378, 0.92177618768381 },
{ 0.95856916599601, -1.917138331992, 0.95856916599601, 1, -1.9154210807478, 0.91885558323625 },
{ 0.94597685600279, -1.8919537120056, 0.94597685600279, 1, -1.8890330793945, 0.89487434461664 },
{ 0.96535326815829, -1.9307065363166, 0.96535326815829, 1, -1.9295057798352, 0.93190729279793 },
{ 0.98252400815195, -1.9650480163039, 0.98252400815195, 1, -1.9647425826904, 0.9653534499174 },
{ 0.98816995007392, -1.9763399001478, 0.98816995007392, 1, -1.9761999451697, 0.97647985512594 },
{ 0.98964101933472, -1.9792820386694, 0.98964101933472, 1, -1.9791747273101, 0.9793893500288 },
{ 0.99247255046129, -1.9849451009226, 0.99247255046129, 1, -1.9848884376234, 0.98500176422183 },
{ 0.99308203517541, -1.9861640703508, 0.99308203517541, 1, -1.9861162115409, 0.98621192916075 },
{ 0.99406737810867, -1.9881347562173, 0.99406737810867, 1, -1.9880995599051, 0.98816995252954 },
{ 0.99480702681278, -1.9896140536256, 0.99480702681278, 1, -1.9895870864732, 0.9896410207779 },
{ 0.99538268958706, -1.9907653791741, 0.99538268958706, 1, -1.9907440595051, 0.99078669884321 },
{ 0.99622916581118, -1.9924583316224, 0.99622916581118, 1, -1.9924441123813, 0.99247255086339 },
{ 0.99653501465135, -1.9930700293027, 0.99653501465135, 1, -1.9930580231432, 0.99308203546221 },
};


#ifdef WIN32
#ifndef __GNUC__
#pragma warning ( default : 4305 )
#endif
#endif

// When calling these filter procedures, make sure that ip[-order] and op[-order] point to real data!

// If your compiler complains that "'operation on 'output' may be undefined", you can
// either ignore the warnings or uncomment the three "y" lines (and comment out the indicated line)

static void YULE_FILTER(nsutil_iir_t *iir, const Float_t* input, Float_t* output, size_t nSamples)
{

	nsutil_iir_Filter_F32(iir, input, output, (int)nSamples);
}

static void BUTTER_FILTER(nsutil_iir_t *iir, Float_t* samples, size_t nSamples)
{
	nsutil_iir_Filter_F32_IP(iir, samples, (int)nSamples);
}

// returns a INIT_GAIN_ANALYSIS_OK if successful, INIT_GAIN_ANALYSIS_ERROR if not

DLLEXPORT int WAResetSampleFrequency(void *context,  long samplefreq )
{
	int i;
	ReplayGainContext *rg=context;

	// zero out initial values
	for ( i = 0; i < MAX_ORDER; i++ )
		rg->linprebuf[i] = rg->loutbuf[i] = rg->rinprebuf[i] = rg->routbuf[i] = 0.;

	switch ( (int)(samplefreq) )
	{
	case 48000: rg->freqindex = 0; break;
	case 44100: rg->freqindex = 1; break;
	case 32000: rg->freqindex = 2; break;
	case 24000: rg->freqindex = 3; break;
	case 22050: rg->freqindex = 4; break;
	case 16000: rg->freqindex = 5; break;
	case 12000: rg->freqindex = 6; break;
	case 11025: rg->freqindex = 7; break;
	case 8000: rg->freqindex = 8; break;
	case 18900: rg->freqindex = 9; break;
	case 37800: rg->freqindex = 10; break;
	case 56000: rg->freqindex = 11; break;
	case 64000: rg->freqindex = 12; break;
	case 88200: rg->freqindex = 13; break;
	case 96000: rg->freqindex = 14; break;
	case 112000: rg->freqindex = 15; break;
	case 128000: rg->freqindex = 16; break;
	case 144000: rg->freqindex = 17; break;
	case 176400: rg->freqindex = 18; break;
	case 192000: rg->freqindex = 19; break;
	default: return INIT_GAIN_ANALYSIS_ERROR;
	}

	rg->sampleWindow = (int) ceil (samplefreq * RMS_WINDOW_TIME);

	rg->lsum = 0.;
	rg->rsum = 0.;
	rg->totsamp = 0;

	// TODO: ippsZero_32f
	memset ( rg->A, 0, sizeof(rg->A) );

	if (rg->butter_iir)
		nsutil_iir_Destroy_F32(rg->butter_iir);

	if (rg->yule_iir)
		nsutil_iir_Destroy_F32(rg->yule_iir);

	nsutil_iir_Create_F32(ABButter[rg->freqindex], 2, &rg->butter_iir);
	nsutil_iir_Create_F32(ABYule[rg->freqindex], 10, &rg->yule_iir);

	return INIT_GAIN_ANALYSIS_OK;
}

DLLEXPORT int WAInitGainAnalysis(void *context,  long samplefreq)
{
	ReplayGainContext *rg=context;

	if (WAResetSampleFrequency(context, samplefreq) != INIT_GAIN_ANALYSIS_OK)
	{
		return INIT_GAIN_ANALYSIS_ERROR;
	}

	rg->linpre = rg->linprebuf + MAX_ORDER;
	rg->rinpre = rg->rinprebuf + MAX_ORDER;
	rg->lout = rg->loutbuf + MAX_ORDER;
	rg->rout = rg->routbuf + MAX_ORDER;

	// TODO: ippsZero_32f
	memset ( rg->B, 0, sizeof(rg->B) );


	return INIT_GAIN_ANALYSIS_OK;
}

// returns GAIN_ANALYSIS_OK if successful, GAIN_ANALYSIS_ERROR if not

static __inline double fsqr(const double d)
{
	return d*d;
}

DLLEXPORT
int WAAnalyzeSamples(void *context, const Float_t* left_samples, const Float_t* right_samples, size_t num_samples, int num_channels)
{
	ReplayGainContext *rg=context;
	const Float_t* curleft;
	const Float_t* curright;
	float left_rms, right_rms;
	size_t batchsamples;
	size_t cursamplepos;

	if ( num_samples == 0 )
		return GAIN_ANALYSIS_OK;

	cursamplepos = 0;
	batchsamples = num_samples;

	switch ( num_channels)
	{
	case 1: right_samples = left_samples;
	case 2: break;
	default: return GAIN_ANALYSIS_ERROR;
	}

	if ( num_samples < MAX_ORDER )
	{
		memcpy ( rg->linprebuf + MAX_ORDER, left_samples , num_samples * sizeof(Float_t) );
		memcpy ( rg->rinprebuf + MAX_ORDER, right_samples, num_samples * sizeof(Float_t) );
	}
	else
	{
		memcpy ( rg->linprebuf + MAX_ORDER, left_samples, MAX_ORDER * sizeof(Float_t) );
		memcpy ( rg->rinprebuf + MAX_ORDER, right_samples, MAX_ORDER * sizeof(Float_t) );
	}

	while ( batchsamples > 0 )
	{
		size_t cursamples = batchsamples > rg->sampleWindow - rg->totsamp ? rg->sampleWindow - rg->totsamp : batchsamples;
		if ( cursamplepos < MAX_ORDER )
		{
			curleft = rg->linpre + cursamplepos;
			curright = rg->rinpre + cursamplepos;
			if (cursamples > MAX_ORDER - cursamplepos )
				cursamples = MAX_ORDER - cursamplepos;
		}
		else
		{
			curleft = left_samples + cursamplepos;
			curright = right_samples + cursamplepos;
		}

		YULE_FILTER( rg->yule_iir, curleft , rg->lout + rg->totsamp, cursamples);
		YULE_FILTER( rg->yule_iir, curright, rg->rout + rg->totsamp, cursamples);

		BUTTER_FILTER(rg->butter_iir, rg->lout + rg->totsamp, cursamples);
		BUTTER_FILTER(rg->butter_iir, rg->rout + rg->totsamp, cursamples);

		curleft = rg->lout + rg->totsamp;                   // Get the squared values
		curright = rg->rout + rg->totsamp;

		nsutil_stats_RMS_F32(curleft, cursamples, &left_rms);
		nsutil_stats_RMS_F32(curright, cursamples, &right_rms);
		rg->lsum += left_rms*left_rms;
		rg->rsum += right_rms*right_rms;

		batchsamples -= cursamples;
		cursamplepos += cursamples;
		rg->totsamp += cursamples;
		if ( rg->totsamp == rg->sampleWindow )
		{  // Get the Root Mean Square (RMS) for this set of samples
			double val = STEPS_per_dB * 10. * log10 ( (rg->lsum + rg->rsum) / rg->totsamp * 0.5 + 1.e-37 );
			int ival = (int) val;
			if ( ival < 0 ) ival = 0;
			if ( ival >= (int)(sizeof(rg->A) / sizeof(*(rg->A))) ) ival = sizeof(rg->A) / sizeof(*(rg->A)) - 1;
			rg->A [ival]++;
			rg->lsum = rg->rsum = 0.;
			memmove ( rg->loutbuf , rg->loutbuf + rg->totsamp, MAX_ORDER * sizeof(Float_t) );
			memmove ( rg->routbuf , rg->routbuf + rg->totsamp, MAX_ORDER * sizeof(Float_t) );
			rg->totsamp = 0;
		}
		if ( rg->totsamp > rg->sampleWindow )   // somehow I really screwed up: Error in programming! Contact author about totsamp > sampleWindow
			return GAIN_ANALYSIS_ERROR;
	}
	if ( num_samples < MAX_ORDER )
	{
		memmove ( rg->linprebuf, rg->linprebuf + num_samples, (MAX_ORDER - num_samples) * sizeof(Float_t) );
		memmove ( rg->rinprebuf, rg->rinprebuf + num_samples, (MAX_ORDER - num_samples) * sizeof(Float_t) );
		memcpy ( rg->linprebuf + MAX_ORDER - num_samples, left_samples, num_samples * sizeof(Float_t) );
		memcpy ( rg->rinprebuf + MAX_ORDER - num_samples, right_samples, num_samples * sizeof(Float_t) );
	}
	else
	{
		memcpy ( rg->linprebuf, left_samples + num_samples - MAX_ORDER, MAX_ORDER * sizeof(Float_t) );
		memcpy ( rg->rinprebuf, right_samples + num_samples - MAX_ORDER, MAX_ORDER * sizeof(Float_t) );
	}

	return GAIN_ANALYSIS_OK;
}


static Float_t analyzeResult(Uint32_t* Array, size_t len)
{
	Uint32_t elems;
	Int32_t upper;
	size_t i;

	elems = 0;
	for ( i = 0; i < len; i++ )
		elems += Array[i];
	if ( elems == 0 )
		return GAIN_NOT_ENOUGH_SAMPLES;

	upper = (Int32_t) ceil (elems * (1. - RMS_PERCENTILE));
	for ( i = len; i-- > 0; )
	{
		if ( (upper -= Array[i]) <= 0 )
			break;
	}

	return (Float_t) ((Float_t)PINK_REF - (Float_t)i / (Float_t)STEPS_per_dB);
}


DLLEXPORT Float_t WAGetTitleGain(void *context)
{
	ReplayGainContext *rg=context;
	Float_t retval;
	int i;

	retval = analyzeResult ( rg->A, sizeof(rg->A) / sizeof(*(rg->A)) );

	for ( i = 0; i < (int)(sizeof(rg->A) / sizeof(*(rg->A))); i++ )
	{
		// TODO: ippsAdd_32f_I
		rg->B[i] += rg->A[i];
		rg->A[i] = 0;
	}

	for ( i = 0; i < MAX_ORDER; i++ )
		rg->linprebuf[i] =  rg->loutbuf[i] = rg->rinprebuf[i] = rg->routbuf[i] = 0.f;

	nsutil_iir_Reset_F32(rg->yule_iir);
	nsutil_iir_Reset_F32(rg->butter_iir);
	rg->totsamp = 0;
	rg->lsum = rg->rsum = 0.;
	return retval;
}


DLLEXPORT Float_t WAGetAlbumGain(void *context)
{
	ReplayGainContext *rg=context;
	return analyzeResult(rg->B, sizeof(rg->B) / sizeof(*(rg->B)) );
}

DLLEXPORT void *WACreateRGContext()
{
	return calloc(1, sizeof(ReplayGainContext));
}
DLLEXPORT	void WAFreeRGContext(void *context)
{
	free(context);
}

/* end of gain_analysis.c */
