#pragma once

int GetImageIndex(const wchar_t *manufacturer, const wchar_t *model);
void GetImagePath(int image_index, int width, int height, wchar_t *path, size_t path_cch);
int GetSmallImageID(int image_index);