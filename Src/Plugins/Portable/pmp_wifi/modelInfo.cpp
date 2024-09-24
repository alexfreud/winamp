#include "main.h"
#include "./modelInfo.h"
#include "./api.h"
#include "./resource.h"

#include <strsafe.h>

typedef struct ManufacturerInfo
{
	const wchar_t		*name;
	const ModelInfo	*records;
	size_t				count;
} ManufacturerInfo;

#define MANUFACTURER(_name, _model_table)\
{ (_name), (_model_table), ARRAYSIZE(_model_table) }

#define MODEL(_name, _title_res_id, _small_icon_res_id, _large_icon_res_id)\
{ (_name), MAKEINTRESOURCE(_title_res_id), MAKEINTRESOURCE(_small_icon_res_id), MAKEINTRESOURCE(_large_icon_res_id)}


const struct ModelInfo HtcModelTable[] =
{
	MODEL(L"PC36100", IDS_DEVICE_MODEL_HTC_EVO, IDB_EVO_16, IDB_EVO_160),
	MODEL(L"ADR6300", IDS_DEVICE_MODEL_HTC_INCREDIBLE, IDB_INCREDIBLE_16, IDB_INCREDIBLE_160),
	MODEL(L"Nexus One", IDS_DEVICE_MODEL_HTC_NEXUS_ONE, IDB_NEXUSONE_16, IDB_NEXUSONE_160),
	MODEL(L"Desire", IDS_DEVICE_MODEL_HTC_DESIRE, IDB_HTC_DESIRE_16, IDB_HTC_DESIRE_160),
	MODEL(L"Wildfire", IDS_DEVICE_MODEL_HTC_WILDFIRE, 0, 0),
	MODEL(L"Hero", IDS_DEVICE_MODEL_HTC_HERO, 0, 0),
};

const struct ModelInfo MotorolaModelTable[] =
{
	MODEL(L"Droid", IDS_DEVICE_MODEL_MOTOROLA_DROID, IDB_DROID_16, IDB_DROID_160),
	MODEL(L"DROID2", IDS_DEVICE_MODEL_MOTOROLA_DROID2, IDB_DROID_16, IDB_DROID_160),
	MODEL(L"DROIDX", IDS_DEVICE_MODEL_MOTOROLA_DROIDX, IDB_DROIDX_16, IDB_DROIDX_160),
	MODEL(L"Milestone", IDS_DEVICE_MODEL_MOTOROLA_MILESTONE, IDB_DROID_16, IDB_DROID_160),
	MODEL(L"DROIDX2", IDS_DEVICE_MODEL_MOTOROLA_DROIDX2, IDB_DROID_16, IDB_DROID_160),
};

const struct ModelInfo SamsungModelTable[] =
{
	MODEL(L"GT I9000", IDS_DEVICE_MODEL_SAMSUNG_GALAXY_S, 0, 0),
};

const static ManufacturerInfo ManufacturerTable[] =
{
	MANUFACTURER(L"HTC", HtcModelTable),
	MANUFACTURER(L"motorola", MotorolaModelTable),
	MANUFACTURER(L"samsung", SamsungModelTable),
};

const static ModelInfo defaultModel = { L"", L"", MAKEINTRESOURCE(IDB_GENERIC_16), MAKEINTRESOURCE(IDB_GENERIC_160)};

const ModelInfo *
FindModelInfo(const wchar_t *manufacturer, const wchar_t *model, BOOL allowDefault)
{
	if (NULL != manufacturer &&
		NULL != model)
	{
		const ManufacturerInfo *manufacturerInfo;
		size_t i;
		unsigned long lcid;

		lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

		for (i = 0; i < ARRAYSIZE(ManufacturerTable); i++)
		{
			manufacturerInfo = &ManufacturerTable[i];
			if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE, manufacturerInfo->name, -1, manufacturer, -1))
			{
				for (i = 0; i < manufacturerInfo->count; i++)
				{
					const ModelInfo *modelInfo = &manufacturerInfo->records[i];
					if (CSTR_EQUAL == CompareString(lcid, NORM_IGNORECASE, modelInfo->name, -1, model, -1))
					{
						return modelInfo;
					}
				}
				break;
			}
		}
	}

	if (FALSE != allowDefault)
		return &defaultModel;

	return NULL;
}

const ModelInfo*
GetDefaultModelInfo()
{
	return &defaultModel;
}

HRESULT ModelInfo_CopyDisplayName(const ModelInfo *modelInfo, wchar_t *buffer, size_t bufferMax)
{
	if (NULL == modelInfo)
		return E_INVALIDARG;

	if (NULL == buffer)
		return E_POINTER;

	if (NULL == modelInfo->displayName)
		return E_UNEXPECTED;

	
	if (FALSE != IS_INTRESOURCE(modelInfo->displayName))
	{
		WASABI_API_LNGSTRINGW_BUF((int)(intptr_t)modelInfo->displayName, buffer, bufferMax);
		return S_OK;
	}
		
	if (FAILED(StringCchCopy(buffer, bufferMax, modelInfo->displayName)))
		return E_FAIL;
	
	return S_OK;
}

const wchar_t *ModelInfo_GetIconName(const ModelInfo *modelInfo, int width, int height, BOOL allowDefault)
{
	if (NULL == modelInfo)
	{
		if (FALSE == allowDefault)
			return NULL;

		modelInfo = GetDefaultModelInfo();
		if (NULL == modelInfo)
			return NULL;
	}
	
	if (width <= 16 && height <= 16)
	{
		if (NULL == modelInfo->smallIcon &&
			FALSE != allowDefault)
		{
			modelInfo = GetDefaultModelInfo();
			if (NULL == modelInfo)
				return NULL;
		}
		return modelInfo->smallIcon;
	}

	if (NULL == modelInfo->largeIcon &&
		FALSE != allowDefault)
	{
		modelInfo = GetDefaultModelInfo();
		if (NULL == modelInfo)
			return NULL;
	}

	return modelInfo->largeIcon;
}

HRESULT ModelInfo_GetIconPath(const ModelInfo *modelInfo, int width, int height, wchar_t *buffer, size_t bufferMax, BOOL allowDefault)
{
	const wchar_t *iconName;

	iconName = ModelInfo_GetIconName(modelInfo, width, height, allowDefault);
	if (NULL == iconName)
		return E_FAIL;

	if (FALSE == FormatResProtocol(iconName, L"PNG", buffer, bufferMax))
		return E_FAIL;

	return S_OK;
}