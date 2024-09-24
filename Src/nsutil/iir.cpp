#include "iir.h"
#include <ippi.h>
#include <ipps.h>
#include <ippcc.h>

/* 32 bit floating point */
int nsutil_iir_Create_F32(const float *coefficients, int order, nsutil_iir_t *out_iir)
{
	IppsIIRState_32f *state;
	ippsIIRInitAlloc_32f(&state, coefficients, order, 0);
	*out_iir = state;
	return 0;
}

int nsutil_iir_Filter_F32_IP(nsutil_iir_t *iir, float *samples, int num_samples)
{
	ippsIIR_32f_I(samples, num_samples, (IppsIIRState_32f *)iir);
	return 0;
}

int nsutil_iir_Filter_F32(nsutil_iir_t *iir, const float *input, float *output, int num_samples)
{
	ippsIIR_32f(input, output, num_samples, (IppsIIRState_32f *)iir);
	return 0;
}

int nsutil_iir_Reset_F32(nsutil_iir_t *iir)
{
	ippsIIRSetDlyLine_32f((IppsIIRState_32f *)iir, 0);
	return 0;
}

int nsutil_iir_Destroy_F32(nsutil_iir_t *iir)
{
	ippsIIRFree_32f((IppsIIRState_32f *)iir);
	return 0;
}

/* 64 bit floating point */
int nsutil_iir_Create_F64(const double *coefficients, int order, nsutil_iir_t *out_iir)
{
	IppsIIRState_64f *state;
	ippsIIRInitAlloc_64f(&state, coefficients, order, 0);
	*out_iir = state;
	return 0;
}

int nsutil_iir_Filter_F64_IP(nsutil_iir_t *iir, double *samples, int num_samples)
{
	ippsIIR_64f_I(samples, num_samples, (IppsIIRState_64f *)iir);
	return 0;
}

int nsutil_iir_Filter_642(nsutil_iir_t *iir, const double *input, double *output, int num_samples)
{
	ippsIIR_64f(input, output, num_samples, (IppsIIRState_64f *)iir);
	return 0;
}

int nsutil_iir_Reset_F64(nsutil_iir_t *iir)
{
	ippsIIRSetDlyLine_64f((IppsIIRState_64f *)iir, 0);
	return 0;
}

int nsutil_iir_Destroy_F64(nsutil_iir_t *iir)
{
	ippsIIRFree_64f((IppsIIRState_64f *)iir);
	return 0;
}
