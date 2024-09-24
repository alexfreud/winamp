#include "c_crossfader.h"

C_CROSSFADER::C_CROSSFADER(int length, int nCh, int sRate) : C_DATAPUMP<short>(length * 1 * 1) { // in milliseconds
	BufferLength = 0;
	srate = sRate;
	nch = nCh;
	crossfade = 0;
	mode = 0;
	SetBufferLength(length);
}

C_CROSSFADER::~C_CROSSFADER() {
	C_DATAPUMP<short>::~C_DATAPUMP();
}

// protected interfaces

void C_CROSSFADER::SampleRateConvert(int newsrate) { // in samples per second  // this needs work
	srate = newsrate;
	SetBufferLength(BufferLength);
/*
	if (BufferBottom && BufferTop) {
	int newsratestep = srate != 0 ? newsrate / srate : 0;
	int oldsratestep = newsrate != 0 ? srate / newsrate : 0;
		if (newsratestep && oldsratestep) {
		int newbuflength = (BufferLength * newsrate) / srate;
		int newbufsize = (newbuflength * nch * srate * sizeof(short)) / 1000;
		short *newbuf = (short *)malloc(newbufsize);
		short *newbufptr = newbuf;
		short *endbufptr = newbufptr+(newbufsize/sizeof(short));
		short *oldbufptr = BufferBottom;
			do {
				for(int i = 0; i < newsratestep; i++, newbufptr+=nch) {
					if (newbufptr >= endbufptr) break;
						*newbufptr = *oldbufptr;
						if (nch == 2) *(newbufptr+1) = *(oldbufptr+1);
					}
					oldbufptr += oldsratestep * nch;
			} while(newbufptr < endbufptr);
			free(BufferBottom);
			BufferLength = newbuflength;
			BufferBottom = newbuf;
			BufferTop = endbufptr;
			BufferStart = BufferEnd = BufferBottom;
		}
	}
*/
}

void C_CROSSFADER::ChannelConvert(int newnch) { // this needs work
	nch = newnch;
	SetBufferLength(BufferLength);
/*
	if (BufferBottom && BufferTop) {
	int newbuflength = (BufferLength * newnch) / nch;
	int newbufsize = (newbuflength * nch * srate * sizeof(short)) / 1000;
	short *newbuf = (short *)malloc(newbufsize);
	short *newbufptr = newbuf;
	short *endbufptr = newbufptr+(newbufsize/sizeof(short));
	short *oldbufptr = BufferBottom;
		for(; newbufptr < endbufptr; newbufptr+=newnch, oldbufptr+=nch) {
			if (newnch == 1) *newbufptr = (*oldbufptr + *(oldbufptr+1)) >> 1;
			else *(newbufptr+1) = *newbufptr = *oldbufptr;
		}
		free(BufferBottom);
		BufferLength = newbuflength;
		BufferBottom = newbuf;
		BufferTop = endbufptr;
		BufferStart = BufferEnd = BufferBottom;
	}
*/
}

// human interfaces

void C_CROSSFADER::SetSampleRate(int sRate) { // in samples per second
	if (sRate != srate) {
		if (srate && sRate) SampleRateConvert(sRate);
	}
	if (sRate) srate = sRate;
}

void C_CROSSFADER::SetChannels(int nCh) {
	if (nCh != nch) {
		if (nch && nCh) ChannelConvert(nCh);
	}
	if (nCh) nch = nCh;
}

void C_CROSSFADER::SetBufferLength(int bufferLength) { // in milliseconds
	BufferLength = bufferLength;
	resizeBuffer((BufferLength * srate * nch) / 1000);
}

void C_CROSSFADER::SetCrossfading(int onoff) {
	if (crossfade == 0) {
		crossfade = onoff ? 1 : 0;
		if (onoff) BufferEnd = BufferStart;
	}
}

void C_CROSSFADER::SetCrossfadeMode(int Mode) {
	mode = Mode;
}

void C_CROSSFADER::addItems(short *inputBuffer, size_t inputSize) {
	if (inputBuffer && inputSize) {
	size_t numsamps = inputSize*nch;
		if (crossfade==0) {
			memcpy(BufferEnd,inputBuffer,numsamps*sizeof(short)); // copy our records in
			BufferEnd += numsamps;
		} else { // do our crappy crossfade
		short *smpptr = inputBuffer;
		size_t bufsamps = (BufferTop-BufferBottom)/nch;
		size_t bufpos = (BufferEnd >= BufferStart ? BufferEnd-BufferStart : (BufferEnd-BufferBottom)+(BufferTop-BufferStart)) / nch;
		size_t dist = ((BufferTop - BufferBottom) / nch) - bufpos;
			for(size_t i = 0; i != numsamps; i++) {
				if (BufferEnd >= BufferTop) BufferEnd = BufferBottom + (BufferEnd-BufferTop);
				if (mode == 0) { // X-style (techno mix-style)
					*BufferEnd = (short)(((((double)*BufferEnd++)*dist) + (((double)*smpptr++)*bufpos)) / bufsamps);
					if (nch==1 || i&1) { // every-other I or always when mono.
						bufpos++;
						dist--;
					}
				} else if (mode == 1) { // h-style (rock radio station-style)
					*BufferEnd = (short)(((((double)*BufferEnd++)*dist) + ((double)*smpptr++)) / bufsamps);
					if (nch==1 || i&1) dist--; // every-other I or always when mono.
				}
			}
			if (dist < inputSize) crossfade = 0;
		}
		if (BufferEnd >= BufferTop) BufferEnd = BufferBottom + (BufferEnd-BufferTop);
	}
}

size_t C_CROSSFADER::put(short *inputBuffer, size_t inputSize) { // in channel-less shorts
	// returns number of <T> records added to logical buffer
	size_t retval = 0;
	if (inputBuffer && inputSize) {
	size_t fitting = (((BufferTop-BufferBottom)-1) - size()) / nch; // can't go over our logical boundary.... blah
		if (fitting > inputSize) fitting = inputSize; // the entire thing can fit.  yeay!
		retval = fitting;
		if (fitting > 0) {
		short *bufptr = inputBuffer;
		size_t top = (BufferEnd >= BufferStart ? BufferTop-BufferEnd : 0) / nch; // number of <T> records free at top of physical buffer
		size_t bottom = (BufferEnd >= BufferStart ? BufferStart-BufferBottom : (BufferStart-BufferEnd)) / nch; // number of <T> records free at bottom of physical buffer
			if (top > 0) {
				if (top > fitting) top = fitting;
				addItems(bufptr,top);
				fitting -= top;
				bufptr += top*nch;
			}
			if (bottom > 0 && fitting > 0) {
				if (bottom > fitting) bottom = fitting;
					addItems(bufptr,bottom);
			}
		}
	}
	return retval;
}

size_t C_CROSSFADER::get(short *outputBuffer, size_t outputSize, int nCh) { // in channel-less shorts
	// returns number of <T> records pulled from the logical buffer
	size_t retval = 0;
	nch = nCh;
	if (outputBuffer && outputSize) {
	size_t fitting = size() / nch;
		if (fitting > outputSize) fitting = outputSize;
		retval = fitting;
		if (fitting > 0) {
		short *bufptr = outputBuffer;
		size_t top = (BufferEnd >= BufferStart ? BufferEnd-BufferStart : BufferTop-BufferStart) / nch; // number of <T> records at top of physical buffer
		size_t bottom = (BufferEnd >= BufferStart ? 0 : BufferEnd-BufferBottom) / nch; // number of <T> records at bottom of physical buffer
			if (top > 0) {
				if (top > fitting) top = fitting;
				getItems(bufptr,top*nch);
				delItems(0,top*nch);
				fitting -= top;
				bufptr += top*nch;
			}
			if (bottom > 0 && fitting > 0) {
				if (bottom > fitting) bottom = fitting;
				getItems(bufptr,bottom*nch);
				delItems(0,bottom*nch);
			}
		}
	}
	return retval;
}