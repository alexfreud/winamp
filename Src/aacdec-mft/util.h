#pragma once
#include <mftransform.h>
#include <bfc/platform/types.h>

HRESULT GetAACCodec(IMFTransform **pDecoder);
HRESULT CreateAACDecoder(IMFTransform **pDecoder, const void *asc, size_t asc_bytes);
HRESULT CreateADTSDecoder(IMFTransform **pDecoder);
bool AssociateFloat(IMFTransform *decoder);