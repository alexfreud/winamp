#include "main.h"
#include "decoder.h"

extern "C"
{
	//returns handle!=0 if successful, 0 if error
	//size will return the final nb of bytes written to the output, -1 if unknown
	__declspec( dllexport ) intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate) {
		VorbisFile * f = VorbisFile::Create(fn,false);
		if(f) {
			if(!*bps) *bps=16; // FUCKO HAX
			Decoder * d = new Decoder();
			d->Init(f, *bps, *nch, false, false);
			*nch = (int)d->nch;
			*srate = (int)d->sr;
			*bps = (int)d->bps;
			*size = (int)(f->Length() * (double)((*nch) * (*srate) * (*bps/8)));
			return (intptr_t)d;
		}
		return 0;
	}

	__declspec( dllexport ) intptr_t winampGetExtendedRead_openW_float(const wchar_t *fn, int *size, int *bps, int *nch, int *srate) {
		VorbisFile * f = VorbisFile::Create(fn,false);
		if(f) {
			Decoder * d = new Decoder();
			d->Init(f, *bps, *nch, true, false);
			*nch = (int)d->nch;
			*srate = (int)d->sr;
			*bps = (int)d->bps;
			*size = (int)(f->Length() * (double)((*nch) * (*srate) * (*bps/8)));
			return (intptr_t)d;
		}
		return 0;
	}

	//returns nb of bytes read. -1 if read error (like CD ejected). if (ret<len), EOF is assumed
	__declspec( dllexport ) intptr_t winampGetExtendedRead_getData(intptr_t handle, char *dest, size_t len, int *killswitch) {
		Decoder * d = (Decoder *)handle;
		size_t used = 0;
		for(;;) {
			used += (UINT)d->Read((UINT)(len - used),dest + used);
			if(used >= len) break;
			if(!d->DoFrame()) break;
			if(*killswitch) break;
			if (used)
				return used;
		}
		return used;
	}

	// return nonzero on success, zero on failure.
	__declspec( dllexport ) int winampGetExtendedRead_setTime(intptr_t handle, int millisecs) {
		Decoder * d = (Decoder *)handle;
		d->Flush();
		return !d->Seek(((double)millisecs) / 1000.0);
	}

	__declspec( dllexport ) void winampGetExtendedRead_close(intptr_t handle) {
		Decoder * d = (Decoder *)handle;
		d->Flush();
		delete d->file;
		delete d;
	}
}