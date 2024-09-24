#ifndef __C_DATAPUMP_H__
#define __C_DATAPUMP_H__

#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#pragma intrinsic(memcpy,memset)

template<class T> class C_DATAPUMP {
private:
protected:
	T *BufferBottom; // bottom of the physical buffer
	T *BufferTop; // top of the physical buffer
	T *BufferStart; // start of the logical buffer
	T *BufferEnd; // end of the logical buffer

	virtual void addItems(T *inputBuffer, size_t inputSize) { // inputSize = number of <T> records inputBuffer contains
		if(inputBuffer && inputSize) {
			memcpy(BufferEnd,inputBuffer,inputSize*sizeof(T)); // copy our records in
			BufferEnd += inputSize;
			if(BufferEnd >= BufferTop) BufferEnd = BufferBottom + (BufferEnd-BufferTop);
		}
	}

	virtual void delItems(int where, size_t numItems) { // where: 0 = start, 1 = end
		if(numItems > 0) {
			if(numItems > size()) { // just void everything
				BufferEnd = BufferStart;
			} else {
				if(where == 0) { // start
					BufferStart += numItems;
					if(BufferStart >= BufferTop) BufferStart = BufferBottom + (BufferTop-BufferStart);
				} else if(where == 1) { // end
					BufferEnd -= numItems;
					if(BufferEnd < BufferBottom) BufferEnd = BufferTop - (BufferBottom-BufferEnd);
				}
			}
		}
	}

	virtual void getItems(T *outputBuffer, size_t outputSize) { // outputSize = number of <T> records outputBuffer needs
		if(outputBuffer && outputSize) {
			memcpy(outputBuffer,BufferStart,outputSize*sizeof(T));
		}
	}

public:
	C_DATAPUMP(int bufferSize) { // bufferSize = number of <T> records
		BufferBottom = NULL;
		BufferTop = NULL;
		BufferStart = NULL;
		BufferEnd = NULL;
		resizeBuffer(bufferSize);
	}

	virtual ~C_DATAPUMP() {
		if(getBufferSize() && BufferBottom) {
			free(BufferBottom);
			BufferBottom = NULL;
		}
	}

	virtual void resizeBuffer(size_t bufferSize) { // bufferSize = number of <T> records
		// this will invalidate any data in the buffer, so be careful when calling this function
		if(bufferSize) {
			if(getBufferSize() != bufferSize) {
				if(BufferBottom && BufferTop && getBufferSize()) { // buffer is valid
					if(getBufferSize() > bufferSize) { // buffer is getting smaller (will invalidate buffer)
						BufferTop -= getBufferSize()-bufferSize;
						invalidate();
					} else { // buffer is getting larger (will _NOT_ invalidate buffer... nicely moves the data over =)
					T *newBuffer = (T *)malloc(bufferSize * sizeof(T));
						// new
						BufferEnd = newBuffer + get(newBuffer,bufferSize);
						free(BufferBottom);
						BufferBottom = newBuffer;
						BufferTop = BufferBottom + bufferSize;
						BufferStart = BufferBottom;
						/* old
						T *bufptr = newBuffer;
						int top = BufferEnd >= BufferStart ? BufferEnd-BufferStart : BufferTop-BufferStart; // number of <T> records at top of physical buffer
						int bottom = BufferEnd >= BufferStart ? 0 : BufferEnd-BufferBottom; // number of <T> records at bottom of physical buffer
						if(top > 0) {
							memcpy(bufptr,BufferStart,top*sizeof(T));
							bufptr += top;
						}
						if(bottom > 0) {
							memcpy(bufptr,BufferBottom,bottom*sizeof(T));
							bufptr += bottom;
						}
						free(BufferBottom);
						BufferBottom = newBuffer;
						BufferTop = BufferBottom + bufferSize;
						BufferStart = BufferBottom;
						BufferEnd = bufptr;
						*/
					}
				} else { // no buffer, create (invalidates the buffer... duh)
					BufferBottom = (T *)malloc(bufferSize * sizeof(T));
					BufferTop = BufferBottom + bufferSize;
					invalidate();
				}
			}
		}
	}

	virtual size_t size() { // will get the number of <T> records the logical buffer contains
		return BufferEnd >= BufferStart ? BufferEnd-BufferStart : (BufferTop-BufferStart)+(BufferEnd-BufferBottom);
	}

	virtual size_t put(T *inputBuffer, size_t inputSize) { // inputSize = number of <T> records inputBuffer contains
		// returns number of <T> records added to logical buffer
		size_t retval = 0;
		if(inputBuffer && inputSize) {
		size_t fitting = ((BufferTop-BufferBottom)-1) - size(); // can't go over our logical boundary.... blah
			if(fitting > inputSize) fitting = inputSize; // the entire thing can fit.  yeay!
			retval = fitting;
			if(fitting > 0) {
			T *bufptr = inputBuffer;
			size_t top = BufferEnd >= BufferStart ? BufferTop-BufferEnd : 0; // number of <T> records free at top of physical buffer
			size_t bottom = BufferEnd >= BufferStart ? BufferStart-BufferBottom : (BufferStart-BufferEnd); // number of <T> records free at bottom of physical buffer
				if(top > 0) {
					if(top > fitting) top = fitting;
					addItems(bufptr,top);
					fitting -= top;
					bufptr += top;
				}
				if(bottom > 0 && fitting > 0) {
					if(bottom > fitting) bottom = fitting;
					addItems(bufptr,bottom);
				}
			}
		}
		return retval;
	}

	virtual size_t get(T *outputBuffer, size_t outputSize) { // outputSize = number of <T> records outputBuffer needs
	// returns number of <T> records pulled from the logical buffer
	size_t retval = 0;
		if(outputBuffer && outputSize) {
		size_t fitting = size();
			if(fitting > outputSize) fitting = outputSize;
			retval = fitting;
			if(fitting > 0) {
			T *bufptr = outputBuffer;
			size_t top = BufferEnd >= BufferStart ? BufferEnd-BufferStart : BufferTop-BufferStart; // number of <T> records at top of physical buffer
			size_t bottom = BufferEnd >= BufferStart ? 0 : BufferEnd-BufferBottom; // number of <T> records at bottom of physical buffer
				if(top > 0) {
					if(top > fitting) top = fitting;
					getItems(bufptr,top);
					delItems(0,top);
					fitting -= top;
					bufptr += top;
				}
				if(bottom > 0 && fitting > 0) {
					if(bottom > fitting) bottom = fitting;
					getItems(bufptr,bottom);
					delItems(0,bottom);
				}
			}
		}
		return retval;
	}

	virtual size_t getBufferSize() { // returns the size of the physical buffer in <T> items
		return BufferTop-BufferBottom;
	}

	virtual void invalidate() { // calling this will wipe all data in the buffer and reset the logical pointers
		BufferStart = BufferEnd = BufferBottom;
	}
};

#endif // !__C_DATAPUMP_H__