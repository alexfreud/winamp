#pragma once

// helper functions for IDispatch
#define JSAPI_PARAM_INDEX(paramInfo, paramNumber) (paramInfo->cArgs - paramNumber)
#define JSAPI_PARAM_EXISTS(paramInfo, paramNumber) (paramInfo->cArgs >= paramNumber)
#define JSAPI_NUM_PARAMS(paramInfo) (paramInfo->cArgs)
#define JSAPI_VERIFY_PARAMCOUNT(paramInfo, count) if (paramInfo->cArgs != count) return DISP_E_BADPARAMCOUNT
#define JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(paramInfo, minParams, maxParams) if (paramInfo->cArgs < minParams || paramInfo->cArgs > maxParams) return DISP_E_BADPARAMCOUNT
#define JSAPI_VERIFY_PARAMTYPE(paramInfo, paramNumber, paramType, argErr) if (paramInfo->rgvarg[paramInfo->cArgs - paramNumber].vt != paramType) { *argErr = paramInfo->cArgs - paramNumber; return DISP_E_TYPEMISMATCH; }
#define JSAPI_VERIFY_PARAMTYPE_OPTIONAL(paramInfo, paramNumber, paramType, argErr) if (JSAPI_PARAM_EXISTS(paramInfo, paramNumber) && paramInfo->rgvarg[paramInfo->cArgs - paramNumber].vt != paramType) { *argErr = paramInfo->cArgs - paramNumber; return DISP_E_TYPEMISMATCH; }
#define JSAPI_GETSTRING(str, paramInfo, paramNumber, argErr) switch(paramInfo->rgvarg[paramInfo->cArgs - paramNumber].vt) { case VT_BSTR: str = paramInfo->rgvarg[paramInfo->cArgs - paramNumber].bstrVal; break; default: *argErr = paramInfo->cArgs - paramNumber; return DISP_E_TYPEMISMATCH; }
#define JSAPI_GETNUMBER_AS_STRING(str, buffer, paramInfo, paramNumber, argErr) \
		if (JSAPI_PARAM(paramInfo, paramNumber).vt == VT_I4) {\
		int val_int = JSAPI_PARAM(paramInfo, paramNumber).lVal;\
		StringCbPrintfW(buffer, sizeof(buffer), L"%d", val_int);\
		str = buffer;\
	}	else if (JSAPI_PARAM(paramInfo, paramNumber).vt == VT_BSTR)\
		str = JSAPI_PARAM(paramInfo, paramNumber).bstrVal;\
	else {\
		if (argErr) *argErr = paramInfo->cArgs - paramNumber;\
		return DISP_E_TYPEMISMATCH;\
	}
#define JSAPI_GETUNSIGNED_AS_NUMBER(num, paramInfo, paramNumber, argErr) \
		if (JSAPI_PARAM(paramInfo, paramNumber).vt == VT_I4) num = (UINT)JSAPI_PARAM(paramInfo, paramNumber).lVal;\
	else if (JSAPI_PARAM(paramInfo, paramNumber).vt == VT_BSTR) num = wcstoul(JSAPI_PARAM(paramInfo, paramNumber).bstrVal,0, 10);	\
	else		return DISP_E_TYPEMISMATCH;


#define JSAPI_PARAM(paramInfo, paramNumber) (paramInfo->rgvarg[paramInfo->cArgs - paramNumber])
#define JSAPI_PARAM_OPTIONAL(paramInfo, paramNumber, dispID, opt) (JSAPI_PARAM_EXISTS(paramInfo, paramNumber)?paramInfo->rgvarg[paramInfo->cArgs - paramNumber].##dispID:(opt))
#define JSAPI_VERIFY_METHOD(flags) if (!(wFlags & DISPATCH_METHOD)) return DISP_E_MEMBERNOTFOUND
#define JSAPI_INIT_RESULT(result, type) if (result) { VariantInit(result); V_VT(result) = type; }
#define JSAPI_SET_RESULT(result, field, value) if (result) { (result)->field = value; }
#define JSAPI_EMPTY_RESULT(result) if (result) { V_VT(result) = VT_EMPTY; }
#define JSAPI_SET_VARIANT(result, macro, value) if (result) { macro(result) = value; }
#define JSAPI_DISP_ENUMIFY(x) __jsapi__enum__ ## x
