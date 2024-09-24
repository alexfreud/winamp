#ifndef NULLSOFT_ML_IMAGE_FILTERWATER_HEADER
#define NULLSOFT_ML_IMAGE_FILTERWATER_HEADER

#include <windows.h>
#include ".\image.h"

class MLImageFilterWater
{
public:
	MLImageFilterWater(void);
	~MLImageFilterWater(void);

public: 
	BOOL CreateFor(const MLImage *image);
	void Render(MLImage* destination, const MLImage* source);

	void CalculateWater(int page, int density);
	void SmoothWater(int page);
	void FlattenWater(void);

	void SineBlob(int x, int y, int radius, int height, int page);
	void WarpBlob(int x, int y, int radius, int height, int page);
	void HeightBox (int x, int y, int radius, int height, int page);
	void HeightBlob(int x, int y, int radius, int height, int page);

protected:	
	void ClearData(void);
	void CalcWaterBigFilter(int page, int density);
	void DrawWaterNoLight(int page,MLImage* destination, const MLImage* source);
	void DrawWaterWithLight(MLImage* destination, const MLImage* source);
	COLORREF GetShiftedColor(COLORREF color,int shift);

private:
	HANDLE hHandle;
	int height;
	int	width;

	BOOL drawWithLight;
	int lightModifier;
	int hPage;
	int	density;

	int* hField1;
	int* hField2;
};

#endif //#define NULLSOFT_ML_IMAGE_FILTERWATER_HEADER