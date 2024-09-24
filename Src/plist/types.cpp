//------------------------------------------------------------------------
//
// iTunes XML Library Data Structures
// Copyright (C) 2003-2011 Nullsoft, Inc.
//
//------------------------------------------------------------------------

#include "types.h"
#include <bfc/util/base64.h>
#include <stdint.h>

//------------------------------------------------------------------------
void plistData::setString(const wchar_t *str) {
	plistString string(str);
	setData(&string);
}

//------------------------------------------------------------------------
// raw data
//------------------------------------------------------------------------
void plistRaw::setData(plistData *d) { 
	while (d && d->getType() == PLISTDATA_KEY) d = ((plistKey*)d)->getData();
	if (!d) return;
	switch (d->getType()) {
		case PLISTDATA_RAW: {
			int size;
			void *m = ((plistRaw*)d)->getMem(&size);
			setMem(m, size);
			break;
		}
		case PLISTDATA_INTEGER: {
			int v = (int)((plistInteger *)d)->getValue();
			setMem(&v, sizeof(v));
			break;
		}
		case PLISTDATA_REAL: {
			double v = ((plistReal *)d)->getValue();
			setMem(&v, sizeof(v));
			break;
		}
		case PLISTDATA_DATE: {
			time_t v = ((plistDate *)d)->getDate();
			setMem(&v, sizeof(v));
			break;
		}
		case PLISTDATA_STRING: {
			const wchar_t *s = ((plistString *)d)->getString();
			MemBlock<wchar_t> t64;
			size_t len = wcslen(s);
			t64.setSize((int)len);
			t64.setMemory(s, (int)len);
			MemBlock<char> data;
			Base64::decode(t64, data);
			setMem(data, data.getSize());
			break;
		}
		case PLISTDATA_BOOLEAN:
			{
				int8_t b = !!((plistBoolean *)d)->getValue();
				setMem(&b, 1);
			}
			break;
		case PLISTDATA_ARRAY: {
			//OutputDebugString(L"Unsupported call to setData() with an array as a parameter\n");
			break;
		}
	}
}

//------------------------------------------------------------------------
const wchar_t *plistRaw::getString() const { 
	MemBlock<wchar_t> t64;
	Base64::encode(mem, t64, 72);
	encoded.ncpy(t64.getMemory(), t64.getSize());
	return encoded; 
}

//------------------------------------------------------------------------
// integer
//------------------------------------------------------------------------
void plistInteger::setData(plistData *d) {
	while (d && d->getType() == PLISTDATA_KEY) d = ((plistKey*)d)->getData();
	if (!d) return;
	switch (d->getType()) {
		case PLISTDATA_RAW: {
			int size;
			void *m = ((plistRaw*)d)->getMem(&size);
			switch(size)
			{
			case 1:
				setValue(*((int8_t *)m));
				break;
			case 2:
				setValue(*((int16_t *)m));
				break;
			case 4:
				setValue(*((int32_t *)m));
				break;
			case 8:
				setValue(*((int64_t *)m));
				break;
			}
			break;
		}
		case PLISTDATA_BOOLEAN:
			{
				int64_t b = !!((plistBoolean *)d)->getValue();
				setValue(b);
			}
			break;
		case PLISTDATA_INTEGER: {
			setValue(((plistInteger *)d)->getValue());
			break;
		}
		case PLISTDATA_REAL: {
			setValue((int64_t)((plistReal *)d)->getValue());
			break;
		}
		case PLISTDATA_DATE: {
			setValue(((plistDate *)d)->getDate());
			break;
		}
		case PLISTDATA_STRING: {
			const wchar_t *s = ((plistString *)d)->getString();
#ifdef _WIN32
			setValue(_wtoi64(s));
#else
			setValue(wcstoll(s, 0, 10));
#endif
			break;
		}
		case PLISTDATA_ARRAY: {
			//OutputDebugString(L"Unsupported conversion from array to int\n");
			break;
		}
	}
}

//------------------------------------------------------------------------
// string
//------------------------------------------------------------------------
void plistString::setData(plistData *d) { 
	while (d && d->getType() == PLISTDATA_KEY) d = ((plistKey*)d)->getData();
	if (!d) return;
	setString(d->getString());
}

//------------------------------------------------------------------------
// date
//------------------------------------------------------------------------
void plistDate::setData(plistData *d) {
	while (d && d->getType() == PLISTDATA_KEY) d = ((plistKey*)d)->getData();
	if (!d) return;
	switch (d->getType()) {
		case PLISTDATA_RAW: {
			int size;
			void *m = ((plistRaw*)d)->getMem(&size);
			if (size >= 4)
				setDate(*(time_t*)m); // assume we have a time_t*
			break;
		}
		case PLISTDATA_INTEGER: {
			setDate((time_t)((plistInteger *)d)->getValue());
			break;
		}
		case PLISTDATA_DATE: {
			setDate(((plistDate *)d)->getDate());
			break;
		}
		case PLISTDATA_REAL: {
			setDate((int)((plistReal *)d)->getValue());
			break;
		}
		case PLISTDATA_STRING: {
			setString(d->getString());
			break;
		}
		case PLISTDATA_ARRAY: {
		OutputDebugString(L"Unsupported conversion from array to date\n");
		break;
		}
	}
}
void plistDate::setString(const wchar_t *_str)
{
	if (wcslen(_str) == 20) // naive (but safe) parser
	{
		tm import_time= {0,};
		import_time.tm_year = _wtoi(&_str[0])-1900;
		import_time.tm_mon  = _wtoi(&_str[5])-1;
		import_time.tm_mday = _wtoi(&_str[8]);
		import_time.tm_hour = _wtoi(&_str[11]);
		import_time.tm_min  = _wtoi(&_str[14]);
		import_time.tm_sec  = _wtoi(&_str[17]);

		when=_mkgmtime(&import_time);			
	}
}
//------------------------------------------------------------------------
// real
//------------------------------------------------------------------------
void plistReal::setData(plistData *d) {
	while (d && d->getType() == PLISTDATA_KEY) d = ((plistKey*)d)->getData();
	if (!d) return;
	switch (d->getType()) {
		case PLISTDATA_RAW: {
			int size;
			void *m = ((plistRaw*)d)->getMem(&size);
			if (size >= 8)
				setValue(*((double *)m));
			break;
		}
			case PLISTDATA_INTEGER: {
			setValue((double)((plistInteger *)d)->getValue());
			break;
		}
			case PLISTDATA_REAL: {
			setValue(((plistReal *)d)->getValue());
			break;
		}
			case PLISTDATA_DATE: {
			setValue((double)(((plistDate *)d)->getDate()));
			break;
		}
		case PLISTDATA_STRING: {
			const wchar_t *s = ((plistString *)d)->getString();
			setValue(WTOF(s));
			break;
		}
		case PLISTDATA_ARRAY: {
			//OutputDebugString(L"Unsupported conversion from array to real\n");
			break;
		}
	}
}

void plistBoolean::setData(plistData *d)
{
	// TODO: implement.  but how necessary is this  really?
}