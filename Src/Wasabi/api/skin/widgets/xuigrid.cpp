#include <precomp.h>
#include "xuigrid.h"
#include <tataki/canvas/canvas.h>
// -----------------------------------------------------------------------

const wchar_t GridXuiObjectStr[] = L"Grid"; // xml tag
char GridXuiSvcName[] = "Grid xui object"; 

XMLParamPair Grid::params[] = {
	{     GRID_SETTOPLEFT, L"TOPLEFT"},
	{         GRID_SETTOP, L"TOP"},
	{    GRID_SETTOPRIGHT, L"TOPRIGHT"},
	{        GRID_SETLEFT, L"LEFT"},
	{      GRID_SETMIDDLE, L"MIDDLE"},
	{       GRID_SETRIGHT, L"RIGHT"},
	{  GRID_SETBOTTOMLEFT, L"BOTTOMLEFT"},
	{      GRID_SETBOTTOM, L"BOTTOM"},
	{ GRID_SETBOTTOMRIGHT, L"BOTTOMRIGHT"},
};
// -----------------------------------------------------------------------
Grid::Grid() {
	setRectRgn(1);
	myxuihandle = newXuiHandle();
	CreateXMLParameters(myxuihandle);

}

void Grid::CreateXMLParameters(int master_handle)
{
	//GRID_PARENT::CreateXMLParameters(master_handle);
	int numParams = sizeof(params) / sizeof(params[0]);
	hintNumberOfParams(myxuihandle, numParams);
	for (int i = 0;i < numParams;i++)
		addParam(myxuihandle, params[i], XUI_ATTRIBUTE_IMPLIED);
}

// -----------------------------------------------------------------------
Grid::~Grid() {
}

// -----------------------------------------------------------------------
int Grid::onInit() {
	GRID_PARENT::onInit();
	doPaint(NULL, 1); // computes the region
	invalidateWindowRegion();
	return 1;
}

// -----------------------------------------------------------------------
int Grid::onResize() {
	GRID_PARENT::onResize();
	doPaint(NULL, 1);
	invalidateWindowRegion();
	return 1;
}

// -----------------------------------------------------------------------
int Grid::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
	if (xuihandle != myxuihandle)
		return GRID_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);

	switch (xmlattributeid) {
		case GRID_SETTOPLEFT:
		case GRID_SETTOP:
		case GRID_SETTOPRIGHT:
		case GRID_SETLEFT:
		case GRID_SETMIDDLE:
		case GRID_SETRIGHT:
		case GRID_SETBOTTOMLEFT:
		case GRID_SETBOTTOM:
		case GRID_SETBOTTOMRIGHT:
			setGridImage(value, xmlattributeid);
			break;
		default:
			return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
void Grid::setGridImage(const wchar_t *elementname, int what) {
	switch (what) {
		case GRID_SETTOPLEFT:       topleft = elementname;      break;
		case GRID_SETTOP:           top = elementname;          break;
		case GRID_SETTOPRIGHT:      topright = elementname;     break;
		case GRID_SETLEFT:          left = elementname;         break;
		case GRID_SETMIDDLE:        middle = elementname;       break;
		case GRID_SETRIGHT:         right = elementname;        break;
		case GRID_SETBOTTOMLEFT:    bottomleft = elementname;   break;
		case GRID_SETBOTTOM:        bottom = elementname;       break;
		case GRID_SETBOTTOMRIGHT:   bottomright = elementname;  break;
		default: return;
	}
	if (isInited()) invalidate(); 
}

// -----------------------------------------------------------------------
int Grid::onPaint(Canvas *canvas) {
	GRID_PARENT::onPaint(canvas);

	doPaint(canvas, 0);

	return 1;
}


void Grid::doPaint(Canvas *canvas, int dorgn) {

	RECT r;
	getGridRect(&r);

	SkinBitmap *left_bm = left.getBitmap();
	SkinBitmap *middle_bm = middle.getBitmap();
	SkinBitmap *right_bm = right.getBitmap();

	SkinBitmap *topleft_bm = topleft.getBitmap();
	SkinBitmap *top_bm = top.getBitmap();
	SkinBitmap *topright_bm = topright.getBitmap();

	SkinBitmap *bottomleft_bm = bottomleft.getBitmap();
	SkinBitmap *bottom_bm = bottom.getBitmap();  
	SkinBitmap *bottomright_bm = bottomright.getBitmap();  

	int left_w = left_bm ? left_bm->getWidth() : 0;
	int left_h = left_bm ? left_bm->getHeight() : 0;
	int top_h = top_bm ? top_bm->getHeight() : 0;
	int top_w = top_bm ? top_bm->getWidth() : 0;
	int topleft_h = topleft_bm ? topleft_bm->getHeight() : 0;
	int topright_h = topright_bm ? topright_bm->getHeight() : 0;
	int topleft_w = topleft_bm ? topleft_bm->getWidth() : 0;
	int topright_w = topright_bm ? topright_bm->getWidth() : 0;
	int right_w = right_bm ? right_bm->getWidth() : 0;
	int right_h = right_bm ? right_bm->getHeight() : 0;
	int bottom_h = bottom_bm ? bottom_bm->getHeight() : 0;
	int bottom_w = bottom_bm ? bottom_bm->getWidth() : 0;
	int bottomleft_h = bottomleft_bm ? bottom_bm->getHeight() : 0;
	int bottomright_h = bottomright_bm ? bottom_bm->getHeight() : 0;
	int bottomleft_w = bottomleft_bm ? bottomleft_bm->getWidth() : 0;
	int bottomright_w = bottomright_bm ? bottomright_bm->getWidth() : 0;
	int middle_w = middle_bm ? middle_bm->getWidth() : 0;
	int middle_h = middle_bm ? middle_bm->getHeight() : 0;

	RECT cell;

	if (dorgn) reg.empty();

	int paintingAlpha = 0;
	if (canvas)
		paintingAlpha = getPaintingAlpha();

	// topleft
	if (topleft_bm) {
		cell.left = r.left;
		cell.top = r.top;
		cell.right = cell.left + topleft_w;
		cell.bottom = r.top + topleft_h;

		if (canvas) topleft_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn) {
			RegionI _r(topleft);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// top
	if (top_bm) {
		cell.left = r.left + topleft_w;
		cell.top = r.top;
		cell.right = r.right - topright_w;
		cell.bottom = r.top + top_h;

		if (canvas) top_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn && cell.left != cell.right) {
			RegionI _r(top);
			_r.scale((double)(cell.right-cell.left) / top_w, 1);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// topright

	if (topright_bm) {
		cell.left = r.right - topright_w;
		cell.top = r.top;
		cell.right = r.right;
		cell.bottom = r.top + topright_h;

		if (canvas) topright_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn) {
			RegionI _r(topright);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// left

	if (left_bm) {
		cell.left = r.left;
		cell.top = r.top + topleft_h;
		cell.right = r.left + left_w;
		cell.bottom = r.bottom - bottomleft_h;

		if (canvas) left_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn && cell.bottom != cell.top) {
			RegionI _r(left);
			_r.scale(1, (double)(cell.bottom-cell.top) / left_h);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// middle

	if (middle_bm) {
		cell.left = r.left + left_w;
		cell.top = r.top + top_h;
		cell.right = r.right - right_w;
		cell.bottom = r.bottom - bottom_h;

		if (canvas) middle_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn && cell.left != cell.right && cell.bottom != cell.top) {
			RegionI _r(middle);
			_r.scale((double)(cell.right-cell.left) / middle_w, (double)(cell.bottom-cell.top) / middle_h);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// right

	if (right_bm) {
		cell.left = r.right - right_w;
		cell.top = r.top + top_h;
		cell.right = r.right;
		cell.bottom = r.bottom - bottomright_h;

		if (canvas) right_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn && cell.bottom != cell.top) {
			RegionI _r(right);
			_r.scale(1, (double)(cell.bottom-cell.top) / right_h);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// bottomleft

	if (bottomleft_bm) {
		cell.left = r.left;
		cell.top = r.bottom - bottomleft_h;
		cell.right = r.left + bottomleft_w;
		cell.bottom = r.bottom;

		if (canvas) bottomleft_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn) {
			RegionI _r(bottomleft);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// bottom

	if (bottom_bm) {
		cell.left = r.left + bottomleft_w;
		cell.top = r.bottom - bottom_h;
		cell.right = r.right - bottomright_w;
		cell.bottom = r.bottom;

		if (canvas) bottom_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn && cell.right != cell.left) {
			RegionI _r(bottom);
			_r.scale((double)(cell.right-cell.left) / bottom_w, 1);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}

	// bottomright

	if (bottomright_bm) {
		cell.left = r.right - bottomright_w;
		cell.top = r.bottom - bottomright_h;
		cell.right = r.right;
		cell.bottom = r.bottom;

		if (canvas) bottomright_bm->stretchToRectAlpha(canvas, &cell, paintingAlpha);
		if (dorgn) {
			RegionI _r(bottomright);
			_r.offset(cell.left-r.left, cell.top-r.top);
			reg.addRegion(&_r);
		}
	}
}