// Declarations of automated parameters used by the plug-in

////////////////////////////////////////////////////////////////////////////////
#ifndef DEFINE_PARAM_INFO
////////////////////////////////////////////////////////////////////////////////

enum
{
	PARAM_ENABLE,

	// TODO: Add new automated parameter IDs here

	NUM_AUTOMATED_PARAMS,

	// TODO: Add new internal parameter IDs here.  Make sure to assign the
	// first value to NUM_AUTOMATED_PARAMS, i.e.,
	//
	// _PARAM_INTERNAL1 = NUM_AUTOMATED_PARAMS,
	// _PARAM_INTERNAL2,
	// ...

	NUM_PARAMS
};

////////////////////////////////////////////////////////////////////////////////
#else
////////////////////////////////////////////////////////////////////////////////

#define MP_NONE	(0)
#define MP_JUMP	(MP_CURVE_JUMP)
#define MP_LINES	(MP_CURVE_JUMP|MP_CURVE_LINEAR)
#define MP_QUADS	(MP_CURVE_JUMP|MP_CURVE_LINEAR|MP_CURVE_SQUARE|MP_CURVE_INVSQUARE)
#define MP_ALL		(MP_CURVE_JUMP|MP_CURVE_LINEAR|MP_CURVE_SQUARE|MP_CURVE_INVSQUARE|MP_CURVE_SINE)

const ParamInfo CMediaParams::m_aParamInfo[ NUM_PARAMS ] =
{
//	MP_TYPE		MP_CAPS		min	max		def	units		label				int.min	int.max	"Enum1,Enum2,.."
//	-------		-------		---	---		---	-----		-----				-------	-------	---------------
{	MPT_BOOL,	MP_QUADS,	0,		1,			1,		L"",		L"Enabled",		0,			1,			NULL	},

// TODO: Add entries for additional parameters here

};

////////////////////////////////////////////////////////////////////////////////
#endif // DEFINE_PARAM_INFO
////////////////////////////////////////////////////////////////////////////////
