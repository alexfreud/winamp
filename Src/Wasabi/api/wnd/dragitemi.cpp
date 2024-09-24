#include <precomp.h>
#include "dragitemi.h"

DragItemI::DragItemI(const wchar_t *_datatype, void *_data) :
		datatype(_datatype)
{
	if (_data != NULL) addVoidDatum(_data);
}

void DragItemI::addVoidDatum(void *newdatum)
{
	datalist.addItem(reinterpret_cast<char *>(newdatum));
}

const wchar_t *DragItemI::getDatatype()
{
	return datatype;
};

int DragItemI::getNumData()
{
	return datalist.getNumItems();
}

void *DragItemI::getDatum(int pos)
{
	return reinterpret_cast<void *>(datalist[pos]);
}

#define CBCLASS DragItemI
START_DISPATCH;
CB(GETDATATYPE, getDatatype);
CB(GETNUMDATA, getNumData);
CB(GETDATUM, getDatum);
END_DISPATCH;
#undef CBCLASS
