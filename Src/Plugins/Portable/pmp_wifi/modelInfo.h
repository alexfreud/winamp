#pragma once

typedef struct ModelInfo
{
	const wchar_t *name;
	const wchar_t *displayName;
	const wchar_t *smallIcon;
	const wchar_t *largeIcon;
} ModelInfo;


const ModelInfo *GetDefaultModelInfo();
const ModelInfo *FindModelInfo(const wchar_t *manufacturer, const wchar_t *model, BOOL allowDefault);

/* helpers*/
HRESULT ModelInfo_CopyDisplayName(const ModelInfo *modelInfo, wchar_t *buffer, size_t bufferMax);

const wchar_t *ModelInfo_GetIconName(const ModelInfo *modelInfo, int width, int height, BOOL allowDefault);

HRESULT ModelInfo_GetIconPath(const ModelInfo *modelInfo, int width, int height, wchar_t *buffer, size_t bufferMax, BOOL allowDefault);


